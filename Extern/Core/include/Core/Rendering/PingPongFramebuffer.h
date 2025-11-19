#pragma once
#include <array>
#include <Rendering/HAL/Framebuffer.h>
#include <Tools/Utils/CircularIterator.h>

namespace Core::Rendering
{
	/**
	* Convenient ping-pong buffer holding two framebuffers
	*/
	class PingPongFramebuffer : public Tools::Utils::CircularIterator<::Rendering::HAL::Framebuffer, 2>
	{
	public:
		/**
		* Create a ping pong buffer
		* @param p_debugName
		*/
		PingPongFramebuffer(std::string_view p_debugName = std::string_view{});

		/**
		* Return the two framebuffers
		*/
		std::array<::Rendering::HAL::Framebuffer, 2>& GetFramebuffers();

	private:
		std::array<::Rendering::HAL::Framebuffer, 2> m_framebuffers;
	};
}
