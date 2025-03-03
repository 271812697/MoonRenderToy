#pragma once
#include "MaterialCompiler.h"
#include "mat/MaterialLexeme.h"
#include "mat/MaterialLexer.h"
#include "mat/JsonishLexer.h"
#include "mat/JsonishParser.h"
#include <fstream>
#include <iostream>
namespace TEST {
	static bool isValidJsonStart(const char* buffer, size_t size);

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


