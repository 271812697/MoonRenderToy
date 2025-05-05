/**
* @project: erload
* @author: erload Tech.
* @licence: MIT
*/

#include <Rendering/HAL/None/NoneVertexBuffer.h>

template<>
Rendering::HAL::NoneVertexBuffer::TVertexBuffer() : TBuffer(Settings::EBufferType::VERTEX)
{
}
