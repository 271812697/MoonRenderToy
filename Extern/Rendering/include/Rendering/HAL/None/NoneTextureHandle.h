/**
* @project: erload
* @author: erload Tech.
* @licence: MIT
*/

#pragma once

#include <Rendering/HAL/Common/TTextureHandle.h>

namespace Rendering::HAL
{
	struct NoneTextureHandleContext {};
	using NoneTextureHandle = TTextureHandle<Settings::EGraphicsBackend::NONE, NoneTextureHandleContext>;
}
