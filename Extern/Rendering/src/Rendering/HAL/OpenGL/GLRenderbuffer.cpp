#include <glad/glad.h>
#include <Rendering/HAL/OpenGL/GLRenderbuffer.h>
#include <Rendering/HAL/OpenGL/GLTypes.h>

template<>
Rendering::HAL::GLRenderbuffer::TRenderbuffer(bool flag)
{
	glCreateRenderbuffers(1, &m_context.id);
	isMultisample = flag;
}

template<>
Rendering::HAL::GLRenderbuffer::~TRenderbuffer()
{
	glDeleteRenderbuffers(1, &m_context.id);
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
	if (isMultisample) {

		glNamedRenderbufferStorageMultisample(m_context.id,4, EnumToValue<GLenum>(m_context.format), m_context.width, m_context.height);
	}
	else {
		glNamedRenderbufferStorage(m_context.id, EnumToValue<GLenum>(m_context.format), m_context.width, m_context.height);
	}
	

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
	
	Allocate(p_width, p_height, m_context.format);
}


template<>
uint16_t Rendering::HAL::GLRenderbuffer::GetWidth() const
{
	
	return m_context.width;
}

template<>
uint16_t Rendering::HAL::GLRenderbuffer::GetHeight() const
{

	return m_context.height;
}
