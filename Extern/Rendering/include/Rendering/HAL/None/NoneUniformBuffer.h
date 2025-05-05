/**
* @project: erload
* @author: erload Tech.
* @licence: MIT
*/

#pragma once

#include <Rendering/HAL/Common/TUniformBuffer.h>
#include <Rendering/HAL/None/NoneBuffer.h>

namespace Rendering::HAL
{
	struct NoneUniformBufferContext {};
	using NoneUniformBuffer = TUniformBuffer<Settings::EGraphicsBackend::NONE, NoneUniformBufferContext, NoneBufferContext>;
}
