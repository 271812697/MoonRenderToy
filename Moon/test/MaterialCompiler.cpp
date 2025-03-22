#pragma once
#include "MaterialCompiler.h"
#include "mat/MaterialLexeme.h"
#include "mat/MaterialLexer.h"
#include "mat/JsonishLexer.h"
#include "mat/JsonishParser.h"
#include "mat/MaterialBuilder.h"
#include "mat/ParametersProcessor.h"
#include <fstream>
#include <iostream>
namespace TEST {
	static constexpr const char* CONFIG_KEY_MATERIAL = "material";
	static constexpr const char* CONFIG_KEY_VERTEX_SHADER = "vertex";
	static constexpr const char* CONFIG_KEY_FRAGMENT_SHADER = "fragment";
	static constexpr const char* CONFIG_KEY_COMPUTE_SHADER = "compute";
	static constexpr const char* CONFIG_KEY_TOOL = "tool";
	static bool isValidJsonStart(const char* buffer, size_t size);
	std::unordered_map<std::string, MaterialCompiler::MaterialConfigProcessorJSON>  MaterialCompiler::mConfigProcessorJSON;
	std::unordered_map <std::string, MaterialCompiler::MaterialConfigProcessor> MaterialCompiler::mConfigProcessor;
	MaterialCompiler::MaterialCompiler()
	{

	}

	Program MaterialCompiler::compile(const std::string shaderFilePath)
	{
		static bool initFlag = false;
		if (!initFlag) {
			mConfigProcessor[CONFIG_KEY_MATERIAL] = &MaterialCompiler::processMaterial;
			mConfigProcessor[CONFIG_KEY_VERTEX_SHADER] = &MaterialCompiler::processVertexShader;
			mConfigProcessor[CONFIG_KEY_FRAGMENT_SHADER] = &MaterialCompiler::processFragmentShader;
			mConfigProcessor[CONFIG_KEY_COMPUTE_SHADER] = &MaterialCompiler::processComputeShader;
			mConfigProcessor[CONFIG_KEY_TOOL] = &MaterialCompiler::ignoreLexeme;

			mConfigProcessorJSON[CONFIG_KEY_MATERIAL] = &MaterialCompiler::processMaterialJSON;
			mConfigProcessorJSON[CONFIG_KEY_VERTEX_SHADER] = &MaterialCompiler::processVertexShaderJSON;
			mConfigProcessorJSON[CONFIG_KEY_FRAGMENT_SHADER] = &MaterialCompiler::processFragmentShaderJSON;
			mConfigProcessorJSON[CONFIG_KEY_COMPUTE_SHADER] = &MaterialCompiler::processComputeShaderJSON;
			mConfigProcessorJSON[CONFIG_KEY_TOOL] = &MaterialCompiler::ignoreLexemeJSON;
			initFlag = true;
		}
		std::ifstream file;
		file.open(shaderFilePath, std::ifstream::binary | std::ios::ate);
		if (!file) {
			std::cerr << "Unable to open material source file '" << shaderFilePath << "'" << std::endl;
			return{};
		}

		size_t fileSize = file.tellg();
		file.seekg(0, std::ios::beg);
		if (fileSize < 0) {
			std::cerr << "Material source file is empty" << std::endl;
			return {};
		}

		std::unique_ptr<char[]> buffer = std::make_unique<char[]>(fileSize);
		if (!file.read(buffer.get(), fileSize)) {
			std::cerr << "Unable to read material source file '" << shaderFilePath << "'" << std::endl;
			return {};
		}
		MaterialBuilder builder;
		bool parsed;
		if (isValidJsonStart(buffer.get(), fileSize)) {
			parsed = parseMaterialAsJSON(buffer.get(), fileSize, builder);
		}
		else {
			parsed = parseMaterial(buffer.get(), fileSize, builder);
		}

		if (!parsed) {
			return {};
		}
		return  builder.build();
	}

