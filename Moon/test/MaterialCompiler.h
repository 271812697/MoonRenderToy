#pragma once
#include <string>
#include <unordered_map>
#include "mat/MaterialLexeme.h"

namespace TEST {
	class JsonishValue;
	class MaterialBuilder;
	class MaterialCompiler {
	public:
		MaterialCompiler();
		static void compile(const std::string shaderFilePath);

		static bool parseMaterial(const char* buffer, size_t size,
			MaterialBuilder& builder);
		static bool processMaterial(const MaterialLexeme& jsonLexeme,
			MaterialBuilder& builder);
		static bool processVertexShader(const MaterialLexeme& jsonLexeme,
			MaterialBuilder& builder);
		static bool processFragmentShader(const MaterialLexeme& jsonLexeme,
			MaterialBuilder& builder);
		static bool processComputeShader(const MaterialLexeme& jsonLexeme,
			MaterialBuilder& builder);
		static bool ignoreLexeme(const MaterialLexeme& jsonLexeme, MaterialBuilder& builder);
		static bool parseMaterialAsJSON(const char* buffer, size_t size, MaterialBuilder& builder);
		static bool processMaterialJSON(const JsonishValue*, MaterialBuilder& builder);
		static bool processVertexShaderJSON(const JsonishValue*, MaterialBuilder& builder);
		static bool processFragmentShaderJSON(const JsonishValue*, MaterialBuilder& builder);
		static bool processComputeShaderJSON(const JsonishValue*, MaterialBuilder& builder);
		static bool ignoreLexemeJSON(const JsonishValue*, MaterialBuilder& builder);
	private:
		// Member function pointer type, this is used to implement a Command design

		using MaterialConfigProcessor = bool (*)(const MaterialLexeme&, MaterialBuilder& builder);
		// Map used to store Command pattern function pointers.
		static std::unordered_map<std::string, MaterialConfigProcessor> mConfigProcessor;
		// The same, but for pure JSON syntax
		using MaterialConfigProcessorJSON = bool (*)
			(const JsonishValue*, MaterialBuilder& builder);
		static std::unordered_map<std::string, MaterialConfigProcessorJSON> mConfigProcessorJSON;

	};

} // namespace filament::backend


