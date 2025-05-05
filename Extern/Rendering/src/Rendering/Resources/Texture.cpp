/**
* @project: erload
* @author: erload Tech.
* @licence: MIT
*/

#include <Debug/Assertion.h>
#include <Rendering/Resources/Texture.h>

Rendering::HAL::Texture& Rendering::Resources::Texture::GetTexture()
{
	ASSERT(m_texture != nullptr, "Trying to access a null Texture");
	return *m_texture;
}

Rendering::Resources::Texture::Texture(const std::string p_path, std::unique_ptr<HAL::Texture>&& p_texture) : path(p_path)
{
	SetTexture(std::move(p_texture));
}

void Rendering::Resources::Texture::SetTexture(std::unique_ptr<HAL::Texture>&& p_texture)
{
	ASSERT(p_texture != nullptr, "Cannot assign an invalid texture!");
	m_texture = std::move(p_texture);
}
