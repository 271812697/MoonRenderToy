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

	MaterialCompiler::MaterialCompiler()
	{
		mConfigProcessorJSON[CONFIG_KEY_MATERIAL] = &MaterialCompiler::processMaterialJSON;
		mConfigProcessorJSON[CONFIG_KEY_VERTEX_SHADER] = &MaterialCompiler::processVertexShaderJSON;
		mConfigProcessorJSON[CONFIG_KEY_FRAGMENT_SHADER] = &MaterialCompiler::processFragmentShaderJSON;
		mConfigProcessorJSON[CONFIG_KEY_COMPUTE_SHADER] = &MaterialCompiler::processComputeShaderJSON;
		mConfigProcessorJSON[CONFIG_KEY_TOOL] = &MaterialCompiler::ignoreLexemeJSON;
	}

	void MaterialCompiler::compile(const std::string shaderFilePath)
	{
		std::ifstream file;
		file.open(shaderFilePath.c_str(), std::ifstream::binary | std::ios::ate);
		if (!file) {
			std::cerr << "Unable to open material source file '" << shaderFilePath << "'" << std::endl;
			return;
		}

		size_t fileSize = file.tellg();
		file.seekg(0, std::ios::beg);
		if (fileSize < 0) {
			std::cerr << "Material source file is empty" << std::endl;
			return;
		}

		std::unique_ptr<char[]> buffer = std::make_unique<char[]>(fileSize);
		if (!file.read(buffer.get(), fileSize)) {
			std::cerr << "Unable to read material source file '" << shaderFilePath << "'" << std::endl;
			return;
		}
		if (!isValidJsonStart(buffer.get(), fileSize)) {
			std::cerr << "is not a valid material source file'" << shaderFilePath << "'" << std::endl;
			return;
		}

		JsonishLexer jlexer;
		jlexer.lex(buffer.get(), fileSize, 1);
		JsonishParser parser(jlexer.getLexemes());
		std::unique_ptr<JsonishObject> json = parser.parse();
		if (json == nullptr) {
			std::cerr << "Could not parse JSON material file" << std::endl;
			return;
		}
		for (auto& entry : json->getEntries()) {
			const std::string& key = entry.first;

		}

	}

	bool MaterialCompiler::processMaterialJSON(const JsonishValue* value,
		MaterialBuilder& builder) const noexcept {

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
		MaterialBuilder& builder) const noexcept {

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
		MaterialBuilder& builder) const noexcept {

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
		MaterialBuilder& builder) const noexcept {

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
		MaterialBuilder&) const noexcept {
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


