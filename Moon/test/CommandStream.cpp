#include "CommandStream.h"

#include <cstddef>
#include <functional>
#include <string>
#include <utility>


namespace MOON {



	// ------------------------------------------------------------------------------------------------

	CommandStream::CommandStream(Driver& driver, CircularBuffer& buffer) noexcept
		: mDriver(driver),
		mCurrentBuffer(buffer),
		mDispatcher(driver.getDispatcher())
		, mThreadId(std::this_thread::get_id())
	{

	}

	void CommandStream::execute(void* buffer) {

		mDriver.execute([this, buffer]() {
			Driver& __restrict driver = mDriver;
			CommandBase* __restrict base = static_cast<CommandBase*>(buffer);
			while (base) {
				base = base->execute(driver);
			}
			});

	}

	void CommandStream::queueCommand(std::function<void()> command) {
		new(allocateCommand(CustomCommand::align(sizeof(CustomCommand)))) CustomCommand(std::move(command));
	}

	template<typename... ARGS>
	template<void (Driver::* METHOD)(ARGS...)>
	template<std::size_t... I>
	void CommandType<void (Driver::*)(ARGS...)>::Command<METHOD>::log(std::index_sequence<I...>) noexcept {

	}

	template<typename... ARGS>
	template<void (Driver::* METHOD)(ARGS...)>
	void CommandType<void (Driver::*)(ARGS...)>::Command<METHOD>::log() noexcept {
		log(std::make_index_sequence<std::tuple_size<SavedParameters>::value>{});
	}


	// ------------------------------------------------------------------------------------------------

	void CustomCommand::execute(Driver&, CommandBase* base, intptr_t* next) {
		*next = CustomCommand::align(sizeof(CustomCommand));
		static_cast<CustomCommand*>(base)->mCommand();
		static_cast<CustomCommand*>(base)->~CustomCommand();
	}

} // namespace filament::backend
