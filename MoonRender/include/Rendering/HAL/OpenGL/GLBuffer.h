#pragma once
#include <Rendering/HAL/Common/TBuffer.h>

namespace Rendering::HAL
{
	struct GLBufferContext
	{
		uint32_t id = 0;
		Settings::EBufferType type = Settings::EBufferType::UNKNOWN;
		uint64_t allocatedBytes = 0;
	};
	
	using GLBuffer = TBuffer<Settings::EGraphicsBackend::OPENGL, GLBufferContext>;
}
