/**
* @project: erload
* @author: erload Tech.
* @licence: MIT
*/

#pragma once

#include <Rendering/HAL/Common/TShaderProgram.h>
#include <Rendering/HAL/None/NoneShaderStage.h>

namespace Rendering::HAL
{
	struct NoneShaderProgramContext {};
	using NoneShaderProgram = TShaderProgram<Settings::EGraphicsBackend::NONE, NoneShaderProgramContext, NoneShaderStageContext>;
}
