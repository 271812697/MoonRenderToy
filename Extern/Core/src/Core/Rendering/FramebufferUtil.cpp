/**
* @project: erload
* @author: erload Tech.
* @licence: MIT
*/

#include<format>

#include <Core/Rendering/FramebufferUtil.h>
#include <Rendering/HAL/Framebuffer.h>
#include <Rendering/HAL/Renderbuffer.h>
#include <Rendering/HAL/Texture.h>

namespace Core::Rendering::FramebufferUtil
{
	void SetupFramebuffer(
		::Rendering::HAL::Framebuffer& p_framebuffer,
		uint32_t p_width,
		uint32_t p_height,
		bool p_useDepth,
		bool p_useStencil,
		bool p_useMipMaps
	)
	{
		using namespace ::Rendering::HAL;
		using namespace ::Rendering::Settings;

		p_width = static_cast<uint16_t>(std::max(1u, p_width));
		p_height = static_cast<uint16_t>(std::max(1u, p_height));

		const auto renderTexture = std::make_shared<Texture>(std::format(
			"{}/Color",
			p_framebuffer.GetDebugName()
		));

		TextureDesc renderTextureDesc{
			.width = p_width,
			.height = p_height,
			.minFilter = p_useMipMaps ? ETextureFilteringMode::LINEAR_MIPMAP_LINEAR : ETextureFilteringMode::LINEAR,
			.magFilter = ETextureFilteringMode::LINEAR,
			.horizontalWrap = ETextureWrapMode::CLAMP_TO_BORDER,
			.verticalWrap = ETextureWrapMode::CLAMP_TO_BORDER,
			.internalFormat = EInternalFormat::RGBA32F,
			.useMipMaps = p_useMipMaps,
			.mutableDesc = MutableTextureDesc{
				.format = EFormat::RGBA,
				.type = EPixelDataType::FLOAT
			}
		};

		renderTexture->Allocate(renderTextureDesc);
		p_framebuffer.Attach(renderTexture, EFramebufferAttachment::COLOR);

		if (p_useDepth || p_useStencil)
		{
			const auto renderbuffer = std::make_shared<Renderbuffer>();
			const auto internalFormat = p_useStencil ? EInternalFormat::DEPTH_STENCIL : EInternalFormat::DEPTH_COMPONENT;
			renderbuffer->Allocate(p_width, p_height, internalFormat);
			if (p_useStencil)
			{
				p_framebuffer.Attach(renderbuffer, EFramebufferAttachment::STENCIL);
			}
			if (p_useDepth)
			{
				p_framebuffer.Attach(renderbuffer, EFramebufferAttachment::DEPTH);
			}
		}

		p_framebuffer.Validate();
	}
}
