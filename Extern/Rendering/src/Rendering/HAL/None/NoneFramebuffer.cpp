/**
* @project: erload
* @author: erload Tech.
* @licence: MIT
*/

#include <Debug/Assertion.h>
#include <Rendering/HAL/None/NoneFramebuffer.h>
#include <Rendering/HAL/None/NoneRenderbuffer.h>

template<>
template<>
void Rendering::HAL::NoneFramebuffer::Attach(std::shared_ptr<NoneRenderbuffer> p_toAttach, Settings::EFramebufferAttachment p_attachment, uint32_t p_index)
{
	ASSERT(p_toAttach != nullptr, "Cannot attach a null renderbuffer");
	const auto index = std::underlying_type_t<Settings::EFramebufferAttachment>(p_attachment);
	m_context.attachments[index] = p_toAttach;
}

template<>
template<>
void Rendering::HAL::NoneFramebuffer::Attach(std::shared_ptr<NoneTexture> p_toAttach, Settings::EFramebufferAttachment p_attachment, uint32_t p_index)
{
	ASSERT(p_toAttach != nullptr, "Cannot attach a null texture");
	const auto index = std::underlying_type_t<Settings::EFramebufferAttachment>(p_attachment);
	m_context.attachments[index] = p_toAttach;
}

template<>
Rendering::HAL::NoneFramebuffer::TFramebuffer(std::string_view p_debugName) :
	m_context{ .debugName = std::string{p_debugName} }
{
}

template<>
Rendering::HAL::NoneFramebuffer::~TFramebuffer()
{
}

template<>
void Rendering::HAL::NoneFramebuffer::Bind() const
{
}

template<>
void Rendering::HAL::NoneFramebuffer::Unbind() const
{
}

bool Rendering::HAL::NoneFramebuffer::Validate()
{
	return m_context.valid = true;
}

template<>
bool Rendering::HAL::NoneFramebuffer::IsValid() const
{
	return m_context.valid;
}

template<>
template<>
Tools::Utils::OptRef<Rendering::HAL::NoneTexture> Rendering::HAL::NoneFramebuffer::GetAttachment(Rendering::Settings::EFramebufferAttachment p_attachment, uint32_t p_index) const
{
	const auto index = std::underlying_type_t<Settings::EFramebufferAttachment>(p_attachment);

	if (m_context.attachments.contains(index))
	{
		auto attachment = m_context.attachments.at(index);

		if (auto pval = std::get_if<std::shared_ptr<NoneTexture>>(&attachment); pval && *pval)
		{
			return **pval;
		}
	}

	return std::nullopt;
}

template<>
template<>
Tools::Utils::OptRef<Rendering::HAL::NoneRenderbuffer> Rendering::HAL::NoneFramebuffer::GetAttachment(Rendering::Settings::EFramebufferAttachment p_attachment, uint32_t p_index) const
{
	const auto index = std::underlying_type_t<Settings::EFramebufferAttachment>(p_attachment);

	if (m_context.attachments.contains(index))
	{
		auto attachment = m_context.attachments.at(index);

		if (auto pval = std::get_if<std::shared_ptr<NoneRenderbuffer>>(&attachment); pval && *pval)
		{
			return **pval;
		}
	}

	return std::nullopt;
}

template<>
void Rendering::HAL::NoneFramebuffer::Resize(uint16_t p_width, uint16_t p_height)
{
	ASSERT(IsValid(), "Cannot resize an invalid framebuffer");

	for (auto& attachment : m_context.attachments)
	{
		if (const auto pval = std::get_if<std::shared_ptr<NoneTexture>>(&attachment.second); pval && *pval)
		{
			(*pval)->Resize(p_width, p_height);
		}
		else if (const auto* pval = std::get_if<std::shared_ptr<NoneRenderbuffer>>(&attachment.second); pval && *pval)
		{
			(*pval)->Resize(p_width, p_height);
		}
	}
}

template<>
void Rendering::HAL::NoneFramebuffer::SetTargetDrawBuffer(std::optional<uint32_t> p_index)
{
	ASSERT(IsValid(), "Cannot set target draw buffer on an invalid framebuffer");
}

template<>
void Rendering::HAL::NoneFramebuffer::SetTargetReadBuffer(std::optional<uint32_t> p_index)
{
	ASSERT(IsValid(), "Cannot set target read buffer on an invalid framebuffer");
}

template<>
uint32_t Rendering::HAL::NoneFramebuffer::GetID() const
{
	return 0;
}

template<>
std::pair<uint16_t, uint16_t> Rendering::HAL::NoneFramebuffer::GetSize(Settings::EFramebufferAttachment p_attachment) const
{
	return { {}, {} }; // <-- I swear it's not an emoji
}

template<>
void Rendering::HAL::NoneFramebuffer::BlitToBackBuffer(uint16_t p_backBufferWidth, uint16_t p_backBufferHeight) const
{
	ASSERT(IsValid(), "Cannot blit an invalid framebuffer to the back buffer");
}

template<>
void Rendering::HAL::NoneFramebuffer::ReadPixels(
	uint32_t p_x,
	uint32_t p_y,
	uint32_t p_width,
	uint32_t p_height,
	Settings::EPixelDataFormat p_format,
	Settings::EPixelDataType p_type,
	void* p_data) const
{
	ASSERT(IsValid(), "Cannot read pixels from an invalid framebuffer");
}
