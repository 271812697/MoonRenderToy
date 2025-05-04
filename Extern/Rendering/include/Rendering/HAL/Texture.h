/**
* @project: erload
* @author: erload Tech.
* @licence: MIT
*/

#pragma once

#if defined(GRAPHICS_API_OPENGL)
#include <Rendering/HAL/OpenGL/GLTexture.h>
#else
#include <Rendering/HAL/None/NoneTexture.h>
#endif // defined(GRAPHICS_API_OPENGL)

namespace Rendering::HAL
{
#if defined(GRAPHICS_API_OPENGL)
	using Texture = GLTexture;
#else
	using Texture = NoneTexture;
#endif // defined(GRAPHICS_API_OPENGL)
}
