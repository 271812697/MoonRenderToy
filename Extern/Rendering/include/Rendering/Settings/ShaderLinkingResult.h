/**
* @project: erload
* @author: erload Tech.
* @licence: MIT
*/

#pragma once

#include <string>

namespace Rendering::Settings
{
	/**
	* Structure that contains the result of a shader linking operation
	*/
	struct ShaderLinkingResult
	{
		const bool success;
		const std::string message;
	};
}
