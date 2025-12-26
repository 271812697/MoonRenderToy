#pragma once

#if defined(GRAPHICS_API_OPENGL)
#include <Rendering/HAL/OpenGL/GLUniformBuffer.h>
#else
#include <OvRendering/HAL/None/NoneUniformBuffer.h>
#endif // defined(GRAPHICS_API_OPENGL)

namespace Rendering::HAL
{
#if defined(GRAPHICS_API_OPENGL)
	using UniformBuffer = GLUniformBuffer;
#else
	using UniformBuffer = NoneUniformBuffer;
#endif // defined(GRAPHICS_API_OPENGL)
}
