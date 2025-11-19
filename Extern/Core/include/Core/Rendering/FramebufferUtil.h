#pragma once
#include <cstdint>
#include <Rendering/HAL/Framebuffer.h>

namespace Core::Rendering::FramebufferUtil
{
	void CopyFramebufferColor(
		::Rendering::HAL::Framebuffer& src_framebuffer,
		int src, 
		::Rendering::HAL::Framebuffer& dst_framebuffer, 
		int dst
	);
	void SetupFramebuffer(
		::Rendering::HAL::Framebuffer& p_framebuffer,
		const ::Rendering::Settings::TextureDesc& p_textureDesc,
		bool p_useDepth = true,
		bool p_useStencil = false,
		bool useMulSample = false
	);
	void SetupFramebuffer(
		::Rendering::HAL::Framebuffer& p_framebuffer,
		uint32_t p_width = 0,
		uint32_t p_height = 0,
		bool p_useDepth = true,
		bool p_useStencil = false,
		bool p_useMipMaps = false,
		bool useMulSample = false
	);
}
