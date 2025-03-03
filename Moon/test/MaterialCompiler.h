#pragma once
#include <string>
#include <unordered_map>

namespace TEST {
	class MaterialCompiler {
	public:
		static void compile(const std::string shaderFilePath);
	private:
		//std::unordered_map<std::string, MaterialConfigProcessorJSON> mConfigProcessorJSON;

	};

} // namespace filament::backend


