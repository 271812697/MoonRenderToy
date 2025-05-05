/**
* @project: erload
* @author: erload Tech.
* @licence: MIT
*/

#include <glad/glad.h>

#include <Rendering/HAL/OpenGL/GLTextureHandle.h>

template<>
Rendering::HAL::GLTextureHandle::TTextureHandle()
{
}

template<>
Rendering::HAL::GLTextureHandle::TTextureHandle(uint32_t p_id) : m_context{ .id = p_id }
{
}

template<>
void Rendering::HAL::GLTextureHandle::Bind(std::optional<uint32_t> p_slot) const
{
	if (p_slot.has_value())
	{
		glActiveTexture(GL_TEXTURE0 + p_slot.value());
	}

	glBindTexture(GL_TEXTURE_2D, m_context.id);
}

template<>
void Rendering::HAL::GLTextureHandle::Unbind() const
{
	glBindTexture(GL_TEXTURE_2D, 0);
}

template<>
uint32_t Rendering::HAL::GLTextureHandle::GetID() const
{
	return m_context.id;
}
