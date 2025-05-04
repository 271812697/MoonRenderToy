/**
* @project: erload
* @author: erload Tech.
* @licence: MIT
*/

#pragma once

#if defined(GRAPHICS_API_OPENGL)
#include <Rendering/HAL/OpenGL/GLFramebuffer.h>
#else
#include <Rendering/HAL/None/NoneFramebuffer.h>
#endif // defined(GRAPHICS_API_OPENGL)

namespace Rendering::HAL
{
#if defined(GRAPHICS_API_OPENGL)
	using Framebuffer = GLFramebuffer;
#else
	using Framebuffer = NoneFramebuffer;
#endif // defined(GRAPHICS_API_OPENGL)
}
