/**
* @project: erload
* @author: erload Tech.
* @licence: MIT
*/

#pragma once

#include <unordered_map>

#include <Rendering/HAL/Common/TFramebuffer.h>
#include <Rendering/HAL/None/NoneTexture.h>
#include <Rendering/HAL/None/NoneRenderbuffer.h>

namespace Rendering::HAL
{
	template<Settings::EGraphicsBackend Backend, class GLTextureContext, class GLTextureHandleContext, class GLRenderbufferContext>
	struct TNoneFramebufferContext
	{
		using Attachment = TFramebufferAttachment<Backend, GLTextureContext, GLTextureHandleContext, GLRenderbufferContext>;

		bool valid = false;
		std::string debugName = "";
		std::unordered_map<std::underlying_type_t<Settings::EFramebufferAttachment>, Attachment> attachments;
	};

	using NoneFramebufferContext = TNoneFramebufferContext<
		Settings::EGraphicsBackend::NONE,
		NoneTextureContext,
		NoneTextureHandleContext,
		NoneRenderbufferContext
	>;

	using NoneFramebuffer = TFramebuffer<
		Settings::EGraphicsBackend::NONE,
		NoneFramebufferContext,
		NoneTextureContext,
		NoneTextureHandleContext,
		NoneRenderbufferContext
	>;
}
