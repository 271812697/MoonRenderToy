#pragma once
#include <string>
#include <vector>
#include "Rendering/Resources/Texture.h"


namespace Rendering::Resources::Loaders
{

	class TextureLoader
	{
	public:
		TextureLoader() = delete;


		static Texture* Create(
			const std::string& p_filepath,
			Rendering::Settings::ETextureFilteringMode p_minFilter,
			Rendering::Settings::ETextureFilteringMode p_magFilter,
			Rendering::Settings::ETextureWrapMode p_horizontalWrapMode,
			Rendering::Settings::ETextureWrapMode p_verticalWrapMode,
			bool p_generateMipmap
		);

		static Texture* CreatePixel(uint8_t r, uint8_t g, uint8_t b, uint8_t a);


		static Texture* CreateFromMemory(
			uint8_t* p_data,
			uint32_t p_width,
			uint32_t p_height,
			Rendering::Settings::ETextureFilteringMode p_minFilter,
			Rendering::Settings::ETextureFilteringMode p_magFilter,
			Rendering::Settings::ETextureWrapMode p_horizontalWrapMode,
			Rendering::Settings::ETextureWrapMode p_verticalWrapMode,
			bool p_generateMipmap
		);

		static void Reload(
			Texture& p_texture,
			const std::string& p_filePath,
			Rendering::Settings::ETextureFilteringMode p_minFilter,
			Rendering::Settings::ETextureFilteringMode p_magFilter,
			Rendering::Settings::ETextureWrapMode p_horizontalWrapMode,
			Rendering::Settings::ETextureWrapMode p_verticalWrapMode,
			bool p_generateMipmap
		);

		static bool Destroy(Texture*& p_textureInstance);
	};
}