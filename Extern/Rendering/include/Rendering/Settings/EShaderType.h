/**
* @project: erload
* @author: erload Tech.
* @licence: MIT
*/

#pragma once

#include <cstdint>

namespace Rendering::Settings
{
	/**
	* OpenGL rasterization mode enum wrapper
	*/
	enum class EShaderType : uint8_t
	{
		NONE,
		VERTEX,
		FRAGMENT,
	};
}
