/**
* @project: erload
* @author: erload Tech.
* @licence: MIT
*/

#include <Rendering/HAL/OpenGL/GLVertexBuffer.h>

template<>
Rendering::HAL::GLVertexBuffer::TVertexBuffer() : TBuffer(Settings::EBufferType::VERTEX)
{
}
