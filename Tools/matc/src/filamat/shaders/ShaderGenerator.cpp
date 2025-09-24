#include "ShaderGenerator.h"
#include "CodeGenerator.h"
#include "filaflat/MaterialEnums.h"
#include "filaflat/DescriptorSets.h"
#include <filamat/MaterialBuilder.h>
#include "filaflat/backend/DriverEnums.h"
#include <sstream>


namespace filamat {

using namespace filament;
using namespace filament::backend;



void ShaderGenerator::appendShader(std::ostringstream& ss,
        const std::string& shader, size_t const lineOffset) noexcept {
    if (!shader.empty()) {
        ss << shader.c_str();
    }
}


// ------------------------------------------------------------------------------------------------

ShaderGenerator::ShaderGenerator(
        std::string const& materialCode, size_t const lineOffset,
        std::string const& materialVertexCode, size_t const vertexLineOffset) noexcept {
    mMaterialFragmentCode = materialCode;
    mMaterialVertexCode = materialVertexCode;
    mIsMaterialVertexShaderEmpty = materialVertexCode.empty();
    mMaterialLineOffset = lineOffset;
    mMaterialVertexLineOffset = vertexLineOffset;
}

void ShaderGenerator::setUpGeomertyCode(std::string const& materialCode, size_t const lineOffset)
{
    mMaterialGeomertyCode = materialCode;
    mMaterialGeomertyLineOffset = lineOffset;
}

void ShaderGenerator::setUpVertexCode(std::string const& code, size_t const lineOffset)
{
    mMaterialVertexCode = code;
    mMaterialVertexLineOffset = lineOffset;
}

void ShaderGenerator::setUpFragmentCode(std::string const& code, size_t const lineOffset)
{
    mMaterialLineOffset = lineOffset;
    mMaterialFragmentCode = code;
}



std::string ShaderGenerator::createSurfaceVertexProgram(
    MaterialInfo const& material, const std::string& featureDefine) const noexcept
{
    std::ostringstream vs;
    const CodeGenerator cg;
    cg.generateCommonProlog(vs, ShaderStage::VERTEX,featureDefine);
    CodeGenerator::generateSurfaceTypes(vs, ShaderStage::VERTEX);
    if( material.uib.getSize() > 0 )
    {
        cg.generateUniforms(vs, ShaderStage::VERTEX,material.uib);   
    }
    CodeGenerator::generateSeparator(vs);
    cg.generateCommonSamplers(vs, material.sib, ShaderStage::VERTEX);
    // shader code
    CodeGenerator::generateSurfaceGetters(vs, ShaderStage::VERTEX);

    // main entry point
    appendShader(vs, mMaterialVertexCode, mMaterialVertexLineOffset);
    CodeGenerator::generateSurfaceMain(vs, ShaderStage::VERTEX);
    CodeGenerator::generateCommonEpilog(vs);
    return vs.str();
}

std::string ShaderGenerator::createSurfaceGeomertyProgram(
    MaterialInfo const& material, const std::string& featureDefine) const noexcept
{

    std::ostringstream gs;
    const CodeGenerator cg;
    cg.generateCommonProlog(gs, ShaderStage::GEOMERTY,featureDefine);
    

    // uniforms
    if (material.uib.getSize() > 0)
    {
        cg.generateUniforms(gs, ShaderStage::GEOMERTY, 
             material.uib);
    }
    CodeGenerator::generateSeparator(gs);
    cg.generateCommonSamplers(gs, material.sib, ShaderStage::GEOMERTY);
    // shader code
    appendShader(gs, mMaterialGeomertyCode, mMaterialGeomertyLineOffset);
    CodeGenerator::generateSurfaceMain(gs, ShaderStage::GEOMERTY);
    CodeGenerator::generateCommonEpilog(gs);
    return gs.str();
}

std::string ShaderGenerator::createSurfaceFragmentProgram(
    MaterialInfo const& material, const std::string& featureDefine) const noexcept
{
    const CodeGenerator cg;
    std::ostringstream fs;
    cg.generateCommonProlog(fs, ShaderStage::FRAGMENT,featureDefine);

    // uniforms and samplers
    if( material.uib.getSize() > 0 )
    {
        cg.generateUniforms(fs, ShaderStage::FRAGMENT,
                material.uib);    
    }


    CodeGenerator::generateSeparator(fs);
    cg.generateCommonSamplers(fs, material.sib, ShaderStage::FRAGMENT);
    appendShader(fs, mMaterialFragmentCode, mMaterialLineOffset);
   
    CodeGenerator::generateCommonEpilog(fs);
    return fs.str();
}

} // namespace filament
