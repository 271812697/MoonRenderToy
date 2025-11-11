#pragma once
#include <cstdint>
#include <OvRendering/HAL/Framebuffer.h>

namespace OvCore::Rendering::FramebufferUtil
{
	void CopyFramebufferColor(
		OvRendering::HAL::Framebuffer& src_framebuffer,
		int src, 
		OvRendering::HAL::Framebuffer& dst_framebuffer, 
		int dst
	);
	void SetupFramebuffer(
		OvRendering::HAL::Framebuffer& p_framebuffer,
		const OvRendering::Settings::TextureDesc& p_textureDesc,
		bool p_useDepth = true,
		bool p_useStencil = false,
		bool useMulSample = false
	);
	void SetupFramebuffer(
		OvRendering::HAL::Framebuffer& p_framebuffer,
		uint32_t p_width = 0,
		uint32_t p_height = 0,
		bool p_useDepth = true,
		bool p_useStencil = false,
		bool p_useMipMaps = false,
		bool useMulSample = false
	);
}
