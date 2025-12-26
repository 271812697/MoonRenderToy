#pragma once

#if defined(GRAPHICS_API_OPENGL)
#include <Rendering/HAL/OpenGL/GLVertexArray.h>
#else
#include <OvRendering/HAL/None/NoneVertexArray.h>
#endif // defined(GRAPHICS_API_OPENGL)

namespace Rendering::HAL
{
#if defined(GRAPHICS_API_OPENGL)
	using VertexArray = GLVertexArray;
#else
	using VertexArray = NoneVertexArray;
#endif // defined(GRAPHICS_API_OPENGL)
}
