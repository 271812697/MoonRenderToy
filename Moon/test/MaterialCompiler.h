#pragma once
#include <string>
#include <unordered_map>


namespace TEST {
	class JsonishValue;
	class MaterialBuilder;
	class MaterialCompiler {
	public:
		MaterialCompiler();
		static void compile(const std::string shaderFilePath);
		bool processMaterialJSON(const JsonishValue*,
			MaterialBuilder& builder) const noexcept;
		bool processVertexShaderJSON(const JsonishValue*,
			MaterialBuilder& builder) const noexcept;
		bool processFragmentShaderJSON(const JsonishValue*,
			MaterialBuilder& builder) const noexcept;
		bool processComputeShaderJSON(const JsonishValue*,
			MaterialBuilder& builder) const noexcept;
		bool ignoreLexemeJSON(const JsonishValue*, MaterialBuilder& builder) const noexcept;
	private:
		// The same, but for pure JSON syntax
		using MaterialConfigProcessorJSON = bool (MaterialCompiler::*)
			(const JsonishValue*, MaterialBuilder& builder) const;
		std::unordered_map<std::string, MaterialConfigProcessorJSON> mConfigProcessorJSON;

	};

} // namespace filament::backend


