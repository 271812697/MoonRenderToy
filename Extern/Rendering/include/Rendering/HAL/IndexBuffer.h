/**
* @project: erload
* @author: erload Tech.
* @licence: MIT
*/

#pragma once

#if defined(GRAPHICS_API_OPENGL)
#include <Rendering/HAL/OpenGL/GLIndexBuffer.h>
#else
#include <Rendering/HAL/None/NoneIndexBuffer.h>
#endif // defined(GRAPHICS_API_OPENGL)

namespace Rendering::HAL
{
#if defined(GRAPHICS_API_OPENGL)
	using IndexBuffer = GLIndexBuffer;
#else
	using IndexBuffer = NoneIndexBuffer;
#endif // defined(GRAPHICS_API_OPENGL)
}
