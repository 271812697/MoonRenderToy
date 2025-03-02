#pragma once

#include "DriverEnums.h"

#include "glad/glad.h"

#include "utils/bitset.h"

#include <new>

#include <stddef.h>
#include <stdint.h>
#include <string.h>

namespace TEST {

	class BindingMap {
		struct CompressedBinding {
			// this is in fact a GLuint, but we only want 8-bits
			uint8_t binding : 7;
			uint8_t sampler : 1;
		};

		CompressedBinding(*mStorage)[MAX_DESCRIPTOR_COUNT];

		utils::bitset64 mActiveDescriptors[MAX_DESCRIPTOR_SET_COUNT];

	public:
		BindingMap() noexcept
			: mStorage(new (std::nothrow) CompressedBinding[MAX_DESCRIPTOR_SET_COUNT][MAX_DESCRIPTOR_COUNT]) {
			memset(mStorage, 0xFF, sizeof(CompressedBinding[MAX_DESCRIPTOR_SET_COUNT][MAX_DESCRIPTOR_COUNT]));
		}

		~BindingMap() noexcept {
			delete[] mStorage;
		}

		BindingMap(BindingMap const&) noexcept = delete;
		BindingMap(BindingMap&&) noexcept = delete;
		BindingMap& operator=(BindingMap const&) noexcept = delete;
		BindingMap& operator=(BindingMap&&) noexcept = delete;

		struct Binding {
			GLuint binding;
			DescriptorType type;
		};

		void insert(descriptor_set_t set, descriptor_binding_t binding, Binding entry) noexcept {
			assert(set < MAX_DESCRIPTOR_SET_COUNT);
			assert(binding < MAX_DESCRIPTOR_COUNT);
			assert(entry.binding < 128); // we reserve 1 bit for the type right now
			mStorage[set][binding] = { (uint8_t)entry.binding,
									   entry.type == DescriptorType::SAMPLER ||
									   entry.type == DescriptorType::SAMPLER_EXTERNAL };
			mActiveDescriptors[set].set(binding);
		}

		GLuint get(descriptor_set_t set, descriptor_binding_t binding) const noexcept {
			assert(set < MAX_DESCRIPTOR_SET_COUNT);
			assert(binding < MAX_DESCRIPTOR_COUNT);
			return mStorage[set][binding].binding;
		}

		utils::bitset64 getActiveDescriptors(descriptor_set_t set) const noexcept {
			return mActiveDescriptors[set];
		}
	};

}