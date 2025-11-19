#pragma once
#include <cstdint>

namespace Rendering::Settings
{
	/**
	* Texture types
	*/
	enum class ETextureType : uint8_t
	{
		TEXTURE_2D,
		TEXTURE_2DMULSAMPLE,
		TEXTURE_BUFFER,
		TEXTURE_CUBE
	};
}
