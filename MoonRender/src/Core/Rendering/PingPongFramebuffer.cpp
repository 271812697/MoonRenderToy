#include <format>
#include <Core/Rendering/PingPongFramebuffer.h>

Core::Rendering::PingPongFramebuffer::PingPongFramebuffer(std::string_view p_debugName) :
	CircularIterator<::Rendering::HAL::Framebuffer, 2>(m_framebuffers),
	m_framebuffers{
		::Rendering::HAL::Framebuffer{std::format("{}PingPong{}", p_debugName, 0)},
		::Rendering::HAL::Framebuffer{std::format("{}PingPong{}", p_debugName, 1)}
	}
{

}

std::array<::Rendering::HAL::Framebuffer, 2>& Core::Rendering::PingPongFramebuffer::GetFramebuffers()
{
	return m_framebuffers;
}
