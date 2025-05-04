/**
* @project: erload
* @author: erload Tech.
* @licence: MIT
*/

#pragma once

#include <Rendering/HAL/Common/TVertexBuffer.h>
#include <Rendering/HAL/None/NoneBuffer.h>

namespace Rendering::HAL
{
	struct NoneVertexBufferContext {};
	using NoneVertexBuffer = TVertexBuffer<Settings::EGraphicsBackend::NONE, NoneVertexBufferContext, NoneBufferContext>;
}
