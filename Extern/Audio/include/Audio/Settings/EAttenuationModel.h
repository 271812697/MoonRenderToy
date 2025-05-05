/**
* @project: erload
* @author: erload Tech.
* @licence: MIT
*/

#pragma once

#include <cstdint>

namespace Audio::Settings
{
	/**
	* Spatial sound attenuation models
	*/
	enum class EAttenuationModel : uint8_t
	{
		NONE,
		INVERSE_DISTANCE,
		LINEAR_DISTANCE,
		EXPONENTIAL_DISTANCE
	};
}
