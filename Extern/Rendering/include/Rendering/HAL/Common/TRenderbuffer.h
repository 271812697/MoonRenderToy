#pragma once
#include <Rendering/Settings/EInternalFormat.h>
#include <Rendering/Settings/EGraphicsBackend.h>

namespace Rendering::HAL
{
	/**
	* Represents a renderbuffer, used to store render data for the graphics backend to use.
	*/
	template<Settings::EGraphicsBackend Backend, class Context>
	class TRenderbuffer final
	{
	public:
		/**
		* Create the render buffer
		*/
		TRenderbuffer(bool multisample);

		/**
		* Destructor
		*/
		~TRenderbuffer();

		/**
		* Bind the framebuffer
		*/
		void Bind() const;

		/**
		* Unbind the framebuffer
		*/
		void Unbind() const;

		/**
		* Returns the ID of the render buffer object
		*/
		uint32_t GetID() const;

		/**
		* Uploads the buffer configuration to the GPU
		* @param p_width
		* @param p_height
		* @param p_format
		*/
		void Allocate(uint16_t p_width, uint16_t p_height, Settings::EInternalFormat p_format);

		/**
		* Returns true if the render buffer has been properly allocated
		*/
		bool IsValid() const;

		/**
		* Resizes the render buffer
		* @param p_width
		* @param p_height
		*/
		void Resize(uint16_t p_width, uint16_t p_height);

		/**
		* Returns the width of the framebuffer
		*/
		uint16_t GetWidth() const;

		/**
		* Returns the width of the framebuffer
		*/
		uint16_t GetHeight() const;

	private:
		bool isMultisample = false;
		Context m_context;
	};
}
