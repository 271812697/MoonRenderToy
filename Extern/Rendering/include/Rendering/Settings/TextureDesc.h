/**
* @project: erload
* @author: erload Tech.
* @licence: MIT
*/

#pragma once

#include <Rendering/Settings/ETextureFilteringMode.h>
#include <Rendering/Settings/ETextureWrapMode.h>
#include <Rendering/Settings/EInternalFormat.h>
#include <Rendering/Settings/EFormat.h>
#include <Rendering/Settings/EPixelDataType.h>

namespace Rendering::Settings
{
	/**
	* Structure that holds additional data for mutable textures
	*/
	struct MutableTextureDesc
	{
		EFormat format = EFormat::RGBA;
		EPixelDataType type = EPixelDataType::UNSIGNED_BYTE;
		const void* data = nullptr;
	};

	/**
	* Structure that holds the description of a texture
	*/
	struct TextureDesc
	{
		uint32_t width = 0;
		uint32_t height = 0;
		ETextureFilteringMode minFilter = ETextureFilteringMode::LINEAR_MIPMAP_LINEAR;
		ETextureFilteringMode magFilter = ETextureFilteringMode::LINEAR;
		ETextureWrapMode horizontalWrap = ETextureWrapMode::REPEAT;
		ETextureWrapMode verticalWrap = ETextureWrapMode::REPEAT;
		EInternalFormat internalFormat = EInternalFormat::RGBA;
		bool useMipMaps = true;
		std::optional<MutableTextureDesc> mutableDesc = std::nullopt;
	};
}
