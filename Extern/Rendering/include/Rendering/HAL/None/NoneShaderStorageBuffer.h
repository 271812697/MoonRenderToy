/**
* @project: erload
* @author: erload Tech.
* @licence: MIT
*/

#pragma once

#include <Rendering/HAL/Common/TShaderStorageBuffer.h>
#include <Rendering/HAL/None/NoneBuffer.h>

namespace Rendering::HAL
{
	struct NoneShaderStorageBufferContext {};
	using NoneShaderStorageBuffer = TShaderStorageBuffer<Settings::EGraphicsBackend::NONE, NoneShaderStorageBufferContext, NoneBufferContext>;
}
