

#pragma once

#include<memory>
#include <vector>
#include"Rendering/Resources/Texture2D.h"
#include "Rendering/Context/Driver.h"

namespace Rendering::Buffers
{
	/**
	* Wraps OpenGL EBO
	*/
	class Framebuffer
	{
	public:
		/**
		* Create the framebuffer
		* @param p_width
		* @param p_height
		*/
		Framebuffer(uint16_t p_width = 0, uint16_t p_height = 0);

		/**
		* Destructor
		*/
		~Framebuffer();
			/**
		* Bind the framebuffer
		*/
		void Bind();
        void Clear() const;
		/**
		* Unbind the framebuffer
		*/
		void Unbind();

		/**
		* Defines a new size for the framebuffer
		* @param p_width
		* @param p_height
		*/
		void Resize(uint16_t p_width, uint16_t p_height);

		/**
		* Add a new color texture
		* @param p_width
		* @param p_height
		*/
        void AddColorTexture(uint16_t count, bool multisample = false);
		void SetColorTexture(GLenum index, GLuint texture_2d);
		void SetColorTexture(GLenum index, GLuint texture_cubemap, GLuint face);
		void AddDepStTexture(bool multisample = false);
		void AddDepStRenderBuffer(bool multisample = false);
		void AddDepthCubemap();
        void SetDrawBuffers() const;
        static void CopyColor(const Framebuffer& fr, GLuint fr_idx, const Framebuffer& to, GLuint to_idx);
		/**
		* Returns the ID of the OpenGL framebuffer
		*/
		uint32_t GetID()const;
        const Rendering::Resources::Texture2D& GetDepthTexture() const;
        void Clear(GLint index) const;
		/**
		* Returns the ID of the OpenGL render texture
		*/
		uint32_t GetTextureID(uint16_t index = 0);
        const Rendering::Resources::Texture2D& GetColorTexture(GLenum index) const;

		/**
		* Returns the ID of the OpenGL render buffer
		*/
		uint32_t GetRenderBufferID();

	private:
		uint32_t width=0, height=0;
        std::vector<Rendering::Resources::Texture2D>color_attachments;
        std::unique_ptr<Rendering::Resources::Texture2D> m_deptexture;


		uint32_t m_bufferID = 0;
		uint32_t m_depthStencilBuffer = 0;
		bool is_multisample = false;
	};
}