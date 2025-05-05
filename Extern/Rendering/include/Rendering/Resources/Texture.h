/**
* @project: erload
* @author: erload Tech.
* @licence: MIT
*/

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

	private:
		Texture(const std::string p_path, std::unique_ptr<HAL::Texture>&& p_texture);
		~Texture() = default;
		void SetTexture(std::unique_ptr<HAL::Texture>&& p_texture);

	public:
		const std::string path;

	private:
		std::unique_ptr<HAL::Texture> m_texture;
	};
}