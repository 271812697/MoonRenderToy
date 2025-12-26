#pragma once

#if defined(GRAPHICS_API_OPENGL)
#include <Rendering/HAL/OpenGL/GLShaderProgram.h>
#else
#include <OvRendering/HAL/None/NoneShaderProgram.h>
#endif // defined(GRAPHICS_API_OPENGL)

namespace Rendering::HAL
{
#if defined(GRAPHICS_API_OPENGL)
	using ShaderProgram = GLShaderProgram;
#else
	using ShaderProgram = NoneShaderProgram;
#endif // defined(GRAPHICS_API_OPENGL)
}
