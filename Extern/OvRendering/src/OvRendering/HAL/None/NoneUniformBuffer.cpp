

#include <OvRendering/HAL/None/NoneUniformBuffer.h>

template<>
OvRendering::HAL::NoneUniformBuffer::TUniformBuffer() : NoneBuffer(Settings::EBufferType::UNIFORM)
{
}
