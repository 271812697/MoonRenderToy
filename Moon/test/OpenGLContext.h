#pragma once
#include "DriverEnums.h"
#include "Handle.h"
#include "utils/bitset.h"
#include <glad/glad.h>
#include <array>
#include <functional>
#include <optional>
#include <tuple>
#include <vector>
#include <unordered_map>
#include <stddef.h>
#include <stdint.h>

namespace TEST {



	class OpenGLContext {
	public:
		static constexpr const size_t MAX_TEXTURE_UNIT_COUNT = MAX_SAMPLER_COUNT;
		static constexpr const size_t DUMMY_TEXTURE_BINDING = 7; // highest binding guaranteed to work with ES2
		static constexpr const size_t MAX_BUFFER_BINDINGS = 32;


		struct RenderPrimitive {
			static_assert(MAX_VERTEX_ATTRIBUTE_COUNT <= 16);

			unsigned int vao[2] = {};                                     // 8
			unsigned int elementArray = 0;                                // 4
			unsigned int indicesType = 0;                                 // 4

			// The optional 32-bit handle to a GLVertexBuffer is necessary only if the referenced
			// VertexBuffer supports buffer objects. If this is zero, then the VBO handles array is
			// immutable.
			Handle<HwVertexBuffer> vertexBufferWithObjects;         // 4

			mutable utils::bitset<uint16_t> vertexAttribArray;      // 2

			uint8_t reserved[2] = {};                               // 2

			// if this differs from vertexBufferWithObjects->bufferObjectsVersion, this VAO needs to
			// be updated (see OpenGLDriver::updateVertexArrayObject())
			uint8_t vertexBufferVersion = 0;                        // 1

			// if this differs from OpenGLContext::state.age, this VAO needs to
			// be updated (see OpenGLDriver::updateVertexArrayObject())
			uint8_t stateVersion = 0;                               // 1

			// If this differs from OpenGLContext::state.age, this VAO's name needs to be updated.
			// See OpenGLContext::bindVertexArray()
			uint8_t nameVersion = 0;                                // 1

			// Size in bytes of indices in the index buffer (1 or 2)
			uint8_t indicesShift = 0;                                // 1

			unsigned int getIndicesType() const noexcept {
				return indicesType;
			}
		} gl;

		//static bool queryOpenGLVersion(int* major, int* minor) noexcept;

		OpenGLContext() noexcept;

		~OpenGLContext() noexcept;

		void terminate() noexcept;



		size_t getIndexForCap(unsigned int cap) noexcept;
		static  size_t getIndexForBufferTarget(unsigned int target) noexcept;



		void resetState() noexcept;

		inline void useProgram(unsigned int program) noexcept;

		void pixelStore(unsigned int, int) noexcept;
		inline void activeTexture(unsigned int unit) noexcept;
		inline void bindTexture(unsigned int unit, unsigned int target, unsigned int texId) noexcept;

		void unbindTexture(unsigned int target, unsigned int id) noexcept;
		void unbindTextureUnit(unsigned int unit) noexcept;
		inline void bindVertexArray(RenderPrimitive const* p) noexcept;
		inline void bindSampler(unsigned int unit, unsigned int sampler) noexcept;
		void unbindSampler(unsigned int sampler) noexcept;

		void bindBuffer(unsigned int target, unsigned int buffer) noexcept;
		inline void bindBufferRange(unsigned int target, unsigned int index, unsigned int buffer,
			__int64 offset, __int64 size) noexcept;

		unsigned int bindFramebuffer(unsigned int target, unsigned int buffer) noexcept;
		void unbindFramebuffer(unsigned int target) noexcept;

