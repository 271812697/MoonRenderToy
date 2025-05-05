/**
* @project: erload
* @author: erload Tech.
* @licence: MIT
*/

#pragma once

#include <Rendering/HAL/Common/TTextureHandle.h>

namespace Rendering::HAL
{
	struct GLTextureHandleContext
	{
		uint32_t id;
	};

	using GLTextureHandle = TTextureHandle<Settings::EGraphicsBackend::OPENGL, GLTextureHandleContext>;
}
