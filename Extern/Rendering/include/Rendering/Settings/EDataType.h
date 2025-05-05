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
	* Enumeration of data types
	*/
	enum class EDataType : uint8_t
	{
		BYTE,
		UNSIGNED_BYTE,
		SHORT,
		UNSIGNED_SHORT,
		INT,
		UNSIGNED_INT,
		FLOAT,
		DOUBLE
	};
}