		inline void enableVertexAttribArray(RenderPrimitive const* rp, unsigned int index) noexcept;
		inline void disableVertexAttribArray(RenderPrimitive const* rp, unsigned int index) noexcept;
		inline void enable(unsigned int cap) noexcept;
		inline void disable(unsigned int cap) noexcept;
		inline void frontFace(unsigned int mode) noexcept;
		inline void cullFace(unsigned int mode) noexcept;
		inline void blendEquation(unsigned int modeRGB, unsigned int modeA) noexcept;
		inline void blendFunction(unsigned int srcRGB, unsigned int srcA, unsigned int dstRGB, unsigned int dstA) noexcept;
		inline void colorMask(unsigned char flag) noexcept;
		inline void depthMask(unsigned char flag) noexcept;
		inline void depthFunc(unsigned int func) noexcept;
		inline void stencilFuncSeparate(unsigned int funcFront, int refFront, unsigned int maskFront,
			unsigned int funcBack, int refBack, unsigned int maskBack) noexcept;
		inline void stencilOpSeparate(unsigned int sfailFront, unsigned int dpfailFront, unsigned int dppassFront,
			unsigned int sfailBack, unsigned int dpfailBack, unsigned int dppassBack) noexcept;
		inline void stencilMaskSeparate(unsigned int maskFront, unsigned int maskBack) noexcept;
		inline void polygonOffset(float factor, float units) noexcept;

		inline void setScissor(int left, int bottom, int width, int height) noexcept;
		inline void viewport(int left, int bottom, int width, int height) noexcept;
		inline void depthRange(float near, float far) noexcept;

		void deleteBuffer(unsigned int buffer, unsigned int target) noexcept;
		void deleteVertexArray(unsigned int vao) noexcept;

		void destroyWithContext(size_t index, std::function<void(OpenGLContext&)> const& closure) noexcept;

		// glGet*() values
		struct Gets {
			float max_anisotropy;
			int max_combined_texture_image_units;
			int max_draw_buffers;
			int max_renderbuffer_size;
			int max_samples;
			int max_texture_image_units;
			int max_transform_feedback_separate_attribs;
			int max_uniform_block_size;
			int max_uniform_buffer_bindings;
			int num_program_binary_formats;
			int uniform_buffer_offset_alignment;
		} gets = {};

		// features supported by this version of GL or GLES
		struct {
			bool multisample_texture;
		} features = {};

		// supported extensions detected at runtime
		struct Extensions {
			bool APPLE_color_buffer_packed_float;
			bool ARB_shading_language_packing;
			bool EXT_clip_control;
			bool EXT_clip_cull_distance;
			bool EXT_color_buffer_float;
			bool EXT_color_buffer_half_float;
			bool EXT_debug_marker;
			bool EXT_depth_clamp;
			bool EXT_discard_framebuffer;
			bool EXT_disjoint_timer_query;
			bool EXT_multisampled_render_to_texture2;
			bool EXT_multisampled_render_to_texture;
			bool EXT_protected_textures;
			bool EXT_shader_framebuffer_fetch;
			bool EXT_texture_compression_bptc;
			bool EXT_texture_compression_etc2;
			bool EXT_texture_compression_rgtc;
			bool EXT_texture_compression_s3tc;
			bool EXT_texture_compression_s3tc_srgb;
			bool EXT_texture_cube_map_array;
			bool EXT_texture_filter_anisotropic;
			bool EXT_texture_sRGB;
			bool GOOGLE_cpp_style_line_directive;
			bool KHR_debug;
			bool KHR_parallel_shader_compile;
			bool KHR_texture_compression_astc_hdr;
			bool KHR_texture_compression_astc_ldr;
			bool OES_EGL_image_external_essl3;
			bool OES_depth24;
			bool OES_depth_texture;
			bool OES_packed_depth_stencil;
			bool OES_rgb8_rgba8;
			bool OES_standard_derivatives;
			bool OES_texture_npot;
			bool OES_vertex_array_object;
			bool OVR_multiview2;
			bool WEBGL_compressed_texture_etc;
			bool WEBGL_compressed_texture_s3tc;
			bool WEBGL_compressed_texture_s3tc_srgb;
		} ext = {};

