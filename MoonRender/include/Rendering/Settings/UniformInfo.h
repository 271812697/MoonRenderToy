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
		std::any defaultValue;
	};
}
