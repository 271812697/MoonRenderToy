#pragma once

#include <memory>
namespace TEST {
	class CommandStream;
	struct TestInstanceData;
	class TestInstance {
	private:
		TestInstance(const TestInstance&) = delete;
		TestInstance& operator=(const TestInstance&) = delete;
		TestInstance();

		std::shared_ptr<TestInstanceData> mData;
	public:
		static TestInstance& Instance();
		CommandStream* getCommandStream();
		void flush();
		void execute();
	};
} // namespace filament::backend


