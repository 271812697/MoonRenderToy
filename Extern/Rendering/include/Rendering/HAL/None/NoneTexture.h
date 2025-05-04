/**
* @project: erload
* @author: erload Tech.
* @licence: MIT
*/

#pragma once

#include <Rendering/HAL/Common/TTexture.h>
#include <Rendering/HAL/None/NoneTextureHandle.h>

namespace Rendering::HAL
{
	struct NoneTextureContext
	{
		Rendering::Settings::TextureDesc desc;
		bool allocated = false;
		std::string debugName = "";
	};

	using NoneTexture = TTexture<Settings::EGraphicsBackend::NONE, NoneTextureContext, NoneTextureHandleContext>;
}
