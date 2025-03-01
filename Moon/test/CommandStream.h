#pragma once
#include "CircularBuffer.h"
#include "Dispatcher.h"
#include "Driver.h"
#include "DriverEnums.h"
#include <cstddef>
#include <functional>
#include <tuple>
#include <type_traits>
#include <utility>
#include <thread>

#include <assert.h>
#include <stddef.h>
#include <stdint.h>

// Set to true to print every command out on log.d. This requires RTTI and DEBUG
#define DEBUG_COMMAND_STREAM false
namespace MOON {

	class CommandBase {
		static constexpr size_t FILAMENT_OBJECT_ALIGNMENT = alignof(std::max_align_t);

	protected:
		using Execute = Dispatcher::Execute;

		constexpr explicit CommandBase(Execute execute) noexcept : mExecute(execute) {}

	public:
		// alignment of all Commands in the CommandStream
		static constexpr size_t align(size_t v) {
			return (v + (FILAMENT_OBJECT_ALIGNMENT - 1)) & -FILAMENT_OBJECT_ALIGNMENT;
		}

		// executes this command and returns the next one
		inline CommandBase* execute(Driver& driver) {
			// returning the next command by output parameter allows the compiler to perform the
			// tail-call optimization in the function called by mExecute, however that comes at
			// a cost here (writing and reading the stack at each iteration), in the end it's
			// probably better to pay the cost at just one location.
			intptr_t next;
			mExecute(driver, this, &next);
			return reinterpret_cast<CommandBase*>(reinterpret_cast<intptr_t>(this) + next);
		}
		inline ~CommandBase() noexcept = default;

	private:
		Execute mExecute;
	};

	// ------------------------------------------------------------------------------------------------

	template<typename T, typename Type, typename D, typename ... ARGS>
	constexpr decltype(auto) invoke(Type T::* m, D&& d, ARGS&& ... args) {
		static_assert(std::is_base_of<T, std::decay_t<D>>::value,
			"member function and object not related");
		return (std::forward<D>(d).*m)(std::forward<ARGS>(args)...);
	}

	template<typename M, typename D, typename T, std::size_t... I>
	constexpr decltype(auto) trampoline(M&& m, D&& d, T&& t, std::index_sequence<I...>) {
		return invoke(std::forward<M>(m), std::forward<D>(d), std::get<I>(std::forward<T>(t))...);
	}

	template<typename M, typename D, typename T>
	constexpr decltype(auto) apply(M&& m, D&& d, T&& t) {
		return trampoline(std::forward<M>(m), std::forward<D>(d), std::forward<T>(t),
			std::make_index_sequence< std::tuple_size<std::remove_reference_t<T>>::value >{});
	}

	/*
	 * CommandType<> is just a wrapper class to specialize on a pointer-to-member of Driver
	 * (i.e. a method pointer to a method of Driver of a particular type -- but not the
	 * method itself). We only do that, so we can identify the parameters of that method.
	 * We won't call through that pointer though.
	 */
	template<typename... ARGS>
	struct CommandType;

	template<typename... ARGS>
	struct CommandType<void (Driver::*)(ARGS...)> {

		/*
		 * Command is templated on a specific method of Driver, using CommandType's template
		 * parameter.
		 * Note that we're never calling this method (which is why it doesn't appear in the
		 * template parameter below). The actual call is made through Command::execute().
		 */
		template<void(Driver::*)(ARGS...)>
		class Command : public CommandBase {
			// We use a std::tuple<> to record the arguments passed to the constructor
			using SavedParameters = std::tuple<std::remove_reference_t<ARGS>...>;
			SavedParameters mArgs;

			void log() noexcept;
			template<std::size_t... I> void log(std::index_sequence<I...>) noexcept;

		public:
			template<typename M, typename D>
			static inline void execute(M&& method, D&& driver, CommandBase* base, intptr_t* next) {
				Command* self = static_cast<Command*>(base);
				*next = align(sizeof(Command));

				apply(std::forward<M>(method), std::forward<D>(driver), std::move(self->mArgs));
				self->~Command();
			}

			// A command can be moved
			inline Command(Command&& rhs) noexcept = default;

			template<typename... A>
			inline explicit constexpr Command(Execute execute, A&& ... args)
				: CommandBase(execute), mArgs(std::forward<A>(args)...) {
			}

