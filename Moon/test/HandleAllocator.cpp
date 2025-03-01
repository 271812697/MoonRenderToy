#include "HandleAllocator.h"
#include "Handle.h"

#include <algorithm>
#include <exception>
#include <limits>
#include <mutex>

#include <stdlib.h>
#include <string.h>

namespace TEST {



	template <size_t P0, size_t P1, size_t P2>
	HandleAllocator<P0, P1, P2>::HandleAllocator(const char* name, size_t size,
		bool disableUseAfterFreeCheck) noexcept
		:
		mUseAfterFreeCheckDisabled(disableUseAfterFreeCheck) {
		// Reserve initial space for debug tags. This prevents excessive calls to malloc when the first
		// few tags are set.
		mDebugTags.reserve(512);
	}

	template <size_t P0, size_t P1, size_t P2>
	HandleAllocator<P0, P1, P2>::~HandleAllocator() {
		auto& overflowMap = mOverflowMap;
		if (!overflowMap.empty()) {

			for (auto& entry : overflowMap) {
				::free(entry.second);
			}
		}
	}

	template <size_t P0, size_t P1, size_t P2>

	void* HandleAllocator<P0, P1, P2>::handleToPointerSlow(HandleBase::HandleId id) const noexcept {
		auto& overflowMap = mOverflowMap;
		std::lock_guard lock(mLock);
		auto pos = overflowMap.find(id);
		if (pos != overflowMap.end()) {
			return pos->second;
		}
		return nullptr;
	}

	template <size_t P0, size_t P1, size_t P2>
	HandleBase::HandleId HandleAllocator<P0, P1, P2>::allocateHandleSlow(size_t size) {
		void* p = ::malloc(size);
		std::unique_lock lock(mLock);

		HandleBase::HandleId id = (++mId) | HANDLE_HEAP_FLAG;

		assert(mId < HANDLE_HEAP_FLAG);

		mOverflowMap.emplace(id, p);
		lock.unlock();

		if (id == (HANDLE_HEAP_FLAG | 1u)) { // meaning id was zero

		}
		return id;
	}

	template <size_t P0, size_t P1, size_t P2>
	void HandleAllocator<P0, P1, P2>::deallocateHandleSlow(HandleBase::HandleId id, size_t) noexcept {
		assert(id & HANDLE_HEAP_FLAG);
		void* p = nullptr;
		auto& overflowMap = mOverflowMap;

		std::unique_lock lock(mLock);
		auto pos = overflowMap.find(id);
		if (pos != overflowMap.end()) {
			p = pos->second;
			overflowMap.erase(pos);
		}
		lock.unlock();

		::free(p);
	}



	template class HandleAllocatorGL;
}
