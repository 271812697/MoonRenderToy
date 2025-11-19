#pragma once
#include <Rendering/HAL/Common/TBackend.h>

namespace Rendering::HAL
{
	struct GLBackendContext {};
	using GLBackend = TBackend<Settings::EGraphicsBackend::OPENGL, GLBackendContext>;
}
