#pragma once
#include <vector>
#include <unordered_map>
#include <Rendering/HAL/Common/TShaderProgram.h>
#include <Rendering/HAL/OpenGL/GLShaderStage.h>

namespace Rendering::HAL
{
	struct GLShaderProgramContext
	{
		const uint32_t id;
		std::unordered_map<std::string, Settings::UniformInfo> uniforms;
		std::unordered_map<std::string, uint32_t> uniformsLocationCache;
		std::vector<std::reference_wrapper<const GLShaderStage>> attachedShaders;

		uint32_t GetUniformLocation(std::string_view p_name);
	};

	using GLShaderProgram = TShaderProgram<Settings::EGraphicsBackend::OPENGL, GLShaderProgramContext, GLShaderStageContext>;
}
