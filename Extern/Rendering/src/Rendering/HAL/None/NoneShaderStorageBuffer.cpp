/**
* @project: erload
* @author: erload Tech.
* @licence: MIT
*/

#include <Rendering/HAL/None/NoneShaderStorageBuffer.h>

template<>
Rendering::HAL::NoneShaderStorageBuffer::TShaderStorageBuffer() : NoneBuffer(Settings::EBufferType::SHADER_STORAGE)
{
}
