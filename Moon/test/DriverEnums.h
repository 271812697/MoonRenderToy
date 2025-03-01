#pragma once
#include <array>        // FIXME: STL headers are not allowed in public headers
#include <type_traits>  // FIXME: STL headers are not allowed in public headers
#include <variant>      // FIXME: STL headers are not allowed in public headers

#include <stddef.h>
#include <stdint.h>


namespace TEST {


	static constexpr size_t MAX_VERTEX_ATTRIBUTE_COUNT = 16;   // This is guaranteed by OpenGL ES.
	static constexpr size_t MAX_SAMPLER_COUNT = 62;   // Maximum needed at feature level 3.
	enum class ElementType : uint8_t {
		BYTE,
		BYTE2,
		BYTE3,
		BYTE4,
		UBYTE,
		UBYTE2,
		UBYTE3,
		UBYTE4,
		SHORT,
		SHORT2,
		SHORT3,
		SHORT4,
		USHORT,
		USHORT2,
		USHORT3,
		USHORT4,
		INT,
		UINT,
		FLOAT,
		FLOAT2,
		FLOAT3,
		FLOAT4,
		HALF,
		HALF2,
		HALF3,
		HALF4,
	};
	enum class BufferUsage : uint8_t {
		STATIC,      //!< content modified once, used many times
		DYNAMIC,     //!< content modified frequently, used many times
	};

	//! Buffer object binding type
	enum class BufferObjectBinding : uint8_t {
		VERTEX,
		UNIFORM,
		SHADER_STORAGE
	};

	//! Vertex attribute descriptor
	struct Attribute {
		//! attribute is normalized (remapped between 0 and 1)
		static constexpr uint8_t FLAG_NORMALIZED = 0x1;
		//! attribute is an integer
		static constexpr uint8_t FLAG_INTEGER_TARGET = 0x2;
		static constexpr uint8_t BUFFER_UNUSED = 0xFF;
		uint32_t offset = 0;                    //!< attribute offset in bytes
		uint8_t stride = 0;                     //!< attribute stride in bytes
		uint8_t buffer = BUFFER_UNUSED;         //!< attribute buffer index
		ElementType type = ElementType::BYTE;   //!< attribute element type
		uint8_t flags = 0x0;                    //!< attribute flags
	};

	using AttributeArray = std::array<Attribute, MAX_VERTEX_ATTRIBUTE_COUNT>;

	//! Sampler Wrap mode
	enum class SamplerWrapMode : uint8_t {
		CLAMP_TO_EDGE,      //!< clamp-to-edge. The edge of the texture extends to infinity.
		REPEAT,             //!< repeat. The texture infinitely repeats in the wrap direction.
		MIRRORED_REPEAT,    //!< mirrored-repeat. The texture infinitely repeats and mirrors in the wrap direction.
	};

	//! Sampler minification filter
	enum class SamplerMinFilter : uint8_t {
		// don't change the enums values
		NEAREST = 0,                //!< No filtering. Nearest neighbor is used.
		LINEAR = 1,                 //!< Box filtering. Weighted average of 4 neighbors is used.
		NEAREST_MIPMAP_NEAREST = 2, //!< Mip-mapping is activated. But no filtering occurs.
		LINEAR_MIPMAP_NEAREST = 3,  //!< Box filtering within a mip-map level.
		NEAREST_MIPMAP_LINEAR = 4,  //!< Mip-map levels are interpolated, but no other filtering occurs.
		LINEAR_MIPMAP_LINEAR = 5    //!< Both interpolated Mip-mapping and linear filtering are used.
	};

	//! Sampler magnification filter
	enum class SamplerMagFilter : uint8_t {
		// don't change the enums values
		NEAREST = 0,                //!< No filtering. Nearest neighbor is used.
		LINEAR = 1,                 //!< Box filtering. Weighted average of 4 neighbors is used.
	};

