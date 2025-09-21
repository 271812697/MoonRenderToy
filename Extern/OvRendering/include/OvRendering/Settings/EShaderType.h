#pragma once
#include <cstdint>

namespace OvRendering::Settings
{
	/**
	* OpenGL rasterization mode enum wrapper
	*/
	enum class EShaderType : uint8_t
	{
		NONE,
		VERTEX,
		GEOMERTY,
		FRAGMENT,
	};
}
