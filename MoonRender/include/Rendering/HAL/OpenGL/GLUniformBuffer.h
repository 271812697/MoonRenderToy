#pragma once
#include <Rendering/HAL/Common/TUniformBuffer.h>
#include <Rendering/HAL/OpenGL/GLBuffer.h>

namespace Rendering::HAL
{
	struct GLUniformBufferContext {};
	using GLUniformBuffer = TUniformBuffer<Settings::EGraphicsBackend::OPENGL, GLUniformBufferContext, GLBufferContext>;
}
