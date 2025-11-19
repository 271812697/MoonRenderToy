#include <glad/glad.h>

#include <Rendering/HAL/OpenGL/GLVertexArray.h>
#include <Rendering/HAL/OpenGL/GLTypes.h>
#include <assert.h>

namespace
{
	uint32_t GetDataTypeSizeInBytes(Rendering::Settings::EDataType p_type)
	{
		switch (p_type)
		{
		case Rendering::Settings::EDataType::BYTE: return sizeof(GLbyte);
		case Rendering::Settings::EDataType::UNSIGNED_BYTE: return sizeof(GLubyte);
		case Rendering::Settings::EDataType::SHORT: return sizeof(GLshort);
		case Rendering::Settings::EDataType::UNSIGNED_SHORT: return sizeof(GLushort);
		case Rendering::Settings::EDataType::INT: return sizeof(GLint);
		case Rendering::Settings::EDataType::UNSIGNED_INT: return sizeof(GLuint);
		case Rendering::Settings::EDataType::FLOAT: return sizeof(GLfloat);
		case Rendering::Settings::EDataType::DOUBLE: return sizeof(GLdouble);
		default: return 0;
		}
	}

	uint32_t CalculateTotalVertexSize(std::span<const Rendering::Settings::VertexAttribute> p_attributes)
	{
		uint32_t result = 0;

		for (const auto& attribute : p_attributes)
		{
			result += GetDataTypeSizeInBytes(attribute.type) * attribute.count;
		}

		return result;
	}
}

template<>
Rendering::HAL::GLVertexArray::TVertexArray()
{
	glCreateVertexArrays(1, &m_context.id);
}

template<>
Rendering::HAL::GLVertexArray::~TVertexArray()
{
	glDeleteVertexArrays(1, &m_context.id);
}

template<>
bool Rendering::HAL::GLVertexArray::IsValid() const
{
	return m_context.attributeCount > 0;
}

template<>
void Rendering::HAL::GLVertexArray::SetLayout(
	Settings::VertexAttributeLayout p_attributes,
	VertexBuffer& p_vertexBuffer,
	IndexBuffer& p_indexBuffer
)
{
	assert(!IsValid()&& "Vertex array layout already set");

	Bind();
	if (p_indexBuffer.IsValid()) {
		p_indexBuffer.Bind();
	}
	p_vertexBuffer.Bind();

	uint32_t attributeIndex = 0;

	const uint32_t totalSize = CalculateTotalVertexSize(p_attributes);
	intptr_t currentOffset = 0;

	for (const auto& attribute : p_attributes)
	{
		assert(attribute.count >= 1 && attribute.count <= 4, "Attribute count must be between 1 and 4");

		glEnableVertexAttribArray(attributeIndex);

		glVertexAttribPointer(
			static_cast<GLuint>(attributeIndex),
			static_cast<GLint>(attribute.count),
			EnumToValue<GLenum>(attribute.type),
			static_cast<GLboolean>(attribute.normalized),
			static_cast<GLsizei>(totalSize),
			reinterpret_cast<const GLvoid*>(currentOffset)
		);

		const uint64_t typeSize = GetDataTypeSizeInBytes(attribute.type);
		const uint64_t attributeSize = typeSize * attribute.count;
		currentOffset += attributeSize;
		++attributeIndex;
		++m_context.attributeCount;
	}

	Unbind();
	if (p_indexBuffer.IsValid()) {
		p_indexBuffer.Unbind();
	}
	p_vertexBuffer.Unbind();
}

template<>
void Rendering::HAL::GLVertexArray::ResetLayout()
{
	assert(IsValid()&&"Vertex array layout not already set");

	Bind();
	for (uint32_t i = 0; i < m_context.attributeCount; ++i)
	{
		glDisableVertexAttribArray(i);
	}
	m_context.attributeCount = 0;
	Unbind();
}

template<>
void Rendering::HAL::GLVertexArray::Bind() const
{
	glBindVertexArray(m_context.id);
}

template<>
void Rendering::HAL::GLVertexArray::Unbind() const
{
	glBindVertexArray(0);
}

template<>
uint32_t Rendering::HAL::GLVertexArray::GetID() const
{
	return m_context.id;
}
