/**
* @project: erload
* @author: erload Tech.
* @licence: MIT
*/

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
		std::vector<Settings::UniformInfo> uniforms;
		std::vector<std::reference_wrapper<const GLShaderStage>> attachedShaders;
		std::unordered_map<std::string, int> uniformLocationCache;

		uint32_t GetUniformLocation(std::string_view p_name);
	};

	using GLShaderProgram = TShaderProgram<Settings::EGraphicsBackend::OPENGL, GLShaderProgramContext, GLShaderStageContext>;
}
