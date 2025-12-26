#pragma once
#include <Rendering/HAL/Common/TVertexArray.h>
#include <Rendering/HAL/OpenGL/GLIndexBuffer.h>
#include <Rendering/HAL/OpenGL/GLVertexBuffer.h>
#include <Tools/Utils/OptRef.h>

namespace Rendering::HAL
{
	struct GLVertexArrayContext
	{
		uint32_t id = 0;
		uint32_t attributeCount = 0;
	};

	using GLVertexArray = TVertexArray<
		Settings::EGraphicsBackend::OPENGL,
		GLVertexArrayContext,
		GLVertexBufferContext,
		GLIndexBufferContext,
		GLBufferContext
	>;
}
