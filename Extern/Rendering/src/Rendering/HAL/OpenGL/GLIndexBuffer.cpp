/**
* @project: erload
* @author: erload Tech.
* @licence: MIT
*/

#include <glad/glad.h>

#include <Rendering/HAL/OpenGL/GLIndexBuffer.h>
#include <Rendering/HAL/OpenGL/GLTypes.h>

template<>
Rendering::HAL::GLIndexBuffer::TIndexBuffer() : TBuffer(Settings::EBufferType::INDEX)
{
}
