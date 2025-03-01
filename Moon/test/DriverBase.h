#pragma once




//#include "private/backend/Dispatcher.h"
#include "Driver.h"

#include <condition_variable>
#include <memory>
#include <mutex>
#include <thread>
#include <utility>
#include <vector>

#include <stdint.h>
#include <assert.h>

namespace MOON {

	struct AcquiredImage;

	/*
	 * Hardware handles
	 */

	struct HwBase {
	};


	struct HwVertexBufferInfo : public HwBase {
		uint8_t bufferCount{};                //   1
		uint8_t attributeCount{};             //   1
		bool padding[2]{};                    //   2
		HwVertexBufferInfo() noexcept = default;
		HwVertexBufferInfo(uint8_t bufferCount, uint8_t attributeCount) noexcept
			: bufferCount(bufferCount),
			attributeCount(attributeCount) {
		}
	};

	struct HwVertexBuffer : public HwBase {
		uint32_t vertexCount{};               //   4
		uint8_t bufferObjectsVersion{ 0xff };   //   1
		bool padding[3]{};                    //   2
		HwVertexBuffer() noexcept = default;
		explicit HwVertexBuffer(uint32_t vertextCount) noexcept
			: vertexCount(vertextCount) {
		}
	};

	struct HwBufferObject : public HwBase {
		uint32_t byteCount{};

		HwBufferObject() noexcept = default;
		explicit HwBufferObject(uint32_t byteCount) noexcept : byteCount(byteCount) {}
	};

	struct HwIndexBuffer : public HwBase {
		uint32_t count : 27;
		uint32_t elementSize : 5;

		HwIndexBuffer() noexcept : count{}, elementSize{} { }
		HwIndexBuffer(uint8_t elementSize, uint32_t indexCount) noexcept :
			count(indexCount), elementSize(elementSize) {
			// we could almost store elementSize on 4 bits because it's never > 16 and never 0
			assert(elementSize > 0 && elementSize <= 16);
			assert(indexCount < (1u << 27));
		}
	};

	struct HwRenderPrimitive : public HwBase {
		PrimitiveType type = PrimitiveType::TRIANGLES;
	};

	struct HwProgram : public HwBase {
		std::string name;
		explicit HwProgram(std::string name) noexcept : name(std::move(name)) { }
		HwProgram() noexcept = default;
	};

	struct HwDescriptorSetLayout : public HwBase {
		HwDescriptorSetLayout() noexcept = default;
	};

	struct HwDescriptorSet : public HwBase {
		HwDescriptorSet() noexcept = default;
	};

	struct HwSamplerGroup : public HwBase {
		HwSamplerGroup() noexcept = default;
	};



	struct HwRenderTarget : public HwBase {
		uint32_t width{};
		uint32_t height{};
		HwRenderTarget() noexcept = default;
		HwRenderTarget(uint32_t w, uint32_t h) : width(w), height(h) { }
	};



	struct HwTimerQuery : public HwBase {
	};

	/*
	 * Base class of all Driver implementations
	 */

	class DriverBase : public Driver {
	public:
		DriverBase() noexcept;
		~DriverBase() noexcept override;

	protected:
		class CallbackDataDetails;

	private:
		std::mutex mPurgeLock;
		std::thread mServiceThread;
		std::mutex mServiceThreadLock;
		std::condition_variable mServiceThreadCondition;

		bool mExitRequested = false;
	};

}