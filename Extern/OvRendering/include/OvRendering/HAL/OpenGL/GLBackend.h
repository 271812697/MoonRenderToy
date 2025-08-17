

#pragma once

#include <OvRendering/HAL/Common/TBackend.h>

namespace OvRendering::HAL
{
	struct GLBackendContext {};
	using GLBackend = TBackend<Settings::EGraphicsBackend::OPENGL, GLBackendContext>;
}
