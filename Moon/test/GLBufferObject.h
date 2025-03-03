#pragma once

#include "DriverBase.h"
#include "DriverEnums.h"
#include <stdint.h>

namespace TEST {

	struct GLBufferObject : public HwBufferObject {
		using HwBufferObject::HwBufferObject;
		GLBufferObject(uint32_t size,
			BufferObjectBinding bindingType, BufferUsage usage) noexcept
			: HwBufferObject(size), usage(usage), bindingType(bindingType) {
		}

		struct {
			unsigned int id;
			union {
				unsigned int binding;
				void* buffer;
			};
		} gl;
		BufferUsage usage;
		BufferObjectBinding bindingType;
		uint16_t age = 0;
	};

} // namespace filament::backend

