/*

#include <backend/platforms/OpenGLPlatform.h>

namespace filament::backend {
	class PlatformGlfwGL :public OpenGLPlatform {
	protected:
		// --------------------------------------------------------------------------------------------
		// Platform Interface

		Driver* createDriver(void* sharedGLContext,
			const Platform::DriverConfig& driverConfig) noexcept override;
		int getOSVersion() const noexcept final override { return 0; }
		// --------------------------------------------------------------------------------------------
		// OpenGLPlatform Interface

		void terminate() noexcept override;
		bool isExtraContextSupported() const noexcept override;
		void createContext(bool shared) override;

		SwapChain* createSwapChain(void* nativewindow, uint64_t flags) noexcept override;
		SwapChain* createSwapChain(uint32_t width, uint32_t height, uint64_t flags) noexcept override;
		void destroySwapChain(SwapChain* swapChain) noexcept override;
		bool makeCurrent(ContextType type, SwapChain* drawSwapChain, SwapChain* readSwapChain) override;
		void commit(SwapChain* swapChain) noexcept override;
	protected:
		
	};


}


*/
