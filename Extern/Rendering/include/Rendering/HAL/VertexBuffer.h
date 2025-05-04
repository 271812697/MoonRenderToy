/**
* @project: erload
* @author: erload Tech.
* @licence: MIT
*/

#pragma once

#if defined(GRAPHICS_API_OPENGL)
#include <Rendering/HAL/OpenGL/GLVertexBuffer.h>
#else
#include <Rendering/HAL/None/NoneVertexBuffer.h>
#endif // defined(GRAPHICS_API_OPENGL)

namespace Rendering::HAL
{
#if defined(GRAPHICS_API_OPENGL)
	using VertexBuffer = GLVertexBuffer;
#else
	using VertexBuffer = NoneVertexBuffer;
#endif // defined(GRAPHICS_API_OPENGL)
}