		struct Bugs {
			// Some drivers have issues with UBOs in the fragment shader when
			// glFlush() is called between draw calls.
			bool disable_glFlush;

			// Some drivers seem to not store the GL_ELEMENT_ARRAY_BUFFER binding
			// in the VAO state.
			bool vao_doesnt_store_element_array_buffer_binding;

			// Some drivers have gl state issues when drawing from shared contexts
			bool disable_shared_context_draws;

			// Some web browsers seem to immediately clear the default framebuffer when calling
			// glInvalidateFramebuffer with WebGL 2.0
			bool disable_invalidate_framebuffer;

			// Some drivers declare GL_EXT_texture_filter_anisotropic but don't support
			// calling glSamplerParameter() with GL_TEXTURE_MAX_ANISOTROPY_EXT
			bool texture_filter_anisotropic_broken_on_sampler;

			// Some drivers have issues when reading from a mip while writing to a different mip.
			// In the OpenGL ES 3.0 specification this is covered in section 4.4.3,
			// "Feedback Loops Between Textures and the Framebuffer".
			bool disable_feedback_loops;

			// Some drivers don't implement timer queries correctly
			bool dont_use_timer_query;

			// Some drivers can't blit from a sidecar renderbuffer into a layer of a texture array.
			// This technique is used for VSM with MSAA turned on.
			bool disable_blit_into_texture_array;

			// Some drivers incorrectly flatten the early exit condition in the EASU code, in which
			// case we need an alternative algorithm
			bool split_easu;

			// As of Android R some qualcomm drivers invalidate buffers for the whole render pass
			// even if glInvalidateFramebuffer() is called at the end of it.
			bool invalidate_end_only_if_invalidate_start;

			// GLES doesn't allow feedback loops even if writes are disabled. So take we the point of
			// view that this is generally forbidden. However, this restriction is lifted on desktop
			// GL and Vulkan and probably Metal.
			bool allow_read_only_ancillary_feedback_loop;

			// Some Adreno drivers crash in glDrawXXX() when there's an uninitialized uniform block,
			// even when the shader doesn't access it.
			bool enable_initialize_non_used_uniform_array;

			// Workarounds specific to PowerVR GPUs affecting shaders (currently, we lump them all
			// under one specialization constant).
			// - gl_InstanceID is invalid when used first in the vertex shader
			bool powervr_shader_workarounds;

			// On PowerVR destroying the destination of a glBlitFramebuffer operation is equivalent to
			// a glFinish. So we must delay the destruction until we know the GPU is finished.
			bool delay_fbo_destruction;

			// Mesa sometimes clears the generic buffer binding when *another* buffer is destroyed,
			// if that other buffer is bound on an *indexed* buffer binding.
			bool rebind_buffer_after_deletion;

			// Force feature level 0. Typically used for low end ES3 devices with significant driver
			// bugs or performance issues.
			bool force_feature_level0;


		} bugs = {};

		// state getters -- as needed.
		//vec4gli const& getViewport() const { return state.window.viewport; }

		// function to handle state changes we don't control
		void updateTexImage(unsigned int target, unsigned int id) noexcept {
			//GL_TEXTURE_EXTERNAL_OES=36197
			assert(target == 36197);
			// if another target is bound to this texture unit, unbind that texture
			if (state.textures.units[state.textures.active].target != target) {
				glBindTexture(state.textures.units[state.textures.active].target, 0);
				state.textures.units[state.textures.active].target = 36197;
			}
			// the texture is already bound to `target`, we just update our internal state
			state.textures.units[state.textures.active].id = id;
		}
		void resetProgram() noexcept { state.program.use = 0; }


		// This is the index of the context in use. Must be 0 or 1. This is used to manange the
		// OpenGL name of ContainerObjects within each context.
		uint32_t contextIndex = 0;

