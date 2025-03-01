#pragma once 


#include <stddef.h>
#include <stdint.h>

namespace MOON {

	class CircularBuffer {
	public:
		// bufferSize: total buffer size.
		//      This must be at least 2*requiredSize to avoid blocking on flush, however
		//      because sometimes the display can get ahead of the render() thread, it's good
		//      to set it to 3*requiredSize to avoid blocking the render thread (usually the UI thread).
		explicit CircularBuffer(size_t bufferSize);

		// can't be moved or copy-constructed
		CircularBuffer(CircularBuffer const& rhs) = delete;
		CircularBuffer(CircularBuffer&& rhs) noexcept = delete;
		CircularBuffer& operator=(CircularBuffer const& rhs) = delete;
		CircularBuffer& operator=(CircularBuffer&& rhs) noexcept = delete;

		~CircularBuffer() noexcept;

		static size_t getBlockSize() noexcept { return sPageSize; }

		// Total size of circular buffer. This is a constant.
		size_t size() const noexcept { return mSize; }

		// Allocates `s` bytes in the circular buffer and returns a pointer to the memory. All
		// allocations must not exceed size() bytes.
		inline void* allocate(size_t s) noexcept {
			// We can never allocate more that size().
			//assert(getUsed() + s <= size());
			char* const cur = static_cast<char*>(mHead);
			mHead = cur + s;
			return cur;
		}

		// Returns true if the buffer is empty, i.e.: no allocations were made since
		// calling getBuffer();
		bool empty() const noexcept { return mTail == mHead; }

		// Returns the size used since the last call to getBuffer()
		size_t getUsed() const noexcept { return intptr_t(mHead) - intptr_t(mTail); }

		// Retrieves the current allocated range and frees it. It is the responsibility of the caller
		// to make sure the returned range is no longer in use by the time allocate() allocates
		// (size() - getUsed()) bytes.
		struct Range {
			void* tail;
			void* head;
		};
		Range getBuffer() noexcept;

	private:
		void* alloc(size_t size) noexcept;
		void dealloc() noexcept;

		// pointer to the beginning of the circular buffer (constant)
		void* mData = nullptr;
		int mAshmemFd = -1;

		// size of the circular buffer (constant)
		size_t const mSize;

		// pointer to the beginning of recorded data
		void* mTail = nullptr;

		// pointer to the next available command
		void* mHead = nullptr;

		// system page size
		static size_t sPageSize;
	};

} // namespace filament::backend

