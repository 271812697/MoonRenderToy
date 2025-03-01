#include "CircularBuffer.h"
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include<assert.h>


namespace TEST {

	size_t CircularBuffer::sPageSize = 4096;

	CircularBuffer::CircularBuffer(size_t size)
		: mSize(size) {
		mData = alloc(size);
		mTail = mData;
		mHead = mData;
	}

	CircularBuffer::~CircularBuffer() noexcept {
		dealloc();
	}

	void* CircularBuffer::alloc(size_t size) noexcept {
		return ::malloc(2 * size);
	}


	void CircularBuffer::dealloc() noexcept {
		::free(mData);
		mData = nullptr;
	}


	CircularBuffer::Range CircularBuffer::getBuffer() noexcept {
		Range const range{ mTail,  mHead };

		char* const pData = static_cast<char*>(mData);
		char const* const pEnd = pData + mSize;
		char const* const pHead = static_cast<char const*>(mHead);
		if (pHead >= pEnd) {
			size_t const overflow = pHead - pEnd;
			if (mAshmemFd > 0) {
				assert(overflow <= mSize);
				mHead = static_cast<void*>(pData + overflow);
				// Data         Tail  End   Head              [virtual]
				//  v             v    v     v
				//  +-------------:----+-----:--------------+
				//  |             :    |     :              |
				//  +-----:------------+--------------------+
				//       Head          |<------ copy ------>| [physical]
			}
			else {
				// Data         Tail  End   Head
				//  v             v    v     v
				//  +-------------:----+-----:--------------+
				//  |             :    |     :              |
				//  +-----|------------+-----|--------------+
				//        |<---------------->|
				//           sliding window
				mHead = mData;
			}
		}
		mTail = mHead;

		return range;
	}

} // namespace filament::backend
