#pragma once
#include "Handle.h"
#include <cstddef>
#include <exception>
#include <type_traits>
#include <unordered_map>
#include <utility>
#include<string>
#include <mutex>
#include <stddef.h>
#include <stdint.h>

#define HandleAllocatorGL  HandleAllocator<32,  96, 136>    // ~4520 / pool / MiB

namespace TEST {

	/*
	 * A utility class to efficiently allocate and manage Handle<>
	 */
	template<size_t P0, size_t P1, size_t P2>
	class HandleAllocator {


	public:
		HandleAllocator(const char* name, size_t size, bool disableUseAfterFreeCheck) noexcept;
		HandleAllocator(HandleAllocator const& rhs) = delete;
		HandleAllocator& operator=(HandleAllocator const& rhs) = delete;
		~HandleAllocator();

		template<typename D, typename ... ARGS>
		Handle<D> allocateAndConstruct(ARGS&& ... args) {
			Handle<D> h{ allocateHandle<D>() };
			D* addr = handle_cast<D*>(h);
			new(addr) D(std::forward<ARGS>(args)...);
			return h;
		}

		template<typename D>
		Handle<D> allocate() noexcept {
			Handle<D> h{ allocateHandle<D>() };
			return h;
		}


		template<typename D, typename B, typename ... ARGS>
		typename std::enable_if_t<std::is_base_of_v<B, D>, D>*
			destroyAndConstruct(Handle<B> const& handle, ARGS&& ... args) {
			assert(handle);
			D* addr = handle_cast<D*>(const_cast<Handle<B>&>(handle));
			assert(addr);
			addr->~D();
			new(addr) D(std::forward<ARGS>(args)...);
			return addr;
		}


		template<typename D, typename B, typename ... ARGS>
		typename std::enable_if_t<std::is_base_of_v<B, D>, D>*
			construct(Handle<B> const& handle, ARGS&& ... args) noexcept {
			assert(handle);
			D* addr = handle_cast<D*>(const_cast<Handle<B>&>(handle));
			assert(addr);
			new(addr) D(std::forward<ARGS>(args)...);
			return addr;
		}

		template <typename B, typename D, typename = typename std::enable_if_t<std::is_base_of_v<B, D>, D>>
		void deallocate(Handle<B>& handle, D const* p) noexcept {
			// allow to destroy the nullptr, similarly to operator delete
			if (p) {
				p->~D();
				deallocateHandle<D>(handle.getId());
			}
		}

		template<typename D>
		void deallocate(Handle<D>& handle) noexcept {
			D const* d = handle_cast<const D*>(handle);
			deallocate(handle, d);
		}


		template<typename Dp, typename B>
		inline typename std::enable_if_t<
			std::is_pointer_v<Dp>&&
			std::is_base_of_v<B, typename std::remove_pointer_t<Dp>>, Dp>
			handle_cast(Handle<B>& handle) {
			assert(handle);
			auto [p, tag] = handleToPointer(handle.getId());
			// check for heap handle use-after-free
			if ((!mUseAfterFreeCheckDisabled)) {
				uint8_t const index = (handle.getId() & HANDLE_INDEX_MASK);
				// if we've already handed out this handle index before, it's definitely a
				// use-after-free, otherwise it's probably just a corrupted handle
				if (index < mId) {
					assert(p != nullptr);
				}
				else {
					assert(p != nullptr);
				}
			}
			return static_cast<Dp>(p);
		}

		template<typename B>
		bool is_valid(Handle<B>& handle) {
			if (!handle) {
				// null handles are invalid
				return false;
			}
			auto [p, tag] = handleToPointer(handle.getId());

			return p != nullptr;
		}

		template<typename Dp, typename B>
		inline typename std::enable_if_t<
			std::is_pointer_v<Dp>&&
			std::is_base_of_v<B, typename std::remove_pointer_t<Dp>>, Dp>
			handle_cast(Handle<B> const& handle) {
			return handle_cast<Dp>(const_cast<Handle<B>&>(handle));
		}
	private:

		template<typename D>
		static constexpr size_t getBucketSize() noexcept {
			if constexpr (sizeof(D) <= P0) { return P0; }
			if constexpr (sizeof(D) <= P1) { return P1; }
			static_assert(sizeof(D) <= P2);
			return P2;
		}


		// allocateHandle()/deallocateHandle() selects the pool to use at compile-time based on the
		// allocation size this is always inlined, because all these do is to call
		// allocateHandleInPool()/deallocateHandleFromPool() with the right pool size.
		template<typename D>
		HandleBase::HandleId allocateHandle() noexcept {
			constexpr size_t BUCKET_SIZE = getBucketSize<D>();
			return allocateHandleInPool<BUCKET_SIZE>();
		}

		template<typename D>
		void deallocateHandle(HandleBase::HandleId id) noexcept {
			constexpr size_t BUCKET_SIZE = getBucketSize<D>();
			deallocateHandleFromPool<BUCKET_SIZE>(id);
		}

		// allocateHandleInPool()/deallocateHandleFromPool() is NOT inlined, which will cause three
		// versions to be generated, one for each pool. Because the arena is synchronized,
		// the code generated is not trivial (even if it's not insane either).
		template<size_t SIZE>
		HandleBase::HandleId allocateHandleInPool() noexcept {
			return allocateHandleSlow(SIZE);
		}

		template<size_t SIZE>
		void deallocateHandleFromPool(HandleBase::HandleId id) noexcept {
			deallocateHandleSlow(id, SIZE);
		}

		// number if bits allotted to the handle's age (currently 4 max)
		static constexpr uint32_t HANDLE_AGE_BIT_COUNT = 4;
		// number if bits allotted to the handle's debug tag (HANDLE_AGE_BIT_COUNT max)
		static constexpr uint32_t HANDLE_DEBUG_TAG_BIT_COUNT = 2;
		// bit shift for both the age and debug tag
		static constexpr uint32_t HANDLE_AGE_SHIFT = 27;
		// mask for the heap (vs pool) flag
		static constexpr uint32_t HANDLE_HEAP_FLAG = 0x80000000u;
		// mask for the age
		static constexpr uint32_t HANDLE_AGE_MASK =
			((1 << HANDLE_AGE_BIT_COUNT) - 1) << HANDLE_AGE_SHIFT;
		// mask for the debug tag
		static constexpr uint32_t HANDLE_DEBUG_TAG_MASK =
			((1 << HANDLE_DEBUG_TAG_BIT_COUNT) - 1) << HANDLE_AGE_SHIFT;
		// mask for the index
		static constexpr uint32_t HANDLE_INDEX_MASK = 0x07FFFFFFu;

		static_assert(HANDLE_DEBUG_TAG_BIT_COUNT <= HANDLE_AGE_BIT_COUNT);



		HandleBase::HandleId allocateHandleSlow(size_t size);
		void deallocateHandleSlow(HandleBase::HandleId id, size_t size) noexcept;


		inline std::pair<void*, uint32_t> handleToPointer(HandleBase::HandleId id) const noexcept {
			// note: the null handle will end-up returning nullptr b/c it'll be handled as
			return { handleToPointerSlow(id), 0 };
		}

		void* handleToPointerSlow(HandleBase::HandleId id) const noexcept;



		//HandleArena mHandleArena;

		// Below is only used when running out of space in the HandleArena
		mutable std::mutex mLock;
		std::unordered_map<HandleBase::HandleId, void*> mOverflowMap;
		std::unordered_map<HandleBase::HandleId, std::string> mDebugTags;
		HandleBase::HandleId mId = 0;
		bool mUseAfterFreeCheckDisabled = false;


	};

}