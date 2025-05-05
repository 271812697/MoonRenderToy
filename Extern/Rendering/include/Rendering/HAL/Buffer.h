/**
* @project: erload
* @author: erload Tech.
* @licence: MIT
*/

#pragma once

#if defined(GRAPHICS_API_OPENGL)
#include <Rendering/HAL/OpenGL/GLBuffer.h>
#else
#include <Rendering/HAL/None/NoneBuffer.h>
#endif // defined(GRAPHICS_API_OPENGL)

namespace Rendering::HAL
{
#if defined(GRAPHICS_API_OPENGL)
	using Buffer = GLBuffer;
#else
	using Buffer = NoneBuffer;
#endif // defined(GRAPHICS_API_OPENGL)
}