		// Try to keep the State structure sorted by data-access patterns
		struct State {
			State() noexcept = default;
			// make sure we don't copy this state by accident
			State(State const& rhs) = delete;
			State(State&& rhs) noexcept = delete;
			State& operator=(State const& rhs) = delete;
			State& operator=(State&& rhs) noexcept = delete;

			int major = 0;
			int minor = 0;

			char const* vendor = nullptr;
			char const* renderer = nullptr;
			char const* version = nullptr;
			char const* shader = nullptr;

			unsigned int draw_fbo = 0;
			unsigned int read_fbo = 0;

			struct {
				unsigned int use = 0;
			} program;

			struct {
				RenderPrimitive* p = nullptr;
			} vao;

			struct {
				unsigned int frontFace = GL_CCW;
				unsigned int cullFace = GL_BACK;
				unsigned int blendEquationRGB = GL_FUNC_ADD;
				unsigned int blendEquationA = GL_FUNC_ADD;
				unsigned int blendFunctionSrcRGB = GL_ONE;
				unsigned int blendFunctionSrcA = GL_ONE;
				unsigned int blendFunctionDstRGB = GL_ZERO;
				unsigned int blendFunctionDstA = GL_ZERO;
				unsigned char colorMask = GL_TRUE;
				unsigned char depthMask = GL_TRUE;
				unsigned int depthFunc = GL_LESS;
			} raster;

			struct {
				struct StencilFunc {
					unsigned int func = GL_ALWAYS;
					int ref = 0;
					unsigned int mask = ~unsigned int(0);
					bool operator != (StencilFunc const& rhs) const noexcept {
						return func != rhs.func || ref != rhs.ref || mask != rhs.mask;
					}
				};
				struct StencilOp {
					unsigned int sfail = GL_KEEP;
					unsigned int dpfail = GL_KEEP;
					unsigned int dppass = GL_KEEP;
					bool operator != (StencilOp const& rhs) const noexcept {
						return sfail != rhs.sfail || dpfail != rhs.dpfail || dppass != rhs.dppass;
					}
				};
				struct {
					StencilFunc func;
					StencilOp op;
					unsigned int stencilMask = ~unsigned int(0);
				} front, back;
			} stencil;

			struct PolygonOffset {
				float factor = 0;
				float units = 0;
				bool operator != (PolygonOffset const& rhs) const noexcept {
					return factor != rhs.factor || units != rhs.units;
				}
			} polygonOffset;

			struct {
				utils::bitset32 caps;
			} enables;

			struct {
				struct {
					struct {
						unsigned int name = 0;
						__int64 offset = 0;
						__int64 size = 0;
					} buffers[MAX_BUFFER_BINDINGS];
				} targets[3];   // there are only 3 indexed buffer targets
				unsigned int genericBinding[7] = {};
			} buffers;

			struct {
				unsigned int active = 0;      // zero-based
				struct {
					unsigned int sampler = 0;
					unsigned int target = 0;
					unsigned int id = 0;
				} units[MAX_TEXTURE_UNIT_COUNT];
			} textures;

			struct {
				int row_length = 0;
				int alignment = 4;
			} unpack;

			struct {
				int alignment = 4;
			} pack;

			// struct {
			 //    vec4gli scissor { 0 };
			 //    vec4gli viewport { 0 };
			 //    vec2glf depthRange { 0.0f, 1.0f };
			// } window;
			uint8_t age = 0;
		} state;

		struct Procs {
			void (*bindVertexArray)(unsigned int array);
			void (*deleteVertexArrays)(int n, const unsigned int* arrays);
			void (*genVertexArrays)(int n, unsigned int* arrays);

			void (*genQueries)(int n, unsigned int* ids);
			void (*deleteQueries)(int n, const unsigned int* ids);
			void (*beginQuery)(unsigned int target, unsigned int id);
			void (*endQuery)(unsigned int target);
			void (*getQueryObjectuiv)(unsigned int id, unsigned int pname, unsigned int* params);
			void (*getQueryObjectui64v)(unsigned int id, unsigned int pname, GLuint64* params);

