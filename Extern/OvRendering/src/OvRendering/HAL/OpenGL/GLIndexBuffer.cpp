

#include <glad/glad.h>

#include <OvRendering/HAL/OpenGL/GLIndexBuffer.h>
#include <OvRendering/HAL/OpenGL/GLTypes.h>

template<>
OvRendering::HAL::GLIndexBuffer::TIndexBuffer() : TBuffer(Settings::EBufferType::INDEX)
{
}
