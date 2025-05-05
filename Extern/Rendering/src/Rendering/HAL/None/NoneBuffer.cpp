/**
* @project: erload
* @author: erload Tech.
* @licence: MIT
*/

#include <Debug/Assertion.h>
#include <Rendering/HAL/None/NoneBuffer.h>

template<>
Rendering::HAL::NoneBuffer::TBuffer(Settings::EBufferType p_type) : m_buffer{
	.type = p_type
}
{
}

template<>
Rendering::HAL::NoneBuffer::~TBuffer()
{
}

template<>
uint64_t Rendering::HAL::NoneBuffer::Allocate(size_t p_size, Settings::EAccessSpecifier p_usage)
{
	return m_buffer.allocatedBytes = p_size;
}

template<>
void Rendering::HAL::NoneBuffer::Upload(const void* p_data, std::optional<BufferMemoryRange> p_range)
{
	ASSERT(IsValid(), "Trying to upload data to an invalid buffer");
	ASSERT(!IsEmpty(), "Trying to upload data to an empty buffer");
}

template<>
void Rendering::HAL::NoneBuffer::Bind(std::optional<uint32_t> p_index) const
{
	ASSERT(IsValid(), "Cannot bind an invalid buffer");
}

template<>
void Rendering::HAL::NoneBuffer::Unbind() const
{
	ASSERT(IsValid(), "Cannot unbind an invalid buffer");
}

template<>
bool Rendering::HAL::NoneBuffer::IsValid() const
{
	return
		m_buffer.type != Settings::EBufferType::UNKNOWN &&
		m_buffer.allocatedBytes > 0;
}

template<>
bool Rendering::HAL::NoneBuffer::IsEmpty() const
{
	return GetSize() == 0;
}

template<>
uint64_t Rendering::HAL::NoneBuffer::GetSize() const
{
	ASSERT(IsValid(), "Cannot get size of an invalid buffer");
	return m_buffer.allocatedBytes;
}

template<>
uint32_t Rendering::HAL::NoneBuffer::GetID() const
{
	ASSERT(IsValid(), "Cannot get ID of an invalid buffer");
	return 0;
}
