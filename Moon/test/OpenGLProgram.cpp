#include "OpenGLProgram.h"
#include "utils/GLUtils.h"
//#include "GLTexture.h"
#include "core/log.h"
#include "Driver.h"
#include "DriverEnums.h"
#include "Program.h"
#include "Handle.h"
#include "utils/FixedCapacityVector.h"//<>
#include <algorithm>
#include <array>
#include <algorithm>
#include <new>
#include <string_view>
#include <utility>
#include <stddef.h>
#include <stdint.h>

namespace TEST {

	//using namespace filament::math;
	using namespace utils;
	//using namespace backend;

	struct OpenGLProgram::LazyInitializationData {
		Program::DescriptorSetInfo descriptorBindings;
		Program::BindingUniformsInfo bindingUniformInfo;
		utils::FixedCapacityVector<Program::PushConstant> vertexPushConstants;
		utils::FixedCapacityVector<Program::PushConstant> fragmentPushConstants;
	};


	OpenGLProgram::OpenGLProgram() noexcept = default;

	OpenGLProgram::OpenGLProgram(Driver& gld, Program&& program) noexcept
		: HwProgram(std::move(program.getName())), mRec709Location(-1) {
		auto* const lazyInitializationData = new(std::nothrow) LazyInitializationData();

		lazyInitializationData->vertexPushConstants = std::move(program.getPushConstants(ShaderStage::VERTEX));
		lazyInitializationData->fragmentPushConstants = std::move(program.getPushConstants(ShaderStage::FRAGMENT));
		lazyInitializationData->descriptorBindings = std::move(program.getDescriptorBindings());

		ShaderCompilerService& compiler = gld.getShaderCompilerService();
		mToken = compiler.createProgram(name, std::move(program));

		ShaderCompilerService::setUserData(mToken, lazyInitializationData);
		//use(&gld, *gld.mContext.get());
	}

	OpenGLProgram::~OpenGLProgram() noexcept {
		if (mToken) {
			// if the token is non-nullptr it means the program has not been used, and
			// we need to clean-up.
			assert(gl.program == 0);

			LazyInitializationData* const lazyInitializationData =
				(LazyInitializationData*)ShaderCompilerService::getUserData(mToken);
			delete lazyInitializationData;

			ShaderCompilerService::terminate(mToken);
		}

		//delete [] mUniformsRecords;
		const GLuint program = gl.program;
		if (program) {
			glDeleteProgram(program);
		}
	}

	bool OpenGLProgram::use(Driver* const gld, OpenGLContext& context) {

		if ((mToken && !gl.program)) {
			// first time a program is used
			initialize(*gld);
		}

		if ((!gl.program)) {
			CORE_ERROR("The glprogram is invalid");
			assert(!mToken);
			return false;
		}

		context.useProgram(gl.program);
		return true;
	}

	void OpenGLProgram::initialize(Driver& gld) {

		assert(gl.program == 0);
		assert(mToken);

		LazyInitializationData* const lazyInitializationData =
			(LazyInitializationData*)ShaderCompilerService::getUserData(mToken);

		ShaderCompilerService& compiler = gld.getShaderCompilerService();
		gl.program = compiler.getProgram(mToken);

		assert(mToken == nullptr);
		if (gl.program) {
			assert(lazyInitializationData);
			initializeProgramState(gld.getContext(), gl.program, *lazyInitializationData);
			delete lazyInitializationData;
		}
	}

	/*
	 * Initializes our internal state from a valid program. This must only be called after
	 * checkProgramStatus() has been successfully called.
	 */
	void OpenGLProgram::initializeProgramState(OpenGLContext& context, GLuint program,
		LazyInitializationData& lazyInitializationData) noexcept {
		// from the pipeline layout we compute a mapping from {set, binding} to {binding}
		// for both buffers and textures
		for (auto&& entry : lazyInitializationData.descriptorBindings) {
			std::sort(entry.begin(), entry.end(),
				[](Program::Descriptor const& lhs, Program::Descriptor const& rhs) {
					return lhs.binding < rhs.binding;
				});
		}

		GLuint tmu = 0;
		GLuint binding = 0;

		// needed for samplers
		context.useProgram(program);


		for (descriptor_set_t set = 0; set < MAX_DESCRIPTOR_SET_COUNT; set++) {
			for (Program::Descriptor const& entry : lazyInitializationData.descriptorBindings[set]) {
				switch (entry.type) {
				case DescriptorType::UNIFORM_BUFFER:
				case DescriptorType::SHADER_STORAGE_BUFFER: {
					if (!entry.name.empty()) {
#ifndef FILAMENT_SILENCE_NOT_SUPPORTED_BY_ES2
						if (true) {
							GLuint const index = glGetUniformBlockIndex(program,
								entry.name.c_str());
							if (index != GL_INVALID_INDEX) {
								// this can fail if the program doesn't use this descriptor
								glUniformBlockBinding(program, index, binding);
								mBindingMap.insert(set, entry.binding,
									{ binding, entry.type });
								++binding;
							}
						}
						else
#endif
						{
							auto pos = std::find_if(lazyInitializationData.bindingUniformInfo.begin(),
								lazyInitializationData.bindingUniformInfo.end(),
								[&name = entry.name](const auto& item) {
									return std::get<1>(item) == name;
								});
							if (pos != lazyInitializationData.bindingUniformInfo.end()) {
								binding = std::get<0>(*pos);
								mBindingMap.insert(set, entry.binding, { binding, entry.type });
							}
						}
					}
					break;
				}
				case DescriptorType::SAMPLER:
				case DescriptorType::SAMPLER_EXTERNAL: {
					if (!entry.name.empty()) {
						GLint const loc = glGetUniformLocation(program, entry.name.c_str());
						if (loc >= 0) {
							// this can fail if the program doesn't use this descriptor
							mBindingMap.insert(set, entry.binding, { tmu, entry.type });
							glUniform1i(loc, GLint(tmu));
							++tmu;
						}
					}
					break;
				}
				case DescriptorType::INPUT_ATTACHMENT:
					break;
				}
				CHECK_GL_ERROR
			}
		}

		if (true) {
			// ES2 initialization of (fake) UBOs
			UniformsRecord* const uniformsRecords = new(std::nothrow) UniformsRecord[Program::UNIFORM_BINDING_COUNT];

			for (auto&& [index, name, uniforms] : lazyInitializationData.bindingUniformInfo) {
				uniformsRecords[index].locations.reserve(uniforms.size());
				uniformsRecords[index].locations.resize(uniforms.size());
				for (size_t j = 0, c = uniforms.size(); j < c; j++) {
					GLint const loc = glGetUniformLocation(program, uniforms[j].name.c_str());
					uniformsRecords[index].locations[j] = loc;
					if ((index == 0)) {
						// This is a bit of a gross hack here, we stash the location of
						// "frameUniforms.rec709", which obviously the backend shouldn't know about,
						// which is used for emulating the "rec709" colorspace in the shader.
						// The backend also shouldn't know that binding 0 is where frameUniform is.
						std::string_view const uniformName{
								uniforms[j].name.data(), uniforms[j].name.size() };
						if (uniformName == "frameUniforms.rec709") {
							mRec709Location = loc;
						}
					}
				}
				uniformsRecords[index].uniforms = std::move(uniforms);
			}
			// mUniformsRecords = uniformsRecords;
		}

		auto& vertexConstants = lazyInitializationData.vertexPushConstants;
		auto& fragmentConstants = lazyInitializationData.fragmentPushConstants;

		size_t const totalConstantCount = vertexConstants.size() + fragmentConstants.size();
		if (totalConstantCount > 0) {
			mPushConstants.reserve(totalConstantCount);
			mPushConstantFragmentStageOffset = vertexConstants.size();
			auto const transformAndAdd = [&](Program::PushConstant const& constant) {
				GLint const loc = glGetUniformLocation(program, constant.name.c_str());
				mPushConstants.push_back({ loc, constant.type });
				};
			std::for_each(vertexConstants.cbegin(), vertexConstants.cend(), transformAndAdd);
			std::for_each(fragmentConstants.cbegin(), fragmentConstants.cend(), transformAndAdd);
		}
	}

	void OpenGLProgram::updateUniforms(
		uint32_t index, GLuint id, void const* buffer, uint16_t age) const noexcept {
		//assert(mUniformsRecords);
		assert(buffer);

		// only update the uniforms if the UBO has changed since last time we updated
		UniformsRecord const& records = mUniformsRecords[index];
		if (records.id == id && records.age == age) {
			return;
		}
		records.id = id;
		records.age = age;

		assert(records.uniforms.size() == records.locations.size());

		for (size_t i = 0, c = records.uniforms.size(); i < c; i++) {
			Program::Uniform const& u = records.uniforms[i];
			GLint const loc = records.locations[i];
			// mRec709Location is special, it is handled by setRec709ColorSpace() and the corresponding
			// entry in `buffer` is typically not initialized, so we skip it.
			if (loc < 0 || loc == mRec709Location) {
				continue;
			}
			// u.offset is in 'uint32_t' units
			GLfloat const* const bf = reinterpret_cast<GLfloat const*>(buffer) + u.offset;
			GLint const* const bi = reinterpret_cast<GLint const*>(buffer) + u.offset;

			switch (u.type) {
			case UniformType::FLOAT:
				glUniform1fv(loc, u.size, bf);
				break;
			case UniformType::FLOAT2:
				glUniform2fv(loc, u.size, bf);
				break;
			case UniformType::FLOAT3:
				glUniform3fv(loc, u.size, bf);
				break;
			case UniformType::FLOAT4:
				glUniform4fv(loc, u.size, bf);
				break;

			case UniformType::BOOL:
			case UniformType::INT:
			case UniformType::UINT:
				glUniform1iv(loc, u.size, bi);
				break;
			case UniformType::BOOL2:
			case UniformType::INT2:
			case UniformType::UINT2:
				glUniform2iv(loc, u.size, bi);
				break;
			case UniformType::BOOL3:
			case UniformType::INT3:
			case UniformType::UINT3:
				glUniform3iv(loc, u.size, bi);
				break;
			case UniformType::BOOL4:
			case UniformType::INT4:
			case UniformType::UINT4:
				glUniform4iv(loc, u.size, bi);
				break;

			case UniformType::MAT3:
				glUniformMatrix3fv(loc, u.size, GL_FALSE, bf);
				break;
			case UniformType::MAT4:
				glUniformMatrix4fv(loc, u.size, GL_FALSE, bf);
				break;

			case UniformType::STRUCT:
				// not supported
				break;
			}
		}
	}

	void OpenGLProgram::setRec709ColorSpace(bool rec709) const noexcept {
		glUniform1i(mRec709Location, rec709);
	}


} // namespace filament::backend