			// placement new declared as "throw" to avoid the compiler's null-check
			inline void* operator new(std::size_t, void* ptr) {
				assert_invariant(ptr);
				return ptr;
			}
		};
	};

	// convert a method of "class Driver" into a Command<> type
#define COMMAND_TYPE(method) CommandType<decltype(&Driver::method)>::Command<&Driver::method>

// ------------------------------------------------------------------------------------------------

	class CustomCommand : public CommandBase {
		std::function<void()> mCommand;
		static void execute(Driver&, CommandBase* base, intptr_t* next);
	public:
		inline CustomCommand(CustomCommand&& rhs) = default;
		inline explicit CustomCommand(std::function<void()> cmd)
			: CommandBase(execute), mCommand(std::move(cmd)) { }
	};

	// ------------------------------------------------------------------------------------------------
	class NoopCommand : public CommandBase {
		intptr_t mNext;
		static void execute(Driver&, CommandBase* self, intptr_t* next) noexcept {
			*next = static_cast<NoopCommand*>(self)->mNext;
		}
	public:
		inline constexpr explicit NoopCommand(void* next) noexcept
			: CommandBase(execute), mNext(intptr_t((char*)next - (char*)this)) { }
	};

	// ------------------------------------------------------------------------------------------------
	class CommandStream {
		template<typename T>
		struct AutoExecute {
			T closure;
			inline explicit AutoExecute(T&& closure) : closure(std::forward<T>(closure)) {}
			inline ~AutoExecute() { closure(); }
		};

	public:
		CommandStream(Driver& driver, CircularBuffer& buffer) noexcept;

		CommandStream(CommandStream const& rhs) noexcept = delete;
		CommandStream& operator=(CommandStream const& rhs) noexcept = delete;

		CircularBuffer const& getCircularBuffer() const noexcept { return mCurrentBuffer; }

	public:



	public:
		// This is for debugging only. Currently, CircularBuffer can only be written from a
		// single thread. In debug builds we assert this condition.
		// Call this first in the render loop.
		inline void debugThreading() noexcept {

			mThreadId = std::this_thread::get_id();

		}

		void execute(void* buffer);

		/*
		 * queueCommand() allows to queue a lambda function as a command.
		 * This is much less efficient than using the Driver* API.
		 */
		void queueCommand(std::function<void()> command);

		/*
		 * Allocates memory associated to the current CommandStreamBuffer.
		 * This memory will be automatically freed after this command buffer is processed.
		 * IMPORTANT: Destructors ARE NOT called
		 */
		inline void* allocate(size_t size, size_t alignment = 8) noexcept;

		/*
		 * Helper to allocate an array of trivially destructible objects
		 */
		template<typename PodType,
			typename = typename std::enable_if<std::is_trivially_destructible<PodType>::value>::type>
		inline PodType* allocatePod(
			size_t count = 1, size_t alignment = alignof(PodType)) noexcept;

	private:
		inline void* allocateCommand(size_t size) {
			assert(std::this_thread::get_id() == mThreadId);

			return mCurrentBuffer.allocate(size);
		}

		// We use a copy of Dispatcher (instead of a pointer) because this removes one dereference
		// when executing driver commands.
		Driver& __restrict mDriver;
		CircularBuffer& __restrict mCurrentBuffer;
		Dispatcher mDispatcher;


		std::thread::id mThreadId{};


		bool mUsePerformanceCounter = false;
	};

	void* CommandStream::allocate(size_t size, size_t alignment) noexcept {
		// make sure alignment is a power of two
		assert(alignment && !(alignment & alignment - 1));

		// pad the requested size to accommodate NoopCommand and alignment
		const size_t s = CustomCommand::align(sizeof(NoopCommand) + size + alignment - 1);

		// allocate space in the command stream and insert a NoopCommand
		char* const p = (char*)allocateCommand(s);
		new(p) NoopCommand(p + s);

		// calculate the "user" data pointer
		void* data = (void*)((uintptr_t(p) + sizeof(NoopCommand) + alignment - 1) & ~(alignment - 1));
		assert(data >= p + sizeof(NoopCommand));
		return data;
	}

	template<typename PodType, typename>
	PodType* CommandStream::allocatePod(size_t count, size_t alignment) noexcept {
		return static_cast<PodType*>(allocate(count * sizeof(PodType), alignment));
	}
}
