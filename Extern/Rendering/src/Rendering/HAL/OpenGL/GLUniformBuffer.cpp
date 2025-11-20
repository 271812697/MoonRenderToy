#include <glad/glad.h>
#include <Rendering/HAL/OpenGL/GLUniformBuffer.h>

template<>
Rendering::HAL::GLUniformBuffer::TUniformBuffer() : GLBuffer(Settings::EBufferType::UNIFORM)
{
}
