/**
* @project: erload
* @author: erload Tech.
* @licence: MIT
*/

#pragma once

#include <Rendering/HAL/Common/TVertexArray.h>
#include <Rendering/HAL/None/NoneIndexBuffer.h>
#include <Rendering/HAL/None/NoneVertexBuffer.h>

namespace Rendering::HAL
{
	struct NoneVertexArrayContext
	{
		uint32_t attributeCount = 0;
	};

	using NoneVertexArray = TVertexArray<
		Settings::EGraphicsBackend::NONE,
		NoneVertexArrayContext,
		NoneVertexBufferContext,
		NoneIndexBufferContext,
		NoneBufferContext
	>;
}
