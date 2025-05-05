/**
* @project: erload
* @author: erload Tech.
* @licence: MIT
*/

#pragma once

#if defined(GRAPHICS_API_OPENGL)
#include <Rendering/HAL/OpenGL/GLShaderStorageBuffer.h>
#else
#include <Rendering/HAL/None/NoneShaderStorageBuffer.h>
#endif // defined(GRAPHICS_API_OPENGL)

namespace Rendering::HAL
{
#if defined(GRAPHICS_API_OPENGL)
	using ShaderStorageBuffer = GLShaderStorageBuffer;
#else
	using ShaderStorageBuffer = NoneShaderStorageBuffer;
#endif // defined(GRAPHICS_API_OPENGL)
}
