#pragma once
#include "CircularBuffer.h"
#include <vector>
#include <condition_variable>
#include <stddef.h>
#include <stdint.h>
namespace MOON
{
	class CommandBufferQueue {
		struct Range {
			void* begin;
			void* end;
		};
		const size_t mRequiredSize;
		CircularBuffer mCircularBuffer;
		mutable std::mutex mLock;
		mutable std::condition_variable mCondition;
		mutable std::vector<Range> mCommandBuffersToExecute;
		size_t mFreeSpace = 0;
		size_t mHighWatermark = 0;
		uint32_t mExitRequested = 0;
		bool mPaused = false;
		static constexpr uint32_t EXIT_REQUESTED = 0x31415926;
	public:
		// requiredSize: guaranteed available space after flush()
		CommandBufferQueue(size_t requiredSize, size_t bufferSize, bool paused);
		~CommandBufferQueue();
		CircularBuffer& getCircularBuffer() noexcept { return mCircularBuffer; }
		CircularBuffer const& getCircularBuffer() const noexcept { return mCircularBuffer; }
		size_t getCapacity() const noexcept { return mRequiredSize; }
		size_t getHighWatermark() const noexcept { return mHighWatermark; }
		std::vector<Range> waitForCommands() const;
		void releaseBuffer(Range const& buffer);
		void flush() noexcept;
		void requestExit();
		bool isPaused() const noexcept;
		void setPaused(bool paused);
		bool isExitRequested() const;
	};
}





