

#pragma once

#include <OvRendering/HAL/Common/TUniformBuffer.h>
#include <OvRendering/HAL/OpenGL/GLBuffer.h>

namespace OvRendering::HAL
{
	struct GLUniformBufferContext {};
	using GLUniformBuffer = TUniformBuffer<Settings::EGraphicsBackend::OPENGL, GLUniformBufferContext, GLBufferContext>;
}
