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
	* Enumerate graphics backend implementations
	*/
	enum class EGraphicsBackend : uint8_t
	{
		NONE,
		OPENGL
	};
}