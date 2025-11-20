#include <glad/glad.h>
#include <Rendering/HAL/OpenGL/GLFramebuffer.h>
#include <Rendering/HAL/OpenGL/GLRenderbuffer.h>
#include <Rendering/HAL/OpenGL/GLTypes.h>

template<>
template<>
void Rendering::HAL::GLFramebuffer::Attach(
	std::shared_ptr<GLRenderbuffer> p_toAttach,
	Settings::EFramebufferAttachment p_attachment,
	uint32_t p_index,
	std::optional<uint32_t> p_layer
)
{

	const auto attachmentIndex = EnumToValue<GLenum>(p_attachment) + static_cast<GLenum>(p_index);
	p_toAttach->Bind();
	glNamedFramebufferRenderbuffer(m_context.id, attachmentIndex, GL_RENDERBUFFER, p_toAttach->GetID());
	m_context.attachments[attachmentIndex] = p_toAttach;
}

template<>
template<>
void Rendering::HAL::GLFramebuffer::Attach(
	std::shared_ptr<GLTexture> p_toAttach,
	Settings::EFramebufferAttachment p_attachment,
	uint32_t p_index,
	std::optional<uint32_t> p_layer
)
{

	const auto attachmentIndex = EnumToValue<GLenum>(p_attachment) + static_cast<GLenum>(p_index);
	constexpr uint32_t kMipMapLevel = 0;

	if (p_layer.has_value())
	{
		glNamedFramebufferTextureLayer(m_context.id, attachmentIndex, p_toAttach->GetID(), kMipMapLevel, p_layer.value());
	}
	else
	{
		
		glNamedFramebufferTexture(m_context.id, attachmentIndex, p_toAttach->GetID(), kMipMapLevel);
	}

	m_context.attachments[attachmentIndex] = p_toAttach;
}

template<>
Rendering::HAL::GLFramebuffer::TFramebuffer(std::string_view p_debugName) :
	m_context{ .debugName = std::string{p_debugName} }
{
	glCreateFramebuffers(1, &m_context.id);
}

template<>
Rendering::HAL::GLFramebuffer::~TFramebuffer()
{
	glDeleteFramebuffers(1, &m_context.id);
}

template<>
void Rendering::HAL::GLFramebuffer::Bind() const
{
	glBindFramebuffer(GL_FRAMEBUFFER, m_context.id);
}

