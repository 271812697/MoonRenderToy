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
	* Structure that holds the result of a shader compilation
	*/
	struct ShaderCompilationResult
	{
		bool success;
		std::string message;
	};
}