			void (*invalidateFramebuffer)(unsigned int target, int numAttachments, const unsigned int* attachments);

			void (*maxShaderCompilerThreadsKHR)(unsigned int count);
		} procs{};

		void unbindEverything() noexcept;
		void synchronizeStateAndCache(size_t index) noexcept;


		unsigned int getSamplerSlow(SamplerParams sp) const noexcept;

		inline unsigned int getSampler(SamplerParams sp) const noexcept {
			assert(!sp.padding0);
			assert(!sp.padding1);
			assert(!sp.padding2);
			auto& samplerMap = mSamplerMap;
			auto pos = samplerMap.find(sp);
			if (pos == samplerMap.end()) {
				return getSamplerSlow(sp);
			}
			return pos->second;
		}


	private:

		std::vector<std::function<void(OpenGLContext&)>> mDestroyWithNormalContext;
		RenderPrimitive mDefaultVAO;
		std::optional<unsigned int> mDefaultFbo[2];
		mutable std::unordered_map<SamplerParams, unsigned int,
			SamplerParams::Hasher, SamplerParams::EqualTo> mSamplerMap;


		void bindFramebufferResolved(unsigned int target, unsigned int buffer) noexcept;

		const std::array<std::tuple<bool const&, char const*, char const*>, sizeof(bugs)> mBugDatabase{ {
				{   bugs.disable_glFlush,
						"disable_glFlush",
						""},
				{   bugs.vao_doesnt_store_element_array_buffer_binding,
						"vao_doesnt_store_element_array_buffer_binding",
						""},
				{   bugs.disable_shared_context_draws,
						"disable_shared_context_draws",
						""},
				{   bugs.disable_invalidate_framebuffer,
						"disable_invalidate_framebuffer",
						""},
				{   bugs.texture_filter_anisotropic_broken_on_sampler,
						"texture_filter_anisotropic_broken_on_sampler",
						""},
				{   bugs.disable_feedback_loops,
						"disable_feedback_loops",
						""},
				{   bugs.dont_use_timer_query,
						"dont_use_timer_query",
						""},
				{   bugs.disable_blit_into_texture_array,
						"disable_blit_into_texture_array",
						""},
				{   bugs.split_easu,
						"split_easu",
						""},
				{   bugs.invalidate_end_only_if_invalidate_start,
						"invalidate_end_only_if_invalidate_start",
						""},
				{   bugs.allow_read_only_ancillary_feedback_loop,
						"allow_read_only_ancillary_feedback_loop",
						""},
				{   bugs.enable_initialize_non_used_uniform_array,
						"enable_initialize_non_used_uniform_array",
						""},
				{   bugs.powervr_shader_workarounds,
						"powervr_shader_workarounds",
						""},
				{   bugs.delay_fbo_destruction,
						"delay_fbo_destruction",
						""},
				{   bugs.rebind_buffer_after_deletion,
						"rebind_buffer_after_deletion",
						""},
				{   bugs.force_feature_level0,
						"force_feature_level0",
						""},
		} };


		static void initBugs(Bugs* bugs, Extensions const& exts,
			int major, int minor,
			char const* vendor,
			char const* renderer,
			char const* version,
			char const* shader
		);

		static void initProcs(Procs* procs,
			Extensions const& exts, int major, int minor) noexcept;


		template <typename T, typename F>
		static inline void update_state(T& state, T const& expected, F functor, bool force = false) noexcept {
			if ((force || state != expected)) {
				state = expected;
				functor();
			}
		}

		void setDefaultState() noexcept;
	};

	// ------------------------------------------------------------------------------------------------

