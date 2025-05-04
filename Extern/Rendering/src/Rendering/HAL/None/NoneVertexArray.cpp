/**
* @project: erload
* @author: erload Tech.
* @licence: MIT
*/

#include <Debug/Assertion.h>
#include <Rendering/HAL/None/NoneVertexArray.h>

template<>
Rendering::HAL::NoneVertexArray::TVertexArray()
{
}

template<>
Rendering::HAL::NoneVertexArray::~TVertexArray()
{
}

template<>
void Rendering::HAL::NoneVertexArray::Bind() const
{
}

template<>
void Rendering::HAL::NoneVertexArray::Unbind() const
{
}

template<>
bool Rendering::HAL::NoneVertexArray::IsValid() const
{
	return m_context.attributeCount > 0;
}

template<>
void Rendering::HAL::NoneVertexArray::SetLayout(
	Settings::VertexAttributeLayout p_attributes,
	VertexBuffer& p_vertexBuffer,
	IndexBuffer& p_indexBuffer
)
{
	ASSERT(!IsValid(), "Vertex array layout already set");

	for (const auto& attribute : p_attributes)
	{
		ASSERT(attribute.count >= 1 && attribute.count <= 4, "Attribute count must be between 1 and 4");
		++m_context.attributeCount;
	}
}

template<>
void Rendering::HAL::NoneVertexArray::ResetLayout()
{
	ASSERT(IsValid(), "Vertex array layout not already set");
	m_context.attributeCount = 0;
}

template<>
uint32_t Rendering::HAL::NoneVertexArray::GetID() const
{
	return 0;
}
