#pragma once
#include <Rendering/HAL/Common/TShaderStorageBuffer.h>
#include <Rendering/HAL/OpenGL/GLBuffer.h>

namespace Rendering::HAL
{
	struct GLShaderStorageBufferContext
	{
		uint32_t id = 0;
	};

	using GLShaderStorageBuffer = TShaderStorageBuffer<Settings::EGraphicsBackend::OPENGL, GLShaderStorageBufferContext, GLBufferContext>;
}
