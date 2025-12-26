#pragma once
#include <string>
#include <vector>
#include "Rendering/Resources/Texture.h"


namespace Rendering::Resources::Loaders
{
	/**
	* Handle the Texture creation and destruction
	*/
	class TextureLoader
	{
	public:
		/**
		* Disabled constructor
		*/
		TextureLoader() = delete;

		/**
		* Create a texture from file
		* @param p_filePath
		* @param p_minFilter
		* @param p_magFilter
		* @param p_horizontalWrapMode
		* @param p_verticalWrapMode
		* @param p_generateMipmap
		*/
		static Texture* Create(
			const std::string& p_filepath,
			Rendering::Settings::ETextureFilteringMode p_minFilter,
			Rendering::Settings::ETextureFilteringMode p_magFilter,
			Rendering::Settings::ETextureWrapMode p_horizontalWrapMode,
			Rendering::Settings::ETextureWrapMode p_verticalWrapMode,
			bool p_generateMipmap
		);

		/**
		* Create a texture from a single SDR pixel color
		* @param p_r
		* @param p_g
		* @param p_b
		* @param p_a
		*/
		static Texture* CreatePixel(uint8_t r, uint8_t g, uint8_t b, uint8_t a);

		/**
		* Create a texture from memory
		* @param p_data
		* @param p_width
		* @param p_height
		* @param p_minFilter
		* @param p_magFilter
		* @param p_horizontalWrapMode
		* @param p_verticalWrapMode
		* @param p_generateMipmap
		*/
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

		/**
		* Reload a texture from file
		* @param p_texture
		* @param p_filePath
		* @param p_minFilter
		* @param p_magFilter
		* @param p_horizontalWrapMode
		* @param p_verticalWrapMode
		* @param p_generateMipmap
		*/
		static void Reload(
			Texture& p_texture,
			const std::string& p_filePath,
			Rendering::Settings::ETextureFilteringMode p_minFilter,
			Rendering::Settings::ETextureFilteringMode p_magFilter,
			Rendering::Settings::ETextureWrapMode p_horizontalWrapMode,
			Rendering::Settings::ETextureWrapMode p_verticalWrapMode,
			bool p_generateMipmap
		);

		/**
		* Destroy a texture
		* @param p_textureInstance
		*/
		static bool Destroy(Texture*& p_textureInstance);
	};
}