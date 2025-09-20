#pragma once
#include "CircularBuffer.h"
#include "Dispatcher.h"
#include "Driver.h"
#include <cstddef>
#include <functional>
#include <tuple>
#include <type_traits>
#include <utility>
#include <thread>
#include <assert.h>
#include <stddef.h>
#include <stdint.h>
namespace MOON
{
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
			intptr_t next;
			mExecute(driver, this, &next);
			return reinterpret_cast<CommandBase*>(reinterpret_cast<intptr_t>(this) + next);
		}
		inline ~CommandBase() noexcept = default;
	private:
		Execute mExecute;
	};
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
	template<typename... ARGS>
	struct CommandType;
	template<typename... ARGS>
	struct CommandType<void (Driver::*)(ARGS...)> {
		template<void(Driver::*)(ARGS...)>
		class Command : public CommandBase {
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
			inline Command(Command&& rhs) noexcept = default;
			template<typename... A>
			inline explicit constexpr Command(Execute execute, A&& ... args)
				: CommandBase(execute), mArgs(std::forward<A>(args)...) {
			}
			inline void* operator new(std::size_t, void* ptr) {
				assert(ptr != nullptr);
				return ptr;
			}
		};
	};
#define COMMAND_TYPE(method) CommandType<decltype(&Driver::method)>::Command<&Driver::method>
	class CustomCommand : public CommandBase {
		std::function<void()> mCommand;
		static void execute(Driver&, CommandBase* base, intptr_t* next);
	public:
		inline CustomCommand(CustomCommand&& rhs) = default;
		inline explicit CustomCommand(std::function<void()> cmd) : CommandBase(execute), mCommand(std::move(cmd)) { }
	};
	class NoopCommand : public CommandBase {
		intptr_t mNext;
		static void execute(Driver&, CommandBase* self, intptr_t* next) noexcept {
			*next = static_cast<NoopCommand*>(self)->mNext;
		}
	public:
		inline constexpr explicit NoopCommand(void* next) noexcept
			: CommandBase(execute), mNext(intptr_t((char*)next - (char*)this)) { }
	};
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
		void test(int val);
	public:
		inline void debugThreading() noexcept {
			mThreadId = std::this_thread::get_id();
		}
		void execute(void* buffer);
		void queueCommand(std::function<void()> command);
		inline void* allocate(size_t size, size_t alignment = 8) noexcept;
		template<typename PodType,
			typename = typename std::enable_if<std::is_trivially_destructible<PodType>::value>::type>
		inline PodType* allocatePod(
			size_t count = 1, size_t alignment = alignof(PodType)) noexcept;
	private:
		inline void* allocateCommand(size_t size) {
			assert(std::this_thread::get_id() == mThreadId);
			return mCurrentBuffer.allocate(size);
		}
		Driver& __restrict mDriver;
		CircularBuffer& __restrict mCurrentBuffer;
		Dispatcher mDispatcher;
		std::thread::id mThreadId{};
		bool mUsePerformanceCounter = false;
	};
	void* CommandStream::allocate(size_t size, size_t alignment) noexcept {
		assert(alignment && !(alignment & alignment - 1));
		const size_t s = CustomCommand::align(sizeof(NoopCommand) + size + alignment - 1);
		char* const p = (char*)allocateCommand(s);
		new(p) NoopCommand(p + s);
		void* data = (void*)((uintptr_t(p) + sizeof(NoopCommand) + alignment - 1) & ~(alignment - 1));
		assert(data >= p + sizeof(NoopCommand));
		return data;
	}
	template<typename PodType, typename>
	PodType* CommandStream::allocatePod(size_t count, size_t alignment) noexcept {
		return static_cast<PodType*>(allocate(count * sizeof(PodType), alignment));
	}
}





