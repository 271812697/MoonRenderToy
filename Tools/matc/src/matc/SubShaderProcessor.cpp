#include "SubShaderProcessor.h"
#include "MaterialLexeme.h"
#include "MaterialLexer.h"

#include "StringLexeme.h"
#include "StringLexer.h"
#include "JsonishLexeme.h"
#include "JsonishParser.h"
#include "JsonishLexer.h"

#include <algorithm>
#include <iostream>
#include <ctype.h>
using namespace filamat;

namespace matc {
static std::vector<std::string> extractFeatures(const std::string& value)
{
    std::vector<std::string> res;
    int k = 0;
    while( k < value.size() )
    {
        if( (value[k] == ' ' || value[k] == ',') && k < value.size() )
        {
            k++;
        }
        int m = k + 1;
        while( !(value[m] == ' ' || value[m] == ',') && m < value.size() )
        {
            m++;
        }
        res.push_back(value.substr(k, m - k));
        k = m;
    }
    return res;

}
SubShaderProcessor::SubShaderProcessor()
    {
    mConfigProcessor["pass"] = &SubShaderProcessor::processPass;
    mConfigProcessor["comment"] = &SubShaderProcessor::processComment;
   
}

bool SubShaderProcessor::process(filamat::MaterialBuilder& builder , const MaterialLexeme& subshaderlexeme)
{
    MaterialLexeme const trimmedLexeme = subshaderlexeme.trimBlockMarkers();
    MaterialLexer subshaderLexer;
    subshaderLexer.lex(trimmedLexeme.getStart(), trimmedLexeme.getSize());
    auto lexemes = subshaderLexer.getLexemes();
   
    // Make sure the lexer did not stumble upon unknown character. This could mean we received
    // a binary file.
    for( auto lexeme : lexemes )
    {
        if( lexeme.getType() == MaterialType::UNKNOWN )
        {
            std::cerr << "Unexpected character at line:" << lexeme.getLine() << " position:" << lexeme.getLinePosition()
                      << std::endl;
            return false;
        }
    }

    // Make a first quick pass just to make sure the format was respected (the material format is
    // a series of IDENTIFIER, BLOCK pairs).
    if( lexemes.size() < 2 )
    {
        std::cerr << "Input MUST be an alternation of [identifier, block] pairs." << std::endl;
        return false;
    }
    for( size_t i = 0; i < lexemes.size(); i += 2 )
    {
        auto lexeme = lexemes.at(i);
        if( lexeme.getType() != MaterialType::IDENTIFIER )
        {
            std::cerr << "An identifier was expected at line:" << lexeme.getLine()
                      << " position:" << lexeme.getLinePosition() << std::endl;
            return false;
        }

        if( i == lexemes.size() - 1 )
        {
            std::cerr << "Identifier at line:" << lexeme.getLine() << " position:" << lexeme.getLinePosition()
                      << " must be followed by a block." << std::endl;
            return false;
        }

        auto nextLexeme = lexemes.at(i + 1);
        if( nextLexeme.getType() != MaterialType::BLOCK )
        {
            std::cerr << "A block was expected at line:" << lexeme.getLine() << " position:" << lexeme.getLinePosition()
                      << std::endl;
            return false;
        }
    }
    std::string identifier;
    for( auto lexeme : lexemes )
    {
        if( lexeme.getType() == MaterialType::IDENTIFIER )
        {
            identifier = lexeme.getStringValue();
            if( mConfigProcessor.find(identifier) == mConfigProcessor.end() )
            {
                std::cerr << "Unknown identifier '" << lexeme.getStringValue() << "' at line:" << lexeme.getLine()
                          << " position:" << lexeme.getLinePosition() << std::endl;
                return false;
            }
        }
        else if( lexeme.getType() == MaterialType::BLOCK )
        {
            SubShaderConfigProcessor const processor = mConfigProcessor.at(identifier);
            if( !(*this.*processor)(lexeme, builder) )
            {
                std::cerr << "Error while processing block with key:'" << identifier << "'" << std::endl;
                return false;
            }
        }
    }
    return true;
}

bool SubShaderProcessor::processPassJson(filamat::MaterialBuilder& builder, const JsonishObject& jsonObject) const
{
    return false;
}

bool SubShaderProcessor::processComment(const MaterialLexeme& lexeme, filamat::MaterialBuilder& builder) const
{
    return true;
}



bool SubShaderProcessor::processPass(const MaterialLexeme& lexeme, filamat::MaterialBuilder& builder) const
{
    StringPairLexer slexer;
    MaterialLexeme trimLexeme = lexeme.trimBlockMarkers();
    slexer.lex(trimLexeme.getStart(), trimLexeme.getSize(), trimLexeme.getLine());
    auto& lexeArray = slexer.getLexemes();
    std::string passTag= "";
    std::vector<MaterialBuilder::ShaderCode> code;
    std::unordered_map<std::string, std::vector<std::string>> shaderFeatures;
    for( int i = 0; i < lexeArray.size(); i += 2 )
    {
        auto key=lexeArray[i].getStringValue();
        auto value = lexeArray[i+1].trimBlockMarkers().getStringValue();
        if( key == "Tag" )
        {
            passTag = value;
        }
        else if( key == "vertex" )
        {
            code.push_back(MaterialBuilder::ShaderCode(value, filament::backend::ShaderStage::VERTEX));
        }
        else if( key == "fragment" )
        {
            code.push_back(MaterialBuilder::ShaderCode(value, filament::backend::ShaderStage::FRAGMENT));
        }
        else if( key == "geomerty" )
        {
            code.push_back(MaterialBuilder::ShaderCode(value, filament::backend::ShaderStage::GEOMERTY));
        }
        else if( key == "vertexfeature" )
        {
            shaderFeatures["vertex"]  = extractFeatures(value);
        }
        else if( key == "geomertyfeature" )
        {
            shaderFeatures["geomerty"] = extractFeatures(value);
        }
        else if( key == "fragmentfeature" )
        {
            shaderFeatures["fragment"] = extractFeatures(value);
        }

    
    }
    builder.addPassInfo(passTag,shaderFeatures,code);
    return true;
}


} // namespace matc
