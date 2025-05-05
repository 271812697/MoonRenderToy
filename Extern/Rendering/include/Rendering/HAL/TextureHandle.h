/**
* @project: erload
* @author: erload Tech.
* @licence: MIT
*/

#pragma once

#if defined(GRAPHICS_API_OPENGL)
#include <Rendering/HAL/OpenGL/GLTextureHandle.h>
#else
#include <Rendering/HAL/None/NoneTextureHandle.h>
#endif // defined(GRAPHICS_API_OPENGL)

namespace Rendering::HAL
{
#if defined(GRAPHICS_API_OPENGL)
	using TextureHandle = GLTextureHandle;
#else
	using TextureHandle = NoneTextureHandle;
#endif // defined(GRAPHICS_API_OPENGL)
}