	size_t OpenGLContext::getIndexForCap(unsigned int cap) noexcept { //NOLINT
		size_t index = 0;
		switch (cap) {
		case GL_BLEND:                          index = 0; break;
		case GL_CULL_FACE:                      index = 1; break;
		case GL_SCISSOR_TEST:                   index = 2; break;
		case GL_DEPTH_TEST:                     index = 3; break;
		case GL_STENCIL_TEST:                   index = 4; break;
		case GL_DITHER:                         index = 5; break;
		case GL_SAMPLE_ALPHA_TO_COVERAGE:       index = 6; break;
		case GL_SAMPLE_COVERAGE:                index = 7; break;
		case GL_POLYGON_OFFSET_FILL:            index = 8; break;
#ifdef GL_ARB_seamless_cube_map
		case GL_TEXTURE_CUBE_MAP_SEAMLESS:      index = 9; break;
#endif

		case GL_PROGRAM_POINT_SIZE:             index = 10; break;

		case GL_DEPTH_CLAMP:                    index = 11; break;
		default: break;
		}
		assert(index < state.enables.caps.size());
		return index;
	}

	size_t OpenGLContext::getIndexForBufferTarget(unsigned int target) noexcept {
		size_t index = 0;
		switch (target) {

		case GL_UNIFORM_BUFFER:             index = 0; break;
		case GL_TRANSFORM_FEEDBACK_BUFFER:  index = 1; break;
		case GL_SHADER_STORAGE_BUFFER:      index = 2; break;

		case GL_ARRAY_BUFFER:               index = 3; break;
		case GL_ELEMENT_ARRAY_BUFFER:       index = 4; break;
		case GL_PIXEL_PACK_BUFFER:          index = 5; break;
		case GL_PIXEL_UNPACK_BUFFER:        index = 6; break;
		default: break;
		}
		assert(index < sizeof(state.buffers.genericBinding) / sizeof(state.buffers.genericBinding[0])); // NOLINT(misc-redundant-expression)
		return index;
	}

	// ------------------------------------------------------------------------------------------------

	void OpenGLContext::activeTexture(unsigned int unit) noexcept {
		assert(unit < MAX_TEXTURE_UNIT_COUNT);
		update_state(state.textures.active, unit, [&]() {
			glActiveTexture(GL_TEXTURE0 + unit);
			});
	}

	void OpenGLContext::bindSampler(unsigned int unit, unsigned int sampler) noexcept {
		assert(unit < MAX_TEXTURE_UNIT_COUNT);

		update_state(state.textures.units[unit].sampler, sampler, [&]() {
			glBindSampler(unit, sampler);
			});

	}

	void OpenGLContext::setScissor(int left, int bottom, int width, int height) noexcept {
		//to do
	}

	void OpenGLContext::viewport(int left, int bottom, int width, int height) noexcept {

	}

	void OpenGLContext::depthRange(float near, float far) noexcept {
		//to do

	}

