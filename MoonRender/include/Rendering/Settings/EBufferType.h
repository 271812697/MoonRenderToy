#pragma once
#include <cstdint>

namespace Rendering::Settings
{
	/**
	* Enumeration of buffer types
	*/
	enum class EBufferType : uint8_t
	{
		VERTEX,
		INDEX,
		UNIFORM,
		SHADER_STORAGE,
		/** Pixel pack buffer: destination of asynchronous glReadPixels. */
		PIXEL_PACK,
		UNKNOWN
	};
}
