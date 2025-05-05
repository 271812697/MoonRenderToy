/**
* @project: erload
* @author: erload Tech.
* @licence: MIT
*/

#pragma once

#include <any>
#include <string>

#include <Rendering/Settings/EUniformType.h>

namespace Rendering::Settings
{
	/**
	* Structure that holds information about a uniform
	*/
	struct UniformInfo
	{
		EUniformType type;
		std::string name;
		uint32_t location;
		std::any defaultValue;
	};
}