	void OpenGLContext::bindVertexArray(RenderPrimitive const* p) noexcept {
		RenderPrimitive* vao = p ? const_cast<RenderPrimitive*>(p) : &mDefaultVAO;
		update_state(state.vao.p, vao, [&]() {

			// See if we need to create a name for this VAO on the fly, this would happen if:
			// - we're not the default VAO, because its name is always 0
			// - our name is 0, this could happen if this VAO was created in the "other" context
			// - the nameVersion is out of date *and* we're on the protected context, in this case:
			//      - the name must be stale from a previous use of this context because we always
			//        destroy the protected context when we're done with it.
			bool const recreateVaoName = p != &mDefaultVAO &&
				((vao->vao[contextIndex] == 0) ||
					(vao->nameVersion != state.age && contextIndex == 1));
			if ((recreateVaoName)) {
				vao->nameVersion = state.age;
				procs.genVertexArrays(1, &vao->vao[contextIndex]);
			}

			procs.bindVertexArray(vao->vao[contextIndex]);
			// update GL_ELEMENT_ARRAY_BUFFER, which is updated by glBindVertexArray
			size_t const targetIndex = getIndexForBufferTarget(GL_ELEMENT_ARRAY_BUFFER);
			state.buffers.genericBinding[targetIndex] = vao->elementArray;
			if ((bugs.vao_doesnt_store_element_array_buffer_binding || recreateVaoName)) {
				// This shouldn't be needed, but it looks like some drivers don't do the implicit
				// glBindBuffer().
				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vao->elementArray);
			}
			});
	}

	void OpenGLContext::bindBufferRange(unsigned int target, unsigned int index, unsigned int buffer,
		__int64 offset, __int64 size) noexcept {

		assert(false || target == GL_UNIFORM_BUFFER || target == GL_TRANSFORM_FEEDBACK_BUFFER || target == GL_SHADER_STORAGE_BUFFER);

		size_t const targetIndex = getIndexForBufferTarget(target);
		// this ALSO sets the generic binding
		assert(targetIndex < sizeof(state.buffers.targets) / sizeof(*state.buffers.targets));
		if (state.buffers.targets[targetIndex].buffers[index].name != buffer
			|| state.buffers.targets[targetIndex].buffers[index].offset != offset
			|| state.buffers.targets[targetIndex].buffers[index].size != size) {
			state.buffers.targets[targetIndex].buffers[index].name = buffer;
			state.buffers.targets[targetIndex].buffers[index].offset = offset;
			state.buffers.targets[targetIndex].buffers[index].size = size;
			state.buffers.genericBinding[targetIndex] = buffer;
			glBindBufferRange(target, index, buffer, offset, size);
		}

	}

	void OpenGLContext::bindTexture(unsigned int unit, unsigned int target, unsigned int texId) noexcept {
		update_state(state.textures.units[unit].target, target, [&]() {
			activeTexture(unit);
			glBindTexture(state.textures.units[unit].target, 0);
			});
		update_state(state.textures.units[unit].id, texId, [&]() {
			activeTexture(unit);
			glBindTexture(target, texId);
			//GL_TEXTURE_EXTERNAL_OES
			}, target == 31697);
	}

	void OpenGLContext::useProgram(unsigned int program) noexcept {
		update_state(state.program.use, program, [&]() {
			glUseProgram(program);
			});
	}

	void OpenGLContext::enableVertexAttribArray(RenderPrimitive const* rp, unsigned int index) noexcept {
		assert(rp);
		assert(index < rp->vertexAttribArray.size());
		bool const force = rp->stateVersion != state.age;
		if ((force || !rp->vertexAttribArray[index])) {
			rp->vertexAttribArray.set(index);
			glEnableVertexAttribArray(index);
		}
	}

	void OpenGLContext::disableVertexAttribArray(RenderPrimitive const* rp, unsigned int index) noexcept {
		assert(rp);
		assert(index < rp->vertexAttribArray.size());
		bool const force = rp->stateVersion != state.age;
		if ((force || rp->vertexAttribArray[index])) {
			rp->vertexAttribArray.unset(index);
			glDisableVertexAttribArray(index);
		}
	}

	void OpenGLContext::enable(unsigned int cap) noexcept {
		size_t const index = getIndexForCap(cap);
		if ((!state.enables.caps[index])) {
			state.enables.caps.set(index);
			glEnable(cap);
		}
	}

	void OpenGLContext::disable(unsigned int cap) noexcept {
		size_t const index = getIndexForCap(cap);
		if ((state.enables.caps[index])) {
			state.enables.caps.unset(index);
			glDisable(cap);
		}
	}

	void OpenGLContext::frontFace(unsigned int mode) noexcept {
		update_state(state.raster.frontFace, mode, [&]() {
			glFrontFace(mode);
			});
	}

	void OpenGLContext::cullFace(unsigned int mode) noexcept {
		update_state(state.raster.cullFace, mode, [&]() {
			glCullFace(mode);
			});
	}

	void OpenGLContext::blendEquation(unsigned int modeRGB, unsigned int modeA) noexcept {
		if ((
			state.raster.blendEquationRGB != modeRGB || state.raster.blendEquationA != modeA)) {
			state.raster.blendEquationRGB = modeRGB;
			state.raster.blendEquationA = modeA;
			glBlendEquationSeparate(modeRGB, modeA);
		}
	}

	void OpenGLContext::blendFunction(unsigned int srcRGB, unsigned int srcA, unsigned int dstRGB, unsigned int dstA) noexcept {
		if ((
			state.raster.blendFunctionSrcRGB != srcRGB ||
			state.raster.blendFunctionSrcA != srcA ||
			state.raster.blendFunctionDstRGB != dstRGB ||
			state.raster.blendFunctionDstA != dstA)) {
			state.raster.blendFunctionSrcRGB = srcRGB;
			state.raster.blendFunctionSrcA = srcA;
			state.raster.blendFunctionDstRGB = dstRGB;
			state.raster.blendFunctionDstA = dstA;
			glBlendFuncSeparate(srcRGB, dstRGB, srcA, dstA);
		}
	}

	void OpenGLContext::colorMask(unsigned char flag) noexcept {
		update_state(state.raster.colorMask, flag, [&]() {
			glColorMask(flag, flag, flag, flag);
			});
	}
	void OpenGLContext::depthMask(unsigned char flag) noexcept {
		update_state(state.raster.depthMask, flag, [&]() {
			glDepthMask(flag);
			});
	}

	void OpenGLContext::depthFunc(unsigned int func) noexcept {
		update_state(state.raster.depthFunc, func, [&]() {
			glDepthFunc(func);
			});
	}

	void OpenGLContext::stencilFuncSeparate(unsigned int funcFront, int refFront, unsigned int maskFront,
		unsigned int funcBack, int refBack, unsigned int maskBack) noexcept {
		update_state(state.stencil.front.func, { funcFront, refFront, maskFront }, [&]() {
			glStencilFuncSeparate(GL_FRONT, funcFront, refFront, maskFront);
			});
		update_state(state.stencil.back.func, { funcBack, refBack, maskBack }, [&]() {
			glStencilFuncSeparate(GL_BACK, funcBack, refBack, maskBack);
			});
	}

	void OpenGLContext::stencilOpSeparate(unsigned int sfailFront, unsigned int dpfailFront, unsigned int dppassFront,
		unsigned int sfailBack, unsigned int dpfailBack, unsigned int dppassBack) noexcept {
		update_state(state.stencil.front.op, { sfailFront, dpfailFront, dppassFront }, [&]() {
			glStencilOpSeparate(GL_FRONT, sfailFront, dpfailFront, dppassFront);
			});
		update_state(state.stencil.back.op, { sfailBack, dpfailBack, dppassBack }, [&]() {
			glStencilOpSeparate(GL_BACK, sfailBack, dpfailBack, dppassBack);
			});
	}

	void OpenGLContext::stencilMaskSeparate(unsigned int maskFront, unsigned int maskBack) noexcept {
		update_state(state.stencil.front.stencilMask, maskFront, [&]() {
			glStencilMaskSeparate(GL_FRONT, maskFront);
			});
		update_state(state.stencil.back.stencilMask, maskBack, [&]() {
			glStencilMaskSeparate(GL_BACK, maskBack);
			});
	}

	void OpenGLContext::polygonOffset(float factor, float units) noexcept {
		update_state(state.polygonOffset, { factor, units }, [&]() {
			if (factor != 0 || units != 0) {
				glPolygonOffset(factor, units);
				enable(GL_POLYGON_OFFSET_FILL);
			}
			else {
				disable(GL_POLYGON_OFFSET_FILL);
			}
			});
	}

}