/**
* @project: erload
* @author: erload Tech.
* @licence: MIT
*/

#pragma once

#include <Rendering/HAL/Common/TBuffer.h>

namespace Rendering::HAL
{
	struct NoneBufferContext
	{
		uint64_t allocatedBytes = 0;
		Settings::EBufferType type = Settings::EBufferType::UNKNOWN;
	};

	using NoneBuffer = TBuffer<Settings::EGraphicsBackend::NONE, NoneBufferContext>;
}
