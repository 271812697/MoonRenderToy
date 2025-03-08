#pragma once
#include "DriverEnums.h"
#include <condition_variable>
#include <memory>
#include <mutex>
#include <thread>
#include <utility>
#include <vector>
#include "utils/CString.h"

#include <stdint.h>
#include <assert.h>

namespace TEST {



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


	struct HwProgram : public HwBase {
		utils::CString name;
		explicit HwProgram(utils::CString name) noexcept : name(std::move(name)) { }
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
	struct HwStream : public HwBase {

		StreamType streamType = StreamType::ACQUIRED;
		uint32_t width{};
		uint32_t height{};


		explicit HwStream() noexcept
			: streamType(StreamType::NATIVE) {
		}
	};
	struct HwTexture : public HwBase {
		uint32_t width{};
		uint32_t height{};
		uint32_t depth{};
		SamplerType target{};
		uint8_t levels : 4;  // This allows up to 15 levels (max texture size of 32768 x 32768)
		uint8_t samples : 4; // Sample count per pixel (should always be a power of 2)
		TextureFormat format{};
		uint8_t reserved0 = 0;
		TextureUsage usage{};
		uint16_t reserved1 = 0;
		HwStream* hwStream = nullptr;

		HwTexture() noexcept : levels{}, samples{} {}
		HwTexture(SamplerType target, uint8_t levels, uint8_t samples,
			uint32_t width, uint32_t height, uint32_t depth, TextureFormat fmt, TextureUsage usage) noexcept
			: width(width), height(height), depth(depth),
			target(target), levels(levels), samples(samples), format(fmt), usage(usage) { }
	};

	struct HwTimerQuery : public HwBase {
	};

}