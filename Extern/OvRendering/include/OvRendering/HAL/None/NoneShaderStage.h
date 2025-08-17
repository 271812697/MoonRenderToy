

#pragma once

#include <OvRendering/HAL/Common/TShaderStage.h>

namespace OvRendering::HAL
{
	struct NoneShaderStageContext {};
	using NoneShaderStage = TShaderStage<Settings::EGraphicsBackend::NONE, NoneShaderStageContext>;
}
