/**
* @project: erload
* @author: erload Tech.
* @licence: MIT
*/

#include <Rendering/HAL/None/NoneIndexBuffer.h>

template<>
Rendering::HAL::NoneIndexBuffer::TIndexBuffer() : TBuffer(Settings::EBufferType::INDEX)
{
}