	bool MaterialCompiler::parseMaterial(const char* buffer, size_t size, MaterialBuilder& builder)
	{

		MaterialLexer materialLexer;
		materialLexer.lex(buffer, size);
		auto lexemes = materialLexer.getLexemes();

		// Make sure the lexer did not stumble upon unknown character. This could mean we received
		// a binary file.
		for (auto lexeme : lexemes) {
			if (lexeme.getType() == MaterialType::UNKNOWN) {
				std::cerr << "Unexpected character at line:" << lexeme.getLine()
					<< " position:" << lexeme.getLinePosition() << std::endl;
				return false;
			}
		}

		// Make a first quick pass just to make sure the format was respected (the material format is
		// a series of IDENTIFIER, BLOCK pairs).
		if (lexemes.size() < 2) {
			std::cerr << "Input MUST be an alternation of [identifier, block] pairs." << std::endl;
			return false;
		}
		for (size_t i = 0; i < lexemes.size(); i += 2) {
			auto lexeme = lexemes.at(i);
			if (lexeme.getType() != MaterialType::IDENTIFIER) {
				std::cerr << "An identifier was expected at line:" << lexeme.getLine()
					<< " position:" << lexeme.getLinePosition() << std::endl;
				return false;
			}

			if (i == lexemes.size() - 1) {
				std::cerr << "Identifier at line:" << lexeme.getLine()
					<< " position:" << lexeme.getLinePosition()
					<< " must be followed by a block." << std::endl;
				return false;
			}

			auto nextLexeme = lexemes.at(i + 1);
			if (nextLexeme.getType() != MaterialType::BLOCK) {
				std::cerr << "A block was expected at line:" << lexeme.getLine()
					<< " position:" << lexeme.getLinePosition() << std::endl;
				return false;
			}
		}


		std::string identifier;
		for (auto lexeme : lexemes) {
			if (lexeme.getType() == MaterialType::IDENTIFIER) {
				identifier = lexeme.getStringValue();
				if (mConfigProcessor.find(identifier) == mConfigProcessor.end()) {
					std::cerr << "Unknown identifier '"
						<< lexeme.getStringValue()
						<< "' at line:" << lexeme.getLine()
						<< " position:" << lexeme.getLinePosition() << std::endl;
					return false;
				}
			}
			else if (lexeme.getType() == MaterialType::BLOCK) {
				MaterialConfigProcessor const processor = mConfigProcessor.at(identifier);
				if (!(*processor)(lexeme, builder)) {
					std::cerr << "Error while processing block with key:'" << identifier << "'"
						<< std::endl;
					return false;
				}
			}
		}
		return true;
	}

	bool MaterialCompiler::processMaterial(const MaterialLexeme& jsonLexeme, MaterialBuilder& builder)
	{
		JsonishLexer jlexer;
		jlexer.lex(jsonLexeme.getStart(), jsonLexeme.getSize(), jsonLexeme.getLine());

		JsonishParser parser(jlexer.getLexemes());
		std::unique_ptr<JsonishObject> const json = parser.parse();

		if (json == nullptr) {
			std::cerr << "JsonishParser error (see above)." << std::endl;
			return false;
		}

		ParametersProcessor parametersProcessor;
		bool const ok = parametersProcessor.process(builder, *json);
		if (!ok) {
			std::cerr << "Error while processing material." << std::endl;
			return false;
		}

		return true;
	}

	bool MaterialCompiler::processVertexShader(const MaterialLexeme& lexeme,
		MaterialBuilder& builder) {

		MaterialLexeme const trimmedLexeme = lexeme.trimBlockMarkers();
		std::string const shaderStr = trimmedLexeme.getStringValue();

		// getLine() returns a line number, with 1 being the first line, but .material wants a 0-based
		// line number offset, where 0 is the first line.
		builder.materialVertex(shaderStr.c_str(), trimmedLexeme.getLine() - 1);
		return true;
	}

	bool MaterialCompiler::processFragmentShader(const MaterialLexeme& lexeme,
		MaterialBuilder& builder) {

		MaterialLexeme const trimmedLexeme = lexeme.trimBlockMarkers();
		std::string const shaderStr = trimmedLexeme.getStringValue();

		// getLine() returns a line number, with 1 being the first line, but .material wants a 0-based
		// line number offset, where 0 is the first line.
		builder.material(shaderStr.c_str(), trimmedLexeme.getLine() - 1);
		return true;
	}

	bool MaterialCompiler::processComputeShader(const MaterialLexeme& lexeme,
		MaterialBuilder& builder) {
		return MaterialCompiler::processFragmentShader(lexeme, builder);
	}

	bool MaterialCompiler::ignoreLexeme(const MaterialLexeme&, MaterialBuilder& builder)
	{
		return true;
	}

