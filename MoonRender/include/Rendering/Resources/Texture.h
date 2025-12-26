#pragma once
#include <cstdint>
#include <string>
#include <memory>

#include <Rendering/Settings/ETextureFilteringMode.h>
#include <Rendering/HAL/Texture.h>

namespace Rendering::Resources
{
	namespace Loaders { class TextureLoader; }

	/**
	* Texture saved on the disk
	*/
	class Texture
	{
		friend class Loaders::TextureLoader;

	public:
		/**
		* Returns the associated HAL::Texture instance
		*/
		HAL::Texture& GetTexture();
		Texture() = default;
		Texture(const std::string p_path, std::unique_ptr<HAL::Texture>&& p_texture);
		~Texture() = default;
		void SetTexture(std::unique_ptr<HAL::Texture>&& p_texture);
	private:


	public:
		const std::string path;

	private:
		std::unique_ptr<HAL::Texture> m_texture;
	};
}