	//! Sampler compare mode
	enum class SamplerCompareMode : uint8_t {
		// don't change the enums values
		NONE = 0,
		COMPARE_TO_TEXTURE = 1
	};

	//! comparison function for the depth / stencil sampler
	enum class SamplerCompareFunc : uint8_t {
		// don't change the enums values
		LE = 0,     //!< Less or equal
		GE,         //!< Greater or equal
		L,          //!< Strictly less than
		G,          //!< Strictly greater than
		E,          //!< Equal
		NE,         //!< Not equal
		A,          //!< Always. Depth / stencil testing is deactivated.
		N           //!< Never. The depth / stencil test always fails.
	};
	//! Sampler parameters
	struct SamplerParams { // NOLINT
		SamplerMagFilter filterMag : 1;    //!< magnification filter (NEAREST)
		SamplerMinFilter filterMin : 3;    //!< minification filter  (NEAREST)
		SamplerWrapMode wrapS : 2;    //!< s-coordinate wrap mode (CLAMP_TO_EDGE)
		SamplerWrapMode wrapT : 2;    //!< t-coordinate wrap mode (CLAMP_TO_EDGE)

		SamplerWrapMode wrapR : 2;    //!< r-coordinate wrap mode (CLAMP_TO_EDGE)
		uint8_t anisotropyLog2 : 3;    //!< anisotropy level (0)
		SamplerCompareMode compareMode : 1;    //!< sampler compare mode (NONE)
		uint8_t padding0 : 2;    //!< reserved. must be 0.

		SamplerCompareFunc compareFunc : 3;    //!< sampler comparison function (LE)
		uint8_t padding1 : 5;    //!< reserved. must be 0.
		uint8_t padding2 : 8;    //!< reserved. must be 0.

		struct Hasher {
			size_t operator()(SamplerParams p) const noexcept {
				// we don't use std::hash<> here, so we don't have to include <functional>
				return *reinterpret_cast<uint32_t const*>(reinterpret_cast<char const*>(&p));
			}
		};

		struct EqualTo {
			bool operator()(SamplerParams lhs, SamplerParams rhs) const noexcept {
				assert_invariant(lhs.padding0 == 0);
				assert_invariant(lhs.padding1 == 0);
				assert_invariant(lhs.padding2 == 0);
				auto* pLhs = reinterpret_cast<uint32_t const*>(reinterpret_cast<char const*>(&lhs));
				auto* pRhs = reinterpret_cast<uint32_t const*>(reinterpret_cast<char const*>(&rhs));
				return *pLhs == *pRhs;
			}
		};

		struct LessThan {
			bool operator()(SamplerParams lhs, SamplerParams rhs) const noexcept {
				assert_invariant(lhs.padding0 == 0);
				assert_invariant(lhs.padding1 == 0);
				assert_invariant(lhs.padding2 == 0);
				auto* pLhs = reinterpret_cast<uint32_t const*>(reinterpret_cast<char const*>(&lhs));
				auto* pRhs = reinterpret_cast<uint32_t const*>(reinterpret_cast<char const*>(&rhs));
				return *pLhs == *pRhs;
			}
		};

	private:
		friend inline bool operator == (SamplerParams lhs, SamplerParams rhs) noexcept {
			return SamplerParams::EqualTo{}(lhs, rhs);
		}
		friend inline bool operator != (SamplerParams lhs, SamplerParams rhs) noexcept {
			return  !SamplerParams::EqualTo{}(lhs, rhs);
		}
		friend inline bool operator < (SamplerParams lhs, SamplerParams rhs) noexcept {
			return SamplerParams::LessThan{}(lhs, rhs);
		}
	};
	static_assert(sizeof(SamplerParams) == 4);

	// The limitation to 64-bits max comes from how we store a SamplerParams in our JNI code
	// see android/.../TextureSampler.cpp
	static_assert(sizeof(SamplerParams) <= sizeof(uint64_t),
		"SamplerParams must be no more than 64 bits");


} // namespace filament::backend
