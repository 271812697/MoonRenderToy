/**
* @project: erload
* @author: erload Tech.
* @licence: MIT
*/

#include <Rendering/HAL/None/NoneUniformBuffer.h>

template<>
Rendering::HAL::NoneUniformBuffer::TUniformBuffer() : NoneBuffer(Settings::EBufferType::UNIFORM)
{
}
