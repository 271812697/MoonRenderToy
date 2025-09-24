#include "MaterialCompiler.h"

#include <memory>
#include <iostream>
#include <utility>

#include <filamat/MaterialBuilder.h>

#include <filamat/Enums.h>


#include "DirIncluder.h"
#include "MaterialLexeme.h"
#include "MaterialLexer.h"
#include "JsonishLexer.h"
#include "JsonishParser.h"
#include "ParametersProcessor.h"
#include "SubShaderProcessor.h"


using namespace filamat;
using namespace std::placeholders;

namespace matc {

	static constexpr const char* CONFIG_KEY_MATERIAL = "material";
	static constexpr const char* CONFIG_KEY_SUBSHADER = "subshader";

	static constexpr const char* CONFIG_KEY_TOOL = "comment";

	MaterialCompiler::MaterialCompiler() {
		mConfigProcessor[CONFIG_KEY_MATERIAL] = &MaterialCompiler::processMaterial;
        mConfigProcessor[CONFIG_KEY_SUBSHADER] = &MaterialCompiler::processSubshader;

		mConfigProcessor[CONFIG_KEY_TOOL] = &MaterialCompiler::ignoreLexeme;

	}

	bool MaterialCompiler::processMaterial(const MaterialLexeme& jsonLexeme,
		MaterialBuilder& builder) const noexcept {

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

    bool MaterialCompiler::processSubshader(const MaterialLexeme& jsonLexeme, MaterialBuilder& builder) const noexcept
    {
        SubShaderProcessor subShaderProcessor;
        if( !subShaderProcessor.process(builder, jsonLexeme) )
        {
            std::cerr << "Error while processing subshader." << std::endl;
            return false;
		}
        return true;
    }

	bool MaterialCompiler::processVertexShader(const MaterialLexeme& lexeme,
		MaterialBuilder& builder) const noexcept {

		MaterialLexeme const trimmedLexeme = lexeme.trimBlockMarkers();
		std::string const shaderStr = trimmedLexeme.getStringValue();

		// getLine() returns a line number, with 1 being the first line, but .material wants a 0-based
		// line number offset, where 0 is the first line.
		builder.materialVertex(shaderStr.c_str(), trimmedLexeme.getLine() - 1);
		return true;
    }

    bool MaterialCompiler::processGeomertyShader(
        const MaterialLexeme&lexeme, filamat::MaterialBuilder& builder) const noexcept
    {
        MaterialLexeme const trimmedLexeme = lexeme.trimBlockMarkers();
        std::string const shaderStr = trimmedLexeme.getStringValue();

        // getLine() returns a line number, with 1 being the first line, but .material wants a 0-based
        // line number offset, where 0 is the first line.
        builder.materialGeomerty(shaderStr.c_str(), trimmedLexeme.getLine() - 1);
        return true;
    }

	bool MaterialCompiler::processFragmentShader(const MaterialLexeme& lexeme,
		MaterialBuilder& builder) const noexcept {

		MaterialLexeme const trimmedLexeme = lexeme.trimBlockMarkers();
		std::string const shaderStr = trimmedLexeme.getStringValue();

		// getLine() returns a line number, with 1 being the first line, but .material wants a 0-based
		// line number offset, where 0 is the first line.
		builder.material(shaderStr.c_str(), trimmedLexeme.getLine() - 1);
		return true;
	}

	bool MaterialCompiler::processComputeShader(const MaterialLexeme& lexeme,
		MaterialBuilder& builder) const noexcept {
		return MaterialCompiler::processFragmentShader(lexeme, builder);
	}

	bool MaterialCompiler::ignoreLexeme(const MaterialLexeme&, MaterialBuilder&) const noexcept {
		return true;
	}


	static bool reflectParameters(const MaterialBuilder& builder) {
		uint8_t const count = builder.getParameterCount();
		const MaterialBuilder::ParameterList& parameters = builder.getParameters();

		std::cout << "{" << std::endl;
		std::cout << "  \"parameters\": [" << std::endl;
		for (uint8_t i = 0; i < count; i++) {
			const MaterialBuilder::Parameter& parameter = parameters[i];
			std::cout << "    {" << std::endl;
			std::cout << R"(      "name": ")" << parameter.name.c_str() << "\"," << std::endl;
			if (parameter.isSampler()) {
				std::cout << R"(      "type": ")" <<
					Enums::toString(parameter.samplerType) << "\"," << std::endl;
				std::cout << R"(      "format": ")" <<
					Enums::toString(parameter.format) << "\"," << std::endl;
				std::cout << R"(      "precision": ")" <<
					Enums::toString(parameter.precision) << "\"," << std::endl;
				std::cout << R"(      "multisample": ")" <<
					(parameter.multisample ? "true" : "false") << "\"" << std::endl;
			}
			else if (parameter.isUniform()) {
				std::cout << R"(      "type": ")" <<
					Enums::toString(parameter.uniformType) << "\"," << std::endl;
				std::cout << R"(      "size": ")" << parameter.size << "\"" << std::endl;
			}
			else if (parameter.isSubpass()) {
				std::cout << R"(      "type": ")" <<
					Enums::toString(parameter.subpassType) << "\"," << std::endl;
				std::cout << R"(      "format": ")" <<
					Enums::toString(parameter.format) << "\"," << std::endl;
				std::cout << R"(      "precision": ")" <<
					Enums::toString(parameter.precision) << "\"" << std::endl;
			}
			std::cout << "    }";
			if (i < count - 1) std::cout << ",";
			std::cout << std::endl;
		}
		std::cout << "  ]" << std::endl;
		std::cout << "}" << std::endl;

		return true;
	}



	bool MaterialCompiler::run(const Config& config) {
		Config::Input* input = config.getInput();
		size_t size = input->open();
		if (size <= 0) {
			std::cerr << "Input file is empty" << std::endl;
			return false;
		}
		std::unique_ptr<const char[]> buffer = input->read();

		Path const materialFilePath = Path(input->getName()).getAbsolutePath();
		assert(materialFilePath.isFile());

		MaterialBuilder::init();
		MaterialBuilder builder;
		bool parsed = parseMaterial(buffer.get(), size_t(size), builder);

		if (!parsed) {
			return false;
		}

		// Set the root include directory to the directory containing the material file.
		DirIncluder includer;
		includer.setIncludeDirectory(materialFilePath.getParent());
        builder
            .includeCallback(includer)
            .fileName(materialFilePath.getName().c_str());
			


	

		if (!processMaterialParameters(builder, config)) {
			std::cerr << "Error while processing material parameters." << std::endl;
			return false;
		}

	

		// Write builder.build() to output.
		Package const package = builder.build();

		
		MaterialBuilder::shutdown();

		if (!package.isValid()) {
			std::cerr << "Could not compile material " << input->getName() << std::endl;
			return false;
		}
		return writePackage(package, config);
	}

	bool MaterialCompiler::checkParameters(const Config& config) {
		// Check for input file.
		if (config.getInput() == nullptr) {
			std::cerr << "Missing input filename." << std::endl;
			return false;
		}

		// If we have reflection we don't need an output file
		if (config.getReflectionTarget() != Config::Metadata::NONE) {
			return true;
		}

		// Check for output format.
		if (config.getOutput() == nullptr) {
			std::cerr << "Missing output filename." << std::endl;
			return false;
		}
		return true;
	}

	bool MaterialCompiler::parseMaterial(const char* buffer, size_t size,
		filamat::MaterialBuilder& builder) const noexcept {
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
				if (!(*this.*processor)(lexeme, builder)) {
					std::cerr << "Error while processing block with key:'" << identifier << "'"
						<< std::endl;
					return false;
				}
			}
		}
		return true;
	}

	bool MaterialCompiler::compileRawShader(const char* glsl, size_t size, bool isDebug,
		Config::Output* output, const char* ext) const noexcept {
		return true;
	}

	bool MaterialCompiler::processMaterialParameters(filamat::MaterialBuilder& builder,
		const Config& config) const {
		ParametersProcessor parametersProcessor;
		bool ok = true;
		for (const auto& param : config.getMaterialParameters()) {
			ok &= parametersProcessor.process(builder, param.first, param.second);
		}
		return ok;
	}

} // namespace matc
