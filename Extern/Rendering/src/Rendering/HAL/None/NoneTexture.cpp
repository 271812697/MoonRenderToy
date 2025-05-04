/**
* @project: erload
* @author: erload Tech.
* @licence: MIT
*/

#include <Debug/Assertion.h>

#include <Rendering/HAL/None//NoneTexture.h>

Tools::Eventing::Event<Rendering::HAL::NoneTexture&> Rendering::HAL::NoneTexture::CreationEvent;
Tools::Eventing::Event<Rendering::HAL::NoneTexture&> Rendering::HAL::NoneTexture::DestructionEvent;

template<>
Rendering::HAL::NoneTexture::TTexture(std::string_view p_debugName) : TTextureHandle{ 0 }
{
	m_textureContext.debugName = p_debugName;
	CreationEvent.Invoke(*this);
}

template<>
Rendering::HAL::NoneTexture::~TTexture()
{
	DestructionEvent.Invoke(*this);
}

template<>
void Rendering::HAL::NoneTexture::Allocate(const Settings::TextureDesc& p_desc)
{
	m_textureContext.desc = p_desc;
	m_textureContext.allocated = true;
}

template<>
bool Rendering::HAL::NoneTexture::IsValid() const
{
	return m_textureContext.allocated;
}

template<>
bool Rendering::HAL::NoneTexture::IsMutable() const
{
	ASSERT(IsValid(), "Cannot check if a texture is mutable before it has been allocated");
	return m_textureContext.desc.mutableDesc.has_value();
}

template<>
void Rendering::HAL::NoneTexture::Upload(const void* p_data, Settings::EFormat p_format, Settings::EPixelDataType p_type)
{
	ASSERT(IsValid(), "Cannot upload data to a texture before it has been allocated");

	if (IsMutable())
	{
		auto& mutableDesc = m_textureContext.desc.mutableDesc.value();
		mutableDesc.format = p_format;
		mutableDesc.type = p_type;
	}
}

template<>
void Rendering::HAL::NoneTexture::Resize(uint32_t p_width, uint32_t p_height)
{
	ASSERT(IsValid(), "Cannot resize a texture before it has been allocated");
	ASSERT(IsMutable(), "Cannot resize an immutable texture");
	
	auto& desc = m_textureContext.desc;

	if (p_width != desc.width || p_height != desc.width)
	{
		desc.width = p_width;
		desc.height = p_height;

		Allocate(desc);
	}
}

template<>
const Rendering::Settings::TextureDesc& Rendering::HAL::NoneTexture::GetDesc() const
{
	ASSERT(IsValid(), "Cannot get the descriptor of a texture before it has been allocated");
	return m_textureContext.desc;
}

template<>
void Rendering::HAL::NoneTexture::GenerateMipmaps() const
{
	ASSERT(IsValid(), "Cannot generate mipmaps for a texture before it has been allocated");
	ASSERT(m_textureContext.desc.useMipMaps, "Cannot generate mipmaps for a texture that doesn't use them");
}

template<>
void Rendering::HAL::NoneTexture::SetBorderColor(const Maths::FVector4& p_color)
{
	ASSERT(IsValid(), "Cannot set border color for a texture before it has been allocated");
}
