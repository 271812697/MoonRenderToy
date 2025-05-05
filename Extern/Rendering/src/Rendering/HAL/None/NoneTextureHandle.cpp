/**
* @project: erload
* @author: erload Tech.
* @licence: MIT
*/

#include <Rendering/HAL/None/NoneTextureHandle.h>

template<>
Rendering::HAL::NoneTextureHandle::TTextureHandle()
{
}

template<>
Rendering::HAL::NoneTextureHandle::TTextureHandle(uint32_t p_id)
{
}

template<>
void Rendering::HAL::NoneTextureHandle::Bind(std::optional<uint32_t> p_slot) const
{
}

template<>
void Rendering::HAL::NoneTextureHandle::Unbind() const
{
}

template<>
uint32_t Rendering::HAL::NoneTextureHandle::GetID() const
{
	return 0;
}
