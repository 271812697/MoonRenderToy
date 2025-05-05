/**
* @project: erload
* @author: erload Tech.
* @licence: MIT
*/

#pragma once

#include <Rendering/HAL/Common/TIndexBuffer.h>
#include <Rendering/HAL/None/NoneBuffer.h>

namespace Rendering::HAL
{
	struct NoneIndexBufferContext {};
	using NoneIndexBuffer = TIndexBuffer<Settings::EGraphicsBackend::NONE, NoneIndexBufferContext, NoneBufferContext>;
}
