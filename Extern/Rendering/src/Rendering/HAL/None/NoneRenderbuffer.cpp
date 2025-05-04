/**
* @project: erload
* @author: erload Tech.
* @licence: MIT
*/

#include <Debug/Assertion.h>
#include <Rendering/HAL/None/NoneRenderbuffer.h>

template<>
Rendering::HAL::NoneRenderbuffer::TRenderbuffer()
{
}

template<>
Rendering::HAL::NoneRenderbuffer::~TRenderbuffer()
{
}

template<>
void Rendering::HAL::NoneRenderbuffer::Bind() const
{
}

template<>
void Rendering::HAL::NoneRenderbuffer::Unbind() const
{
}

template<>
uint32_t Rendering::HAL::NoneRenderbuffer::GetID() const
{
	return 0;
}

template<>
void Rendering::HAL::NoneRenderbuffer::Allocate(uint16_t p_width, uint16_t p_height, Settings::EInternalFormat p_format)
{
	m_context.width = p_width;
	m_context.height = p_height;
	m_context.format = p_format;
	m_context.allocated = true;
}

template<>
bool Rendering::HAL::NoneRenderbuffer::IsValid() const
{
	return m_context.allocated;
}

template<>
void Rendering::HAL::NoneRenderbuffer::Resize(uint16_t p_width, uint16_t p_height)
{
	ASSERT(IsValid(), "Cannot resize a renderbuffer that has not been allocated");
}

template<>
uint16_t Rendering::HAL::NoneRenderbuffer::GetWidth() const
{
	ASSERT(IsValid(), "Cannot get the height of a renderbuffer that has not been allocated");
	return m_context.width;
}

template<>
uint16_t Rendering::HAL::NoneRenderbuffer::GetHeight() const
{
	ASSERT(IsValid(), "Cannot get the height of a renderbuffer that has not been allocated");
	return m_context.height;
}
