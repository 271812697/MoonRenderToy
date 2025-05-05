/**
* @project: erload
* @author: erload Tech.
* @licence: MIT
*/

#pragma once

#include <Rendering/HAL/Common/TTexture.h>
#include <Rendering/HAL/OpenGL/GLTextureHandle.h>

namespace Rendering::HAL
{
	struct GLTextureContext
	{
		Settings::TextureDesc desc;
		bool allocated = false;
		std::string debugName;
	};

	using GLTexture = TTexture<Settings::EGraphicsBackend::OPENGL, GLTextureContext, GLTextureHandleContext>;
}
