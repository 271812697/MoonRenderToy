/**
* @project: erload
* @author: erload Tech.
* @licence: MIT
*/

#pragma once

#include <cstdint>

namespace Rendering::Settings
{
	/**
	* Enumeration of framebuffer attachments
	*/
	enum class EFramebufferAttachment : uint8_t
	{
		COLOR,
		DEPTH,
		STENCIL,
		DEPTH_STENCIL
	};
}
