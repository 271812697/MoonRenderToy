#include "CommandBufferQueue.h"
#include "CircularBuffer.h"
#include "CommandStream.h"
#include <algorithm>
#include <mutex>
#include <iterator>
#include <utility>
#include <vector>
#include <assert.h>

#include <stddef.h>
#include <stdint.h>

#define UTILS_HAS_THREADING 0
namespace MOON {

	CommandBufferQueue::CommandBufferQueue(size_t requiredSize, size_t bufferSize, bool paused)
		: mRequiredSize((requiredSize + (CircularBuffer::getBlockSize() - 1u)) & ~(CircularBuffer::getBlockSize() - 1u)),
		mCircularBuffer(bufferSize),
		mFreeSpace(mCircularBuffer.size()),
		mPaused(paused) {
		assert(mCircularBuffer.size() > requiredSize);
	}

	CommandBufferQueue::~CommandBufferQueue() {
		assert(mCommandBuffersToExecute.empty());
	}

	void CommandBufferQueue::requestExit() {
		std::lock_guard<std::mutex> const lock(mLock);
		mExitRequested = EXIT_REQUESTED;
		mCondition.notify_one();
	}

	bool CommandBufferQueue::isPaused() const noexcept {
		std::lock_guard<std::mutex> const lock(mLock);
		return mPaused;
	}

	void CommandBufferQueue::setPaused(bool paused) {
		std::lock_guard<std::mutex> const lock(mLock);
		if (paused) {
			mPaused = true;
		}
		else {
			mPaused = false;
			mCondition.notify_one();
		}
	}

	bool CommandBufferQueue::isExitRequested() const {
		std::lock_guard<std::mutex> const lock(mLock);
		return (bool)mExitRequested;
	}


	void CommandBufferQueue::flush() noexcept {

		CircularBuffer& circularBuffer = mCircularBuffer;
		if (circularBuffer.empty()) {
			return;
		}

		// add the terminating command
		// always guaranteed to have enough space for the NoopCommand
		new(circularBuffer.allocate(sizeof(NoopCommand))) NoopCommand(nullptr);

		const size_t requiredSize = mRequiredSize;

		// get the current buffer
		auto const [begin, end] = circularBuffer.getBuffer();

		assert(circularBuffer.empty());

		// size of the current buffer
		size_t const used = std::distance(
			static_cast<char const*>(begin), static_cast<char const*>(end));


		std::unique_lock<std::mutex> lock(mLock);


		mFreeSpace -= used;
		mCommandBuffersToExecute.push_back({ begin, end });
		mCondition.notify_one();

		// wait until there is enough space in the buffer
		if ((mFreeSpace < requiredSize)) {


			mCondition.wait(lock, [this, requiredSize]() -> bool {
				// TODO: on macOS, we need to call pumpEvents from time to time
				return mFreeSpace >= requiredSize;
				});
		}
	}

	std::vector<CommandBufferQueue::Range> CommandBufferQueue::waitForCommands() const {
		if (!UTILS_HAS_THREADING) {
			return std::move(mCommandBuffersToExecute);
		}
		std::unique_lock<std::mutex> lock(mLock);
		while ((mCommandBuffersToExecute.empty() || mPaused) && !mExitRequested) {
			mCondition.wait(lock);
		}
		return std::move(mCommandBuffersToExecute);
	}

	void CommandBufferQueue::releaseBuffer(CommandBufferQueue::Range const& buffer) {
		size_t const used = std::distance(
			static_cast<char const*>(buffer.begin), static_cast<char const*>(buffer.end));
		std::lock_guard<std::mutex> const lock(mLock);
		mFreeSpace += used;
		mCondition.notify_one();
	}

} // namespace filament::backend
