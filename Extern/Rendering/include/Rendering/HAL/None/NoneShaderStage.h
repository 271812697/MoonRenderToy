/**
* @project: erload
* @author: erload Tech.
* @licence: MIT
*/

#pragma once

#include <Rendering/HAL/Common/TShaderStage.h>

namespace Rendering::HAL
{
	struct NoneShaderStageContext {};
	using NoneShaderStage = TShaderStage<Settings::EGraphicsBackend::NONE, NoneShaderStageContext>;
}
