

#pragma once

#include <cstdint>

namespace OvRendering::Settings
{
	/**
	* Enumerate graphics backend implementations
	*/
	enum class EGraphicsBackend : uint8_t
	{
		NONE,
		OPENGL
	};
}