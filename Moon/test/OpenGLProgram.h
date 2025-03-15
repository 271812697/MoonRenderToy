#pragma once
#include "DriverBase.h"
#include "BindingMap.h"
#include "OpenGLContext.h"
#include "Driver.h"
#include "DriverEnums.h"
#include "Program.h"
#include "utils/bitset.h"
#include "utils/FixedCapacityVector.h"
#include <limits>
#include <stddef.h>
#include <stdint.h>
#include <assert.h>

namespace TEST {

	class Driver;

	class OpenGLProgram : public HwProgram {
	public:

		OpenGLProgram() noexcept;
		OpenGLProgram(Driver& gld, Program&& program) noexcept;
		~OpenGLProgram() noexcept;

		// bool isValid() const noexcept { return mToken || gl.program != 0; }
		bool isValid() const noexcept { return gl.program != 0; }
		bool use(Driver* const gld, OpenGLContext& context);

		GLuint getBufferBinding(descriptor_set_t set, descriptor_binding_t binding) const noexcept {
			return mBindingMap.get(set, binding);
		}

		GLuint getTextureUnit(descriptor_set_t set, descriptor_binding_t binding) const noexcept {
			return mBindingMap.get(set, binding);
		}

		utils::bitset64 getActiveDescriptors(descriptor_set_t set) const noexcept {
			return mBindingMap.getActiveDescriptors(set);
		}
		void updateUniforms(uint32_t index, GLuint id, void const* buffer, uint16_t age) const noexcept;
		void setRec709ColorSpace(bool rec709) const noexcept;
	private:
		// keep these away from of other class attributes
		struct LazyInitializationData;

		void initialize(Driver& gld);

		void initializeProgramState(OpenGLContext& context, GLuint program,
			LazyInitializationData& lazyInitializationData) noexcept;

		BindingMap mBindingMap;     // 8 bytes + out-of-line 256 bytes
		ShaderCompilerService::program_token_t mToken{};    // 16 bytes
		utils::FixedCapacityVector<std::pair<GLint, ConstantType>> mPushConstants;// 16 bytes
		// only needed for ES2
		using LocationInfo = utils::FixedCapacityVector<GLint>;
		struct UniformsRecord {
			Program::UniformInfo uniforms;
			LocationInfo locations;
			mutable GLuint id = 0;
			mutable uint16_t age = std::numeric_limits<uint16_t>::max();
		};
		UniformsRecord const* mUniformsRecords = nullptr;
		GLint mRec709Location : 24;     // 4 bytes

		// Push constant array offset for fragment stage constants.
		GLint mPushConstantFragmentStageOffset : 8;      // 1 byte

	public:
		struct {
			GLuint program = 0;
		} gl;                                               // 4 bytes
	};

	// if OpenGLProgram is larger than 96 bytes, it'll fall in a larger Handle bucket.
	//static_assert(sizeof(OpenGLProgram) <= 96); // currently 96 bytes

} // namespace filament::backend
