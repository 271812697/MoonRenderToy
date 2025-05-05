/**
* @project: erload
* @author: erload Tech.
* @licence: MIT
*/

#pragma once

#include <cstdint>

#include <Rendering/HAL/Framebuffer.h>

namespace Core::Rendering::FramebufferUtil
{
	/**
	* Prepare the given framebuffer for rendering by attaching
	* the necessary textures and renderbuffers
	* @param p_framebuffer
	* @param p_width
	* @param p_height
	* @param p_useDepth
	* @param p_useStencil
	* @param p_useMipMaps
	*/
	void SetupFramebuffer(
		::Rendering::HAL::Framebuffer& p_framebuffer,
		uint32_t p_width = 0,
		uint32_t p_height = 0,
		bool p_useDepth = true,
		bool p_useStencil = false,
		bool p_useMipMaps = false
	);
}
