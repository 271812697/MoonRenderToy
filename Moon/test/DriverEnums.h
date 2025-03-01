#pragma once


#include <array>        // FIXME: STL headers are not allowed in public headers
#include <type_traits>  // FIXME: STL headers are not allowed in public headers
#include <variant>      // FIXME: STL headers are not allowed in public headers

#include <stddef.h>
#include <stdint.h>

/**
 * Types and enums used by filament's driver.
 *
 * Effectively these types are public but should not be used directly. Instead use public classes
 * internal redeclaration of these types.
 * For e.g. Use Texture::Sampler instead of filament::SamplerType.
 */
namespace MOON {

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



} // namespace filament::backend
