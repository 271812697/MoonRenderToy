/**
* @project: erload
* @author: erload Tech.
* @licence: MIT
*/

#pragma once

#include <Rendering/HAL/Common/TIndexBuffer.h>
#include <Rendering/HAL/OpenGL/GLBuffer.h>

namespace Rendering::HAL
{
	struct GLIndexBufferContext {};
	using GLIndexBuffer = TIndexBuffer<Settings::EGraphicsBackend::OPENGL, GLIndexBufferContext, GLBufferContext>;
}
