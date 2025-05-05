/**
* @project: erload
* @author: erload Tech.
* @licence: MIT
*/

#include <glad/glad.h>

#include <Debug/Assertion.h>
#include <Rendering/HAL/OpenGL/GLRenderbuffer.h>
#include <Rendering/HAL/OpenGL/GLTypes.h>

template<>
Rendering::HAL::GLRenderbuffer::TRenderbuffer()
{
	glGenRenderbuffers(1, &m_context.id);
}

template<>
Rendering::HAL::GLRenderbuffer::~TRenderbuffer()
{
	glDeleteBuffers(1, &m_context.id);
}

template<>
void Rendering::HAL::GLRenderbuffer::Bind() const
{
	glBindRenderbuffer(GL_RENDERBUFFER, m_context.id);
}

template<>
void Rendering::HAL::GLRenderbuffer::Unbind() const
{
	glBindRenderbuffer(GL_RENDERBUFFER, 0);
}

template<>
uint32_t Rendering::HAL::GLRenderbuffer::GetID() const
{
	return m_context.id;
}

template<>
void Rendering::HAL::GLRenderbuffer::Allocate(uint16_t p_width, uint16_t p_height, Settings::EInternalFormat p_format)
{
	m_context.width = p_width;
	m_context.height = p_height;
	m_context.format = p_format;

	glNamedRenderbufferStorageEXT(m_context.id, EnumToValue<GLenum>(m_context.format), m_context.width, m_context.height);

	m_context.allocated = true;
}

template<>
bool Rendering::HAL::GLRenderbuffer::IsValid() const
{
	return m_context.allocated;
}

template<>
void Rendering::HAL::GLRenderbuffer::Resize(uint16_t p_width, uint16_t p_height)
{
	ASSERT(IsValid(), "Cannot resize a renderbuffer that has not been allocated");
	Allocate(p_width, p_height, m_context.format);
}


template<>
uint16_t Rendering::HAL::GLRenderbuffer::GetWidth() const
{
	ASSERT(IsValid(), "Cannot get width of an invalid renderbuffer");
	return m_context.width;
}

template<>
uint16_t Rendering::HAL::GLRenderbuffer::GetHeight() const
{
	ASSERT(IsValid(), "Cannot get height of an invalid renderbuffer");
	return m_context.height;
}
