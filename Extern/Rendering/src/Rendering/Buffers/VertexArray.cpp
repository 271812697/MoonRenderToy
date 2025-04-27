

#include "Rendering/Buffers/VertexArray.h"

Rendering::Buffers::VertexArray::VertexArray()
{
	glGenVertexArrays(1, &m_bufferID);
	glBindVertexArray(m_bufferID);
}

Rendering::Buffers::VertexArray::~VertexArray()
{
	glDeleteVertexArrays(1, &m_bufferID);
}

void Rendering::Buffers::VertexArray::Bind()
{
	glBindVertexArray(m_bufferID);
}

void Rendering::Buffers::VertexArray::Unbind()
{
	glBindVertexArray(0);
}

GLint Rendering::Buffers::VertexArray::GetID()
{
	return m_bufferID;
}
