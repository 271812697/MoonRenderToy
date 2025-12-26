#pragma once
#include <Rendering/HAL/Common/TVertexBuffer.h>
#include <Rendering/HAL/OpenGL/GLBuffer.h>

namespace Rendering::HAL
{
	struct GLVertexBufferContext{};
	using GLVertexBuffer = TVertexBuffer<Settings::EGraphicsBackend::OPENGL, GLVertexBufferContext, GLBufferContext>;
}
