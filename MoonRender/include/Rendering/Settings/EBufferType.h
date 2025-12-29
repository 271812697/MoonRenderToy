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
		UNKNOWN
	};
}
