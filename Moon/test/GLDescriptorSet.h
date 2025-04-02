
#ifndef TNT_FILAMENT_BACKEND_OPENGL_GLDESCRIPTORSET_H
#define TNT_FILAMENT_BACKEND_OPENGL_GLDESCRIPTORSET_H

#include "DriverBase.h"

#include <glad/glad.h>

#include "HandleAllocator.h"

#include "DriverEnums.h"
#include "Handle.h"

#include <test/utils/bitset.h>
#include <test/utils/FixedCapacityVector.h>

#include <test/math/half.h>

#include <array>
#include <variant>

#include <stddef.h>
#include <stdint.h>

namespace TEST {

	struct GLBufferObject;
	struct GLTexture;
	struct GLTextureRef;
	struct GLDescriptorSetLayout;
	class OpenGLProgram;
	class OpenGLContext;
	class OpenGLDriver;

	struct GLDescriptorSet : public HwDescriptorSet {

		using HwDescriptorSet::HwDescriptorSet;

		GLDescriptorSet(OpenGLContext& gl, DescriptorSetLayoutHandle dslh,
			GLDescriptorSetLayout const* layout) noexcept;

		// update a buffer descriptor in the set
		void update(OpenGLContext& gl,
			descriptor_binding_t binding, GLBufferObject* bo, size_t offset, size_t size) noexcept;

		// update a sampler descriptor in the set
		void update(OpenGLContext& gl,
			descriptor_binding_t binding, GLTexture* t, SamplerParams params) noexcept;

		// conceptually bind the set to the command buffer
		void bind(
			OpenGLContext& gl,
			HandleAllocatorGL& handleAllocator,
			OpenGLProgram const& p,
			descriptor_set_t set, uint32_t const* offsets, bool offsetsOnly) const noexcept;

		uint32_t getDynamicBufferCount() const noexcept {
			return dynamicBufferCount;
		}

		void validate(HandleAllocatorGL& allocator, DescriptorSetLayoutHandle pipelineLayout) const;

	private:
		// a Buffer Descriptor such as SSBO or UBO with static offset
		struct Buffer {
			// Workaround: we cannot define the following as Buffer() = default because one of our
			// clients has their compiler set up where such declaration (possibly coupled with explicit)
			// will be considered a deleted constructor.
			Buffer() {}

			explicit Buffer(GLenum target) noexcept : target(target) {}
			GLenum target;                          // 4
			GLuint id = 0;                          // 4
			uint32_t offset = 0;                    // 4
			uint32_t size = 0;                      // 4
		};

		// a Buffer Descriptor such as SSBO or UBO with dynamic offset
		struct DynamicBuffer {
			DynamicBuffer() = default;
			explicit DynamicBuffer(GLenum target) noexcept : target(target) { }
			GLenum target;                          // 4
			GLuint id = 0;                          // 4
			uint32_t offset = 0;                    // 4
			uint32_t size = 0;                      // 4
		};

		// a UBO descriptor for ES2
		struct BufferGLES2 {
			BufferGLES2() = default;
			explicit BufferGLES2(bool dynamicOffset) noexcept : dynamicOffset(dynamicOffset) { }
			GLBufferObject const* bo = nullptr;     // 8
			uint32_t offset = 0;                    // 4
			bool dynamicOffset = false;             // 4
		};

		// A sampler descriptor
		struct Sampler {
			GLenum target = 0;                      // 4
			GLuint id = 0;                          // 4
			GLuint sampler = 0;                     // 4
			Handle<GLTextureRef> ref;               // 4
			int8_t baseLevel = 0x7f;                // 1
			int8_t maxLevel = -1;                   // 1
			std::array<TextureSwizzle, 4> swizzle{  // 4
					TextureSwizzle::CHANNEL_0,
					TextureSwizzle::CHANNEL_1,
					TextureSwizzle::CHANNEL_2,
					TextureSwizzle::CHANNEL_3
			};
		};

		struct SamplerWithAnisotropyWorkaround {
			GLenum target = 0;                      // 4
			GLuint id = 0;                          // 4
			GLuint sampler = 0;                     // 4
			Handle<GLTextureRef> ref;               // 4
			math::half anisotropy = 1.0f;           // 2
			int8_t baseLevel = 0x7f;                // 1
			int8_t maxLevel = -1;                   // 1
			std::array<TextureSwizzle, 4> swizzle{  // 4
					TextureSwizzle::CHANNEL_0,
					TextureSwizzle::CHANNEL_1,
					TextureSwizzle::CHANNEL_2,
					TextureSwizzle::CHANNEL_3
			};
		};

		// A sampler descriptor for ES2
		struct SamplerGLES2 {
			GLenum target = 0;                      // 4
			GLuint id = 0;                          // 4
			SamplerParams params{};                 // 4
			float anisotropy = 1.0f;                // 4
		};
		struct Descriptor {
			std::variant<
				Buffer,
				DynamicBuffer,
				BufferGLES2,
				Sampler,
				SamplerWithAnisotropyWorkaround,
				SamplerGLES2> desc;
		};
		static_assert(sizeof(Descriptor) <= 32);

		template<typename T>
		static void updateTextureView(OpenGLContext& gl,
			HandleAllocatorGL& handleAllocator, GLuint unit, T const& desc) noexcept;

		utils::FixedCapacityVector<Descriptor> descriptors;     // 16
		utils::bitset64 dynamicBuffers;                         // 8
		DescriptorSetLayoutHandle dslh;                         // 4
		uint8_t dynamicBufferCount = 0;                         // 1
	};
	static_assert(sizeof(GLDescriptorSet) <= 32);

} // namespace filament::backend

#endif //TNT_FILAMENT_BACKEND_OPENGL_GLDESCRIPTORSET_H
