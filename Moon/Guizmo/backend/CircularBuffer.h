#pragma once 
#include <stddef.h>
#include <stdint.h>
namespace MOON
{
	class CircularBuffer {
	public:
		explicit CircularBuffer(size_t bufferSize);
		CircularBuffer(CircularBuffer const& rhs) = delete;
		CircularBuffer(CircularBuffer&& rhs) noexcept = delete;
		CircularBuffer& operator=(CircularBuffer const& rhs) = delete;
		CircularBuffer& operator=(CircularBuffer&& rhs) noexcept = delete;
		~CircularBuffer() noexcept;
		static size_t getBlockSize() noexcept { return sPageSize; }
		size_t size() const noexcept { return mSize; }
		inline void* allocate(size_t s) noexcept {
			//assert(getUsed() + s <= size());
			char* const cur = static_cast<char*>(mHead);
			mHead = cur + s;
			return cur;
		}
		bool empty() const noexcept { return mTail == mHead; }
		size_t getUsed() const noexcept { return intptr_t(mHead) - intptr_t(mTail); }
		struct Range {
			void* tail;
			void* head;
		};
		Range getBuffer() noexcept;
	private:
		void* alloc(size_t size) noexcept;
		void dealloc() noexcept;
		void* mData = nullptr;
		int mAshmemFd = -1;
		size_t const mSize;
		void* mTail = nullptr;
		void* mHead = nullptr;
		static size_t sPageSize;
	};
}

