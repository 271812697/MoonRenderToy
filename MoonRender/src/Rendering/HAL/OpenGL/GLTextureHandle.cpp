#include <glad/glad.h>
#include <Rendering/HAL/OpenGL/GLTextureHandle.h>
#include <Rendering/HAL/OpenGL/GLTypes.h>

template<>
Rendering::HAL::GLTextureHandle::TTextureHandle(Settings::ETextureType p_type) : m_context{
	.type = EnumToValue<GLenum>(p_type)
}
{
}

template<>
Rendering::HAL::GLTextureHandle::TTextureHandle(Settings::ETextureType p_type, uint32_t p_id) : m_context{
	.id = p_id,
	.type = EnumToValue<GLenum>(p_type)
}
{
}

template<>
void Rendering::HAL::GLTextureHandle::Bind(std::optional<uint32_t> p_slot) const
{
	if (p_slot.has_value())
	{
		glBindTextureUnit(p_slot.value(), m_context.id);
	}
	else
	{
		glBindTexture(m_context.type, m_context.id);
	}
}

template<>
void Rendering::HAL::GLTextureHandle::Unbind() const
{
	glBindTexture(m_context.type, 0);
}

template<>
uint32_t Rendering::HAL::GLTextureHandle::GetID() const
{
	return m_context.id;
}

template<>
Rendering::Settings::ETextureType Rendering::HAL::GLTextureHandle::GetType() const
{
	return ValueToEnum<Settings::ETextureType>(m_context.type);
}
