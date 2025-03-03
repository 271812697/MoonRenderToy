#pragma once
#include "Handle.h"
#include <stddef.h>
#include <stdint.h>

namespace TEST {

	//! \privatesection
	struct HwTexture;
	struct TargetBufferInfo {
		// note: the parameters of this constructor are not in the order of this structure's fields
		TargetBufferInfo(Handle<HwTexture> handle, uint8_t level, uint16_t layer) noexcept
			: handle(handle), level(level), layer(layer) {
		}

		TargetBufferInfo(Handle<HwTexture> handle, uint8_t level) noexcept
			: handle(handle), level(level) {
		}

		TargetBufferInfo(Handle<HwTexture> handle) noexcept // NOLINT(*-explicit-constructor)
			: handle(handle) {
		}

		TargetBufferInfo() noexcept = default;

		// texture to be used as render target
		Handle<HwTexture> handle;

		// level to be used
		uint8_t level = 0;

		// - For cubemap textures, this indicates the face of the cubemap. See TextureCubemapFace for
		//   the face->layer mapping)
		// - For 2d array, cubemap array, and 3d textures, this indicates an index of a single layer of
		//   them.
		// - For multiview textures (i.e., layerCount for the RenderTarget is greater than 1), this
		//   indicates a starting layer index of the current 2d array texture for multiview.
		uint16_t layer = 0;
	};

	class MRT {
	public:
		static constexpr uint8_t MIN_SUPPORTED_RENDER_TARGET_COUNT = 4u;

		// When updating this, make sure to also take care of RenderTarget.java
		static constexpr uint8_t MAX_SUPPORTED_RENDER_TARGET_COUNT = 8u;

	private:
		TargetBufferInfo mInfos[MAX_SUPPORTED_RENDER_TARGET_COUNT];

	public:
		TargetBufferInfo const& operator[](size_t i) const noexcept {
			return mInfos[i];
		}

		TargetBufferInfo& operator[](size_t i) noexcept {
			return mInfos[i];
		}

		MRT() noexcept = default;

		MRT(TargetBufferInfo const& color) noexcept // NOLINT(hicpp-explicit-conversions, *-explicit-constructor)
			: mInfos{ color } {
		}

		MRT(TargetBufferInfo const& color0, TargetBufferInfo const& color1) noexcept
			: mInfos{ color0, color1 } {
		}

		MRT(TargetBufferInfo const& color0, TargetBufferInfo const& color1,
			TargetBufferInfo const& color2) noexcept
			: mInfos{ color0, color1, color2 } {
		}

		MRT(TargetBufferInfo const& color0, TargetBufferInfo const& color1,
			TargetBufferInfo const& color2, TargetBufferInfo const& color3) noexcept
			: mInfos{ color0, color1, color2, color3 } {
		}

		// this is here for backward compatibility
		MRT(Handle<HwTexture> handle, uint8_t level, uint16_t layer) noexcept
			: mInfos{ { handle, level, layer } } {
		}
	};

}