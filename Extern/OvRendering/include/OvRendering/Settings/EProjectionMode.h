

#pragma once

#include <cstdint>

namespace OvRendering::Settings
{
	/**
	* Projection modes, mostly used for cameras
	*/
	enum class EProjectionMode : uint8_t
	{
		ORTHOGRAPHIC,
		PERSPECTIVE
	};
}