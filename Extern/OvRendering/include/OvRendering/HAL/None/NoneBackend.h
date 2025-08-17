

#pragma once

#include <OvRendering/HAL/Common/TBackend.h>

namespace OvRendering::HAL
{
	struct NoneBackendContext {};
	using NoneBackend = TBackend<Settings::EGraphicsBackend::NONE, NoneBackendContext>;
}
