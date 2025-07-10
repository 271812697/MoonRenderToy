
/*

#include "GlfwPlatformGl.h"
#include "../src/opengl/gl_headers.h"
#include <GLFW/glfw3.h>
#include <utils/Logger.h>
#include <utils/Panic.h>
namespace filament::backend {
	struct GlfwSwapChain
	{
		GLFWwindow* window;
	};
	Driver* PlatformGlfwGL::createDriver(void* sharedGLContext, const Platform::DriverConfig& driverConfig) noexcept
	{
	
		int result = bluegl::bind();
		FILAMENT_CHECK_POSTCONDITION(!result) << "Unable to load OpenGL entry points.";

		return OpenGLPlatform::createDefaultDriver(this, sharedGLContext, driverConfig);
	}
	void PlatformGlfwGL::terminate() noexcept
	{
	}
	bool PlatformGlfwGL::isExtraContextSupported() const noexcept
	{
		return false;
	}
	void PlatformGlfwGL::createContext(bool shared)
	{
		
	}
	Platform::SwapChain* PlatformGlfwGL::createSwapChain(void* nativewindow, uint64_t flags) noexcept
	{
		auto* swapChain = new GlfwSwapChain();
		swapChain->window =(GLFWwindow*) nativewindow;
		return (Platform::SwapChain*)swapChain;
	}
	Platform::SwapChain* PlatformGlfwGL::createSwapChain(uint32_t width, uint32_t height, uint64_t flags) noexcept
	{
		return nullptr;
	}
	void PlatformGlfwGL::destroySwapChain(SwapChain* swapChain) noexcept
	{
		auto* glfwSwapChain = (GlfwSwapChain*)swapChain;
		delete glfwSwapChain;
	}
	bool PlatformGlfwGL::makeCurrent(ContextType type, SwapChain* drawSwapChain, SwapChain* readSwapChain)
	{
		auto* glfwSwapChain = (GlfwSwapChain*)drawSwapChain;
		glfwMakeContextCurrent(glfwSwapChain->window);
		return true;
	}
	void PlatformGlfwGL::commit(SwapChain* swapChain) noexcept
	{
		auto* glfwSwapChain = (GlfwSwapChain*)swapChain;
		glfwSwapBuffers(glfwSwapChain->window);
	}
}

*/