	bool MaterialCompiler::parseMaterialAsJSON(const char* buffer, size_t size,
		MaterialBuilder& builder) {

		JsonishLexer jlexer;
		jlexer.lex(buffer, size, 1);

		JsonishParser parser(jlexer.getLexemes());
		std::unique_ptr<JsonishObject> json = parser.parse();
		if (json == nullptr) {
			std::cerr << "Could not parse JSON material file" << std::endl;
			return false;
		}

		for (auto& entry : json->getEntries()) {
			const std::string& key = entry.first;
			if (mConfigProcessorJSON.find(key) == mConfigProcessorJSON.end()) {
				std::cerr << "Unknown identifier '" << key << "'" << std::endl;
				return false;
			}

			// Retrieve function member pointer
			MaterialConfigProcessorJSON const p = mConfigProcessorJSON.at(key);
			// Call it.
			bool const ok = (*p)(entry.second, builder);
			if (!ok) {
				std::cerr << "Error while processing block with key:'" << key << "'" << std::endl;
				return false;
			}
		}

		return true;
	}

	bool MaterialCompiler::processMaterialJSON(const JsonishValue* value,
		MaterialBuilder& builder) {

		if (!value) {
			std::cerr << "'material' block does not have a value, one is required." << std::endl;
			return false;
		}

		if (value->getType() != JsonishValue::OBJECT) {
			std::cerr << "'material' block has an invalid type: "
				<< JsonishValue::typeToString(value->getType())
				<< ", should be OBJECT."
				<< std::endl;
			return false;
		}

		ParametersProcessor parametersProcessor;
		bool const ok = parametersProcessor.process(builder, *value->toJsonObject());
		if (!ok) {
			std::cerr << "Error while processing material." << std::endl;
			return false;
		}

		return true;
	}

	bool MaterialCompiler::processVertexShaderJSON(const JsonishValue* value,
		MaterialBuilder& builder) {

		if (!value) {
			std::cerr << "'vertex' block does not have a value, one is required." << std::endl;
			return false;
		}

		if (value->getType() != JsonishValue::STRING) {
			std::cerr << "'vertex' block has an invalid type: "
				<< JsonishValue::typeToString(value->getType())
				<< ", should be STRING."
				<< std::endl;
			return false;
		}

		builder.materialVertex(value->toJsonString()->getString().c_str());
		return true;
	}

	bool MaterialCompiler::processFragmentShaderJSON(const JsonishValue* value,
		MaterialBuilder& builder) {

		if (!value) {
			std::cerr << "'fragment' block does not have a value, one is required." << std::endl;
			return false;
		}

		if (value->getType() != JsonishValue::STRING) {
			std::cerr << "'fragment' block has an invalid type: "
				<< JsonishValue::typeToString(value->getType())
				<< ", should be STRING."
				<< std::endl;
			return false;
		}

		builder.material(value->toJsonString()->getString().c_str());
		return true;
	}
	bool MaterialCompiler::processComputeShaderJSON(const JsonishValue* value,
		MaterialBuilder& builder) {

		if (!value) {
			std::cerr << "'compute' block does not have a value, one is required." << std::endl;
			return false;
		}

		if (value->getType() != JsonishValue::STRING) {
			std::cerr << "'compute' block has an invalid type: "
				<< JsonishValue::typeToString(value->getType())
				<< ", should be STRING."
				<< std::endl;
			return false;
		}

		builder.material(value->toJsonString()->getString().c_str());
		return true;
	}

	bool MaterialCompiler::ignoreLexemeJSON(const JsonishValue*,
		MaterialBuilder&) {
		return true;
	}

	static bool isValidJsonStart(const char* buffer, size_t size) {
		// Skip all whitespace characters.
		const char* end = buffer + size;
		while (buffer != end && isspace(buffer[0])) {
			buffer++;
		}

		// A buffer made only of whitespace is not a valid JSON start.
		if (buffer == end) {
			return false;
		}

		const char c = buffer[0];

		// Take care of block, array, and string
		if (c == '{' || c == '[' || c == '"') {
			return true;
		}

		// boolean true
		if (c == 't' && (end - buffer) > 3 && strncmp(buffer, "true", 4) != 0) {
			return true;
		}

		// boolean false
		if (c == 'f' && (end - buffer) > 4 && strncmp(buffer, "false", 5) != 0) {
			return true;
		}

		// null literal
		return c == 'n' && (end - buffer) > 3 && strncmp(buffer, "null", 5) != 0;
	}
}


