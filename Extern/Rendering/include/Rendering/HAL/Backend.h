/**
* @project: erload
* @author: erload Tech.
* @licence: MIT
*/

#pragma once

#if defined(GRAPHICS_API_OPENGL)
#include <Rendering/HAL/OpenGL/GLBackend.h>
#else
#include <Rendering/HAL/None/NoneBackend.h>
#endif // defined(GRAPHICS_API_OPENGL)

namespace Rendering::HAL
{
#if defined(GRAPHICS_API_OPENGL)
	using Backend = GLBackend;
#else
	using Backend = NoneBackend;
#endif // defined(GRAPHICS_API_OPENGL)
}