template<>
void Rendering::HAL::GLFramebuffer::Unbind() const
{
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

bool Rendering::HAL::GLFramebuffer::Validate()
{
	const GLenum status = glCheckNamedFramebufferStatus(m_context.id, GL_FRAMEBUFFER);

	if (status != GL_FRAMEBUFFER_COMPLETE)
	{
	
		return m_context.valid = false;
	}

	return m_context.valid = true;
}

template<>
bool Rendering::HAL::GLFramebuffer::IsValid() const
{
	return m_context.valid;
}

template<>
template<>
Tools::Utils::OptRef<Rendering::HAL::GLTexture> Rendering::HAL::GLFramebuffer::GetAttachment(
	Rendering::Settings::EFramebufferAttachment p_attachment,
	uint32_t p_index
) const
{
	const auto attachmentIndex = EnumToValue<GLenum>(p_attachment) + static_cast<GLenum>(p_index);

	if (m_context.attachments.contains(attachmentIndex))
	{
		auto attachment = m_context.attachments.at(attachmentIndex);

		if (auto pval = std::get_if<std::shared_ptr<GLTexture>>(&attachment); pval && *pval)
		{
			return **pval;
		}
	}

	return std::nullopt;
}

template<>
template<>
Tools::Utils::OptRef<Rendering::HAL::GLRenderbuffer> Rendering::HAL::GLFramebuffer::GetAttachment(
	Rendering::Settings::EFramebufferAttachment p_attachment,
	uint32_t p_index
) const
{
	const auto attachmentIndex = EnumToValue<GLenum>(p_attachment) + static_cast<GLenum>(p_index);

	if (m_context.attachments.contains(attachmentIndex))
	{
		auto attachment = m_context.attachments.at(attachmentIndex);

		if (auto pval = std::get_if<std::shared_ptr<GLRenderbuffer>>(&attachment); pval && *pval)
		{
			return **pval;
		}
	}

	return std::nullopt;
}

template<>
void Rendering::HAL::GLFramebuffer::Resize(uint16_t p_width, uint16_t p_height)
{
	

	for (auto& attachment : m_context.attachments)
	{
		if (const auto pval = std::get_if<std::shared_ptr<GLTexture>>(&attachment.second); pval && *pval)
		{
			(*pval)->Resize(p_width, p_height);
		}
		else if (const auto* pval = std::get_if<std::shared_ptr<GLRenderbuffer>>(&attachment.second); pval && *pval)
		{
			(*pval)->Resize(p_width, p_height);
		}
	}
}

template<>
void Rendering::HAL::GLFramebuffer::SetTargetDrawBuffer(std::optional<uint32_t> p_index)
{
	

	if (p_index.has_value())
	{
		glNamedFramebufferDrawBuffer(m_context.id, GL_COLOR_ATTACHMENT0 + p_index.value());
	}
	else
	{
		glNamedFramebufferDrawBuffer(m_context.id, GL_NONE);
	}
}

template<>
void Rendering::HAL::GLFramebuffer::SetTargetReadBuffer(std::optional<uint32_t> p_index)
{
	

	if (p_index.has_value())
	{
		glNamedFramebufferReadBuffer(m_context.id, GL_COLOR_ATTACHMENT0 + p_index.value());
	}
	else
	{
		glNamedFramebufferReadBuffer(m_context.id, GL_NONE);
	}
}

template<>
uint32_t Rendering::HAL::GLFramebuffer::GetID() const
{
	return m_context.id;
}

template<>
std::pair<uint16_t, uint16_t> Rendering::HAL::GLFramebuffer::GetSize(
	Settings::EFramebufferAttachment p_attachment
) const
{
	for (auto& attachment : m_context.attachments)
	{
		if (const auto pval = std::get_if<std::shared_ptr<GLTexture>>(&attachment.second); pval && *pval)
		{
			return {
				(*pval)->GetDesc().width,
				(*pval)->GetDesc().height
			};
		}
		else if (const auto* pval = std::get_if<std::shared_ptr<GLRenderbuffer>>(&attachment.second); pval && *pval)
		{
			return {
				(*pval)->GetWidth(),
				(*pval)->GetHeight()
			};
		}
	}
	return { {}, {} }; // <-- not an emoji
}

template<>
void Rendering::HAL::GLFramebuffer::BlitToBackBuffer(uint16_t p_backBufferWidth, uint16_t p_backBufferHeight) const
{
	
	auto [width, height] = GetSize(Settings::EFramebufferAttachment::COLOR);

	glBlitNamedFramebuffer(
		m_context.id, 0,
		0, 0, width, height,
		0, 0, p_backBufferWidth, p_backBufferHeight,
		GL_COLOR_BUFFER_BIT, GL_LINEAR
	);
}

template<>
void Rendering::HAL::GLFramebuffer::ReadPixels(
	uint32_t p_x,
	uint32_t p_y,
	uint32_t p_width,
	uint32_t p_height,
	Settings::EPixelDataFormat p_format,
	Settings::EPixelDataType p_type,
	void* p_data) const
{

	Bind();
	glReadPixels(
		p_x, p_y,
		p_width,
		p_height,
		EnumToValue<GLenum>(p_format),
		EnumToValue<GLenum>(p_type),
		p_data
	);
	Unbind();
}

template<>
const std::string& Rendering::HAL::GLFramebuffer::GetDebugName() const
{
	return m_context.debugName;
}
