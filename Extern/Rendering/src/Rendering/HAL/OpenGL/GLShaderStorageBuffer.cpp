/**
* @project: erload
* @author: erload Tech.
* @licence: MIT
*/

#include <glad/glad.h>

#include <Rendering/HAL/OpenGL/GLTypes.h>
#include <Rendering/HAL/OpenGL/GLShaderStorageBuffer.h>

template<>
Rendering::HAL::GLShaderStorageBuffer::TShaderStorageBuffer() : GLBuffer(Settings::EBufferType::SHADER_STORAGE)
{
}
