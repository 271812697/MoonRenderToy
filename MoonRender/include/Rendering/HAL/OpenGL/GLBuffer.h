#pragma once
#include <Rendering/HAL/Common/TBuffer.h>

namespace Rendering::HAL
{
	struct GLBufferContext
	{
		uint32_t id = 0;
		Settings::EBufferType type = Settings::EBufferType::UNKNOWN;
		uint64_t allocatedBytes = 0;
		/** GLsync handle of the most recent fence, nullptr when none is active. */
		void* fence = nullptr;
	};
	
	using GLBuffer = TBuffer<Settings::EGraphicsBackend::OPENGL, GLBufferContext>;
}
