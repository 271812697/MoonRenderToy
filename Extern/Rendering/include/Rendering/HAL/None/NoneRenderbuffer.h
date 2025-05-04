/**
* @project: erload
* @author: erload Tech.
* @licence: MIT
*/

#pragma once

#include <Rendering/HAL/Common/TRenderbuffer.h>

namespace Rendering::HAL
{
	struct NoneRenderbufferContext
	{
		uint16_t width = 0;
		uint16_t height = 0;
		Settings::EInternalFormat format = Settings::EInternalFormat::RGBA;
		bool allocated = false;
	};

	using NoneRenderbuffer = TRenderbuffer<Settings::EGraphicsBackend::NONE, NoneRenderbufferContext>;
}
