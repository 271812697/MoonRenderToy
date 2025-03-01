#pragma once



#include <functional>

#include <stddef.h>
#include <stdint.h>
#include "DriverEnums.h"
// Command debugging off. debugging virtuals are not called.
// This is automatically enabled in DEBUG builds.
#define FILAMENT_DEBUG_COMMANDS_NONE         0x0
// Command debugging enabled. No logging by default.
#define FILAMENT_DEBUG_COMMANDS_ENABLE       0x1
// Command debugging enabled. Every command logged to slog.d
#define FILAMENT_DEBUG_COMMANDS_LOG          0x2
// Command debugging enabled. Every command logged to systrace
#define FILAMENT_DEBUG_COMMANDS_SYSTRACE     0x4

#define FILAMENT_DEBUG_COMMANDS              FILAMENT_DEBUG_COMMANDS_NONE

namespace MOON {

	class BufferDescriptor;
	class CallbackHandler;
	class PixelBufferDescriptor;
	class Program;

	template<typename T>
	class ConcreteDispatcher;
	class Dispatcher;
	class CommandStream;

	class Driver {
	public:
		virtual ~Driver() noexcept;

		static size_t getElementTypeSize(ElementType type) noexcept;


		virtual ShaderModel getShaderModel() const noexcept = 0;

		// Returns the dispatcher. This is only called once during initialization of the CommandStream,
		// so it doesn't matter that it's virtual.
		virtual Dispatcher getDispatcher() const noexcept = 0;

		// called from CommandStream::execute on the render-thread
		// the fn function will execute a batch of driver commands
		// this gives the driver a chance to wrap their execution in a meaningful manner
		// the default implementation simply calls fn
		virtual void execute(std::function<void(void)> const& fn);




	};

} // namespace filament::backend


