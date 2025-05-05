/**
* @project: erload
* @author: erload Tech.
* @licence: MIT
*/

#pragma once

#include <Rendering/HAL/Common/TBackend.h>

namespace Rendering::HAL
{
	struct NoneBackendContext {};
	using NoneBackend = TBackend<Settings::EGraphicsBackend::NONE, NoneBackendContext>;
}
