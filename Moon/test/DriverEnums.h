#pragma once
#include <array>        // FIXME: STL headers are not allowed in public headers
#include <type_traits>  // FIXME: STL headers are not allowed in public headers
#include <variant>      // FIXME: STL headers are not allowed in public headers

#include <stddef.h>
#include <stdint.h>


namespace TEST {
	static constexpr size_t MAX_VERTEX_ATTRIBUTE_COUNT = 16;   // This is guaranteed by OpenGL ES.
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

} // namespace filament::backend
