#pragma once

#include <stdint.h>
namespace UI::Panels {
	class Framebuffer
	{
	public:

		Framebuffer(uint16_t p_width = 0, uint16_t p_height = 0);


		~Framebuffer();


		void Bind();

		void Unbind();


		void Resize(uint16_t p_width, uint16_t p_height);

		uint32_t GetID();


		uint32_t GetTextureID();


		uint32_t GetRenderBufferID();
		uint16_t width;
		uint16_t height;

	private:
		uint32_t m_bufferID = 0;
		uint32_t m_renderTexture = 0;
		uint32_t m_depthStencilBuffer = 0;
	};
}