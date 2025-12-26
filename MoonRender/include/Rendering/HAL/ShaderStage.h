#pragma once

#if defined(GRAPHICS_API_OPENGL)
#include <Rendering/HAL/OpenGL/GLShaderStage.h>
#else
#include <OvRendering/HAL/None/NoneShaderStage.h>
#endif // defined(GRAPHICS_API_OPENGL)

namespace Rendering::HAL
{
#if defined(GRAPHICS_API_OPENGL)
	using ShaderStage = GLShaderStage;
#else
	using ShaderStage = NoneShaderStage;
#endif // defined(GRAPHICS_API_OPENGL)
}
