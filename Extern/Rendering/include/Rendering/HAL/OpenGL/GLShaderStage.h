/**
* @project: erload
* @author: erload Tech.
* @licence: MIT
*/

#pragma once

#include <Rendering/HAL/Common/TShaderStage.h>

namespace Rendering::HAL
{
	struct GLShaderStageContext
	{
		uint32_t id;
		Settings::EShaderType type;
	};

	using GLShaderStage = TShaderStage<Settings::EGraphicsBackend::OPENGL, GLShaderStageContext>;
}
