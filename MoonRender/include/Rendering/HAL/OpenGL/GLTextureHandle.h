#pragma once
#include <Rendering/HAL/Common/TTextureHandle.h>

namespace Rendering::HAL
{
	struct GLTextureHandleContext
	{
		uint32_t id;
		const uint32_t type;
	};

	using GLTextureHandle = TTextureHandle<Settings::EGraphicsBackend::OPENGL, GLTextureHandleContext>;
}
