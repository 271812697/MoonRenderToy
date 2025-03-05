#include "TestInstance.h"
#include "Driver.h"
#include "CircularBuffer.h"
#include "CommandBufferQueue.h"
#include "CommandStream.h"
#include "MaterialCompiler.h"
namespace TEST {

	static constexpr const size_t MiB = 1024u * 1024u;
	struct TestInstanceData {
		TestInstanceData() {}
		std::shared_ptr<Driver>driver;
		std::shared_ptr<CommandBufferQueue>buffer;
		std::shared_ptr<CommandStream>api;

	};
	TestInstance::TestInstance()
	{
		mData = std::make_shared<TestInstanceData>();

		mData->driver = std::make_shared<Driver>();
		mData->buffer = std::shared_ptr<CommandBufferQueue>(new CommandBufferQueue(9 * MiB, 27 * MiB, false));
		mData->api = std::make_shared<CommandStream>(*mData->driver.get(), mData->buffer->getCircularBuffer());

		MaterialCompiler::compile("normalColor.mat");
	}

	void TestInstance::flush()
	{
		mData->buffer->flush();
	}

	void TestInstance::execute()
	{
		auto buffers = mData->buffer->waitForCommands();

		for (auto& item : buffers) {
			if (item.begin) {
				mData->api->execute(item.begin);
				mData->buffer->releaseBuffer(item);
			}
		}
	}

	TestInstance& TestInstance::Instance()
	{
		static TestInstance instance;
		return instance;
	}

	CommandStream* TestInstance::getCommandStream()
	{
		return mData->api.get();
	}

} // namespace filament::backend
