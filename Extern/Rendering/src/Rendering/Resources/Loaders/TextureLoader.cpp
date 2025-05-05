/**
* @project: erload
* @author: erload Tech.
* @licence: MIT
*/



#include <stb_image/stb_image.h>
#include <array>
#include <memory>

#include "Rendering/Resources/Loaders/TextureLoader.h"
#include <Tools/Utils/PathParser.h>

namespace
{
	struct Image
	{
		uint8_t* data;
		int width;
		int height;
		int bpp;

		Image(const std::string& p_filepath)
		{
			stbi_set_flip_vertically_on_load(true);
			data = stbi_load(p_filepath.c_str(), &width, &height, &bpp, 4);
		}

		~Image()
		{
			stbi_image_free(data);
		}

		bool IsValid() const
		{
			return data;
		}

		operator bool() const
		{
			return IsValid();
		}
	};

	void PrepareTexture(
		Rendering::HAL::Texture& p_texture,
		uint8_t* p_data,
		Rendering::Settings::ETextureFilteringMode p_minFilter,
		Rendering::Settings::ETextureFilteringMode p_magFilter,
		uint32_t p_width,
		uint32_t p_height,
		bool p_generateMipmap
	)
	{
		using namespace Rendering::Settings;

		p_texture.Allocate({
			.width = p_width,
			.height = p_height,
			.minFilter = p_minFilter,
			.magFilter = p_magFilter,
			.internalFormat = EInternalFormat::RGBA8,
			.useMipMaps = p_generateMipmap
			});

		p_texture.Upload(p_data, EFormat::RGBA, EPixelDataType::UNSIGNED_BYTE);

		if (p_generateMipmap)
		{
			p_texture.GenerateMipmaps();
		}
	}
}

Rendering::Resources::Texture* Rendering::Resources::Loaders::TextureLoader::Create(const std::string& p_filepath, Rendering::Settings::ETextureFilteringMode p_minFilter, Rendering::Settings::ETextureFilteringMode p_magFilter, bool p_generateMipmap)
{
	if (Image image{ p_filepath })
	{
		auto texture = std::make_unique<HAL::Texture>(Tools::Utils::PathParser::GetElementName(p_filepath));
		PrepareTexture(*texture, image.data, p_minFilter, p_magFilter, image.width, image.height, p_generateMipmap);
		return new Texture{ p_filepath, std::move(texture) };
	}

	return nullptr;
}

Rendering::Resources::Texture* Rendering::Resources::Loaders::TextureLoader::CreatePixel(uint8_t r, uint8_t g, uint8_t b, uint8_t a)
{
	std::array<uint8_t, 4> colorData = { r, g, b, a };

	return Rendering::Resources::Loaders::TextureLoader::CreateFromMemory(
		colorData.data(), 1, 1,
		Rendering::Settings::ETextureFilteringMode::NEAREST,
		Rendering::Settings::ETextureFilteringMode::NEAREST,
		false
	);
}

Rendering::Resources::Texture* Rendering::Resources::Loaders::TextureLoader::CreateFromMemory(uint8_t* p_data, uint32_t p_width, uint32_t p_height, Rendering::Settings::ETextureFilteringMode p_minFilter, Rendering::Settings::ETextureFilteringMode p_magFilter, bool p_generateMipmap)
{
	auto texture = std::make_unique<HAL::Texture>("FromMemory");
	PrepareTexture(*texture, p_data, p_minFilter, p_magFilter, p_width, p_height, p_generateMipmap);
	return new Texture("", std::move(texture));
}

void Rendering::Resources::Loaders::TextureLoader::Reload(Texture& p_texture, const std::string& p_filePath, Rendering::Settings::ETextureFilteringMode p_minFilter, Rendering::Settings::ETextureFilteringMode p_magFilter, bool p_generateMipmap)
{
	if (Image image{ p_filePath })
	{
		auto texture = std::make_unique<HAL::Texture>(Tools::Utils::PathParser::GetElementName(p_filePath));
		PrepareTexture(*texture, image.data, p_minFilter, p_magFilter, image.width, image.height, p_generateMipmap);
		p_texture.SetTexture(std::move(texture));
	}
}

bool Rendering::Resources::Loaders::TextureLoader::Destroy(Texture*& p_textureInstance)
{
	if (p_textureInstance)
	{
		delete p_textureInstance;
		p_textureInstance = nullptr;
		return true;
	}

	return false;
}
