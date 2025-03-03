#include "OpenGLContext.h"
#include "DriverEnums.h"
#include "utils/GLUtils.h"
#include <functional>
#include <string_view>
#include <utility>
#include <stddef.h>
#include <string.h>
#include <iostream>

// change to true to display all GL extensions in the console on start-up
#define DEBUG_PRINT_EXTENSIONS false

using namespace utils;

namespace TEST {
	static constexpr const size_t MAX_TEXTURE_UNIT_COUNT = MAX_SAMPLER_COUNT;
	static constexpr const size_t DUMMY_TEXTURE_BINDING = 7; // highest binding guaranteed to work with ES2
	static constexpr const size_t MAX_BUFFER_BINDINGS = 32;
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
			OpenGLContext::RenderPrimitive* p = nullptr;
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


	bool OpenGLContext::queryOpenGLVersion(int* major, int* minor) noexcept
	{
		// OpenGL version
		glGetIntegerv(GL_MAJOR_VERSION, major);
		glGetIntegerv(GL_MINOR_VERSION, minor);
		return (glGetError() == GL_NO_ERROR);
	}

	OpenGLContext::OpenGLContext() noexcept
		: mSamplerMap(32)
	{
		state.vao.p = &mDefaultVAO;
		// These queries work with all GL/GLES versions!
		state.vendor = (char const*)glGetString(GL_VENDOR);
		state.renderer = (char const*)glGetString(GL_RENDERER);
		state.version = (char const*)glGetString(GL_VERSION);
		state.shader = (char const*)glGetString(GL_SHADING_LANGUAGE_VERSION);
		queryOpenGLVersion(&state.major, &state.minor);
		OpenGLContext::initExtensions(&ext, state.major, state.minor);
		OpenGLContext::initProcs(&procs, ext, state.major, state.minor);
		OpenGLContext::initBugs(&bugs, ext, state.major, state.minor, state.vendor, state.renderer, state.version, state.shader);
		glGetIntegerv(GL_MAX_RENDERBUFFER_SIZE, &gets.max_renderbuffer_size);
		glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &gets.max_texture_image_units);
		glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, &gets.max_combined_texture_image_units);
		features.multisample_texture = true;
		if (true) {
			if (ext.EXT_texture_filter_anisotropic) {
				glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &gets.max_anisotropy);
			}
			glGetIntegerv(GL_MAX_DRAW_BUFFERS,
				&gets.max_draw_buffers);
			glGetIntegerv(GL_MAX_SAMPLES,
				&gets.max_samples);
			glGetIntegerv(GL_MAX_TRANSFORM_FEEDBACK_SEPARATE_ATTRIBS,
				&gets.max_transform_feedback_separate_attribs);
			glGetIntegerv(GL_MAX_UNIFORM_BLOCK_SIZE,
				&gets.max_uniform_block_size);
			glGetIntegerv(GL_MAX_UNIFORM_BUFFER_BINDINGS,
				&gets.max_uniform_buffer_bindings);
			glGetIntegerv(GL_NUM_PROGRAM_BINARY_FORMATS,
				&gets.num_program_binary_formats);
			glGetIntegerv(GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT,
				&gets.uniform_buffer_offset_alignment);
		}
		for (auto [enabled, name, _] : mBugDatabase) {
			if (enabled) {

			}
		}
		assert(gets.max_draw_buffers >= 4); // minspec
		setDefaultState();

#ifdef GL_EXT_texture_filter_anisotropic

		if (ext.EXT_texture_filter_anisotropic) {
			// make sure we don't have any error flag
			while (glGetError() != GL_NO_ERROR) {}

			// check that we can actually set the anisotropy on the sampler
			GLuint s;
			glGenSamplers(1, &s);
			glSamplerParameterf(s, GL_TEXTURE_MAX_ANISOTROPY_EXT, gets.max_anisotropy);
			if (glGetError() != GL_NO_ERROR) {
				// some drivers only allow to set the anisotropy on the texture itself
				bugs.texture_filter_anisotropic_broken_on_sampler = true;
			}
			glDeleteSamplers(1, &s);
		}

#endif



	}

	OpenGLContext::~OpenGLContext() noexcept {
		// note: this is called from the main thread. Can't do any GL calls.

	}

	void OpenGLContext::terminate() noexcept {
		// note: this is called from the backend thread


		for (auto& item : mSamplerMap) {
			unbindSampler(item.second);
			glDeleteSamplers(1, &item.second);
		}
		mSamplerMap.clear();

	}

	void OpenGLContext::destroyWithContext(
		size_t index, std::function<void(OpenGLContext&)> const& closure) noexcept {
		if (index == 0) {
			// Note: we only need to delay the destruction of objects on the unprotected context
			// (index 0) because the protected context is always immediately destroyed and all its
			// active objects and bindings are then automatically destroyed.
			// TODO: this is only guaranteed for EGLPlatform, but that's the only one we care about.
			mDestroyWithNormalContext.push_back(closure);
		}
	}

	void OpenGLContext::updateTexImage(unsigned int target, unsigned int id) noexcept
	{

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

	void OpenGLContext::resetProgram() noexcept
	{
		state.program.use = 0;
	}

	void OpenGLContext::unbindEverything() noexcept {
		// TODO:  we're supposed to unbind everything here so that resources don't get
		//        stuck in this context (contextIndex) when destroyed in the other context.
		//        However, because EGLPlatform always immediately destroys the protected context (1),
		//        the bindings will automatically be severed when we switch back to the default context.
		//        Since bindings now only exist in one context, we don't have a ref-counting issue to
		//        worry about.
	}

	void OpenGLContext::synchronizeStateAndCache(size_t index) noexcept {

		// if we're just switching back to context 0, run all the pending destructors
		if (index == 0) {
			auto list = std::move(mDestroyWithNormalContext);
			for (auto&& fn : list) {
				fn(*this);
			}
		}

		// the default FBO could be invalid
		mDefaultFbo[index].reset();

		contextIndex = index;
		resetState();
	}

	void OpenGLContext::setDefaultState() noexcept {
		// We need to make sure our internal state matches the GL state when we start.
		// (some of these calls may be unneeded as they might be the gl defaults)
		GLenum const caps[] = {
			GL_BLEND,
			GL_CULL_FACE,
			GL_SCISSOR_TEST,
			GL_DEPTH_TEST,
			GL_STENCIL_TEST,
			GL_DITHER,
			GL_SAMPLE_ALPHA_TO_COVERAGE,
			GL_SAMPLE_COVERAGE,
			GL_POLYGON_OFFSET_FILL,
		};


		for (auto const capi : caps) {
			size_t const capIndex = getIndexForCap(capi);
			if (state.enables.caps[capIndex]) {
				glEnable(capi);
			}
			else {
				glDisable(capi);
			}
		}

		// Point sprite size and seamless cubemap filtering are disabled by default in desktop GL.
		// In OpenGL ES, these flags do not exist because they are always on.

		glEnable(GL_PROGRAM_POINT_SIZE);
		enable(GL_PROGRAM_POINT_SIZE);


#ifdef GL_ARB_seamless_cube_map
		glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
		enable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
#endif

#ifdef GL_FRAGMENT_SHADER_DERIVATIVE_HINT
		glHint(GL_FRAGMENT_SHADER_DERIVATIVE_HINT, GL_NICEST);
#endif


		if (ext.EXT_clip_control) {

			glClipControl(GL_LOWER_LEFT, GL_ZERO_TO_ONE);

		}


		if (ext.EXT_clip_cull_distance) {
			glEnable(GL_CLIP_DISTANCE0);
			glEnable(GL_CLIP_DISTANCE1);
		}
	}


	void OpenGLContext::initProcs(Procs* procs,
		Extensions const& ext, GLint major, GLint) noexcept {
		(void)ext;
		(void)major;

		// default procs that can be overridden based on runtime version

		procs->genVertexArrays = glGenVertexArrays;
		procs->bindVertexArray = glBindVertexArray;
		procs->deleteVertexArrays = glDeleteVertexArrays;

		// these are core in GL and GLES 3.x
		procs->genQueries = glGenQueries;
		procs->deleteQueries = glDeleteQueries;
		procs->beginQuery = glBeginQuery;
		procs->endQuery = glEndQuery;
		procs->getQueryObjectuiv = glGetQueryObjectuiv;

		procs->getQueryObjectui64v = glGetQueryObjectui64v; // only core in GL


		// core in ES 3.0 and GL 4.3
		procs->invalidateFramebuffer = glInvalidateFramebuffer;


		// no-op if not supported
		procs->maxShaderCompilerThreadsKHR = +[](GLuint) {};


		procs->maxShaderCompilerThreadsKHR = glMaxShaderCompilerThreadsARB;

	}

	void OpenGLContext::initBugs(Bugs* bugs, Extensions const& exts,
		GLint major, GLint minor,
		char const* vendor,
		char const* renderer,
		char const* version,
		char const* shader) {

		(void)major;
		(void)minor;
		(void)vendor;
		(void)renderer;
		(void)version;
		(void)shader;

		const bool isAngle = strstr(renderer, "ANGLE");
		if (!isAngle) {
			if (strstr(renderer, "Adreno")) {
				// Qualcomm GPU
				bugs->invalidate_end_only_if_invalidate_start = true;

				// On Adreno (As of 3/20) timer query seem to return the CPU time, not the GPU time.
				bugs->dont_use_timer_query = true;

				// Blits to texture arrays are failing
				//   This bug continues to reproduce, though at times we've seen it appear to "go away".
				//   The standalone sample app that was written to show this problem still reproduces.
				//   The working hypothesis is that some other state affects this behavior.
				bugs->disable_blit_into_texture_array = true;

				// early exit condition is flattened in EASU code
				bugs->split_easu = true;

				// initialize the non-used uniform array for Adreno drivers.
				bugs->enable_initialize_non_used_uniform_array = true;

				int maj, min, driverMajor, driverMinor;
				int const c = sscanf(version, "OpenGL ES %d.%d V@%d.%d", // NOLINT(cert-err34-c)
					&maj, &min, &driverMajor, &driverMinor);
				if (c == 4) {
					// Workarounds based on version here.
					// Notes:
					//  bugs.invalidate_end_only_if_invalidate_start
					//  - appeared at least in
					//      "OpenGL ES 3.2 V@0490.0 (GIT@85da404, I46ff5fc46f, 1606794520) (Date:11/30/20)"
					//  - wasn't present in
					//      "OpenGL ES 3.2 V@0490.0 (GIT@0905e9f, Ia11ce2d146, 1599072951) (Date:09/02/20)"
					//  - has been confirmed fixed in V@570.1 by Qualcomm
					if (driverMajor < 490 || driverMajor > 570 ||
						(driverMajor == 570 && driverMinor >= 1)) {
						bugs->invalidate_end_only_if_invalidate_start = false;
					}
				}

				// qualcomm seems to have no problem with this (which is good for us)
				bugs->allow_read_only_ancillary_feedback_loop = true;

#ifndef __EMSCRIPTEN__
				// Older Adreno devices that support ES3.0 only tend to be extremely buggy, so we
				// fall back to ES2.0.
				if (major == 3 && minor == 0) {
					bugs->force_feature_level0 = true;
				}
#endif
			}
			else if (strstr(renderer, "Mali")) {
				// ARM GPU
				bugs->vao_doesnt_store_element_array_buffer_binding = true;
				if (strstr(renderer, "Mali-T")) {
					bugs->disable_glFlush = true;
					bugs->disable_shared_context_draws = true;
					// We have not verified that timer queries work on Mali-T, so we disable to be safe.
					bugs->dont_use_timer_query = true;
				}
				if (strstr(renderer, "Mali-G")) {
					// We have run into several problems with timer queries on Mali-Gxx:
					// - timer queries seem to cause memory corruptions in some cases on some devices
					//   (see b/233754398)
					//          - appeared at least in: "OpenGL ES 3.2 v1.r26p0-01eac0"
					//          - wasn't present in: "OpenGL ES 3.2 v1.r32p1-00pxl1"
					// - timer queries sometime crash with an NPE (see b/273759031)
					bugs->dont_use_timer_query = true;
				}
				// Mali seems to have no problem with this (which is good for us)
				bugs->allow_read_only_ancillary_feedback_loop = true;
			}
			else if (strstr(renderer, "Intel")) {
				// Intel GPU
				bugs->vao_doesnt_store_element_array_buffer_binding = true;
			}
			else if (strstr(renderer, "PowerVR")) {
				// PowerVR GPU
				// On PowerVR (Rogue GE8320) glFlush doesn't seem to do anything, in particular,
				// it doesn't kick the GPU earlier, so don't issue these calls as they seem to slow
				// things down.
				bugs->disable_glFlush = true;
				// On PowerVR (Rogue GE8320) using gl_InstanceID too early in the shader doesn't work.
				bugs->powervr_shader_workarounds = true;
				// On PowerVR (Rogue GE8320) destroying a fbo after glBlitFramebuffer is effectively
				// equivalent to glFinish.
				bugs->delay_fbo_destruction = true;
				// PowerVR seems to have no problem with this (which is good for us)
				bugs->allow_read_only_ancillary_feedback_loop = true;
			}
			else if (strstr(renderer, "Apple")) {
				// Apple GPU
			}
			else if (strstr(renderer, "Tegra") ||
				strstr(renderer, "GeForce") ||
				strstr(renderer, "NV")) {
				// NVIDIA GPU
			}
			else if (strstr(renderer, "Vivante")) {
				// Vivante GPU
			}
			else if (strstr(renderer, "AMD") ||
				strstr(renderer, "ATI")) {
				// AMD/ATI GPU
			}
			else if (strstr(vendor, "Mesa")) {
				// Seen on
				//  [Mesa],
				//  [llvmpipe (LLVM 17.0.6, 256 bits)],
				//  [4.5 (Core Profile) Mesa 24.0.6-1],
				//  [4.50]
				// not known which version are affected
				bugs->rebind_buffer_after_deletion = true;
			}
			else if (strstr(renderer, "Mozilla")) {
				bugs->disable_invalidate_framebuffer = true;
			}
		}
		else {
			// When running under ANGLE, it's a different set of workaround that we need.
			if (strstr(renderer, "Adreno")) {
				// Qualcomm GPU
				// early exit condition is flattened in EASU code
				// (that should be regardless of ANGLE, but we should double-check)
				bugs->split_easu = true;
			}
		}

#ifdef BACKEND_OPENGL_VERSION_GLES
#   ifndef IOS // IOS is guaranteed to have ES3.x
		if (UTILS_UNLIKELY(major == 2)) {
			if (UTILS_UNLIKELY(!exts.OES_vertex_array_object)) {
				// we activate this workaround path, which does the reset of array buffer
				bugs->vao_doesnt_store_element_array_buffer_binding = true;
			}
		}
#   endif // IOS
#else
		// feedback loops are allowed on GL desktop as long as writes are disabled
		bugs->allow_read_only_ancillary_feedback_loop = true;
#endif
	}



#ifdef BACKEND_OPENGL_VERSION_GLES

	void OpenGLContext::initExtensionsGLES(Extensions* ext, GLint major, GLint minor) noexcept {
		const char* const extensions = (const char*)glGetString(GL_EXTENSIONS);
		GLUtils::unordered_string_set const exts = GLUtils::split(extensions);
		if constexpr (DEBUG_PRINT_EXTENSIONS) {
			for (auto extension : exts) {
				slog.d << "\"" << std::string_view(extension) << "\"\n";
			}
			flush(slog.d);
		}

		// figure out and initialize the extensions we need
		using namespace std::literals;
		ext->APPLE_color_buffer_packed_float = exts.has("GL_APPLE_color_buffer_packed_float"sv);
#ifndef __EMSCRIPTEN__
		ext->EXT_clip_control = exts.has("GL_EXT_clip_control"sv);
#endif
		ext->EXT_clip_cull_distance = exts.has("GL_EXT_clip_cull_distance"sv);
		ext->EXT_color_buffer_float = exts.has("GL_EXT_color_buffer_float"sv);
		ext->EXT_color_buffer_half_float = exts.has("GL_EXT_color_buffer_half_float"sv);
#ifndef __EMSCRIPTEN__
		ext->EXT_debug_marker = exts.has("GL_EXT_debug_marker"sv);
#endif
		ext->EXT_depth_clamp = exts.has("GL_EXT_depth_clamp"sv);
		ext->EXT_discard_framebuffer = exts.has("GL_EXT_discard_framebuffer"sv);
#ifndef __EMSCRIPTEN__
		ext->EXT_disjoint_timer_query = exts.has("GL_EXT_disjoint_timer_query"sv);
		ext->EXT_multisampled_render_to_texture = exts.has("GL_EXT_multisampled_render_to_texture"sv);
		ext->EXT_multisampled_render_to_texture2 = exts.has("GL_EXT_multisampled_render_to_texture2"sv);
		ext->EXT_protected_textures = exts.has("GL_EXT_protected_textures"sv);
#endif
		ext->EXT_shader_framebuffer_fetch = exts.has("GL_EXT_shader_framebuffer_fetch"sv);
#ifndef __EMSCRIPTEN__
		ext->EXT_texture_compression_etc2 = true;
#endif
		ext->EXT_texture_compression_s3tc = exts.has("GL_EXT_texture_compression_s3tc"sv);
		ext->EXT_texture_compression_s3tc_srgb = exts.has("GL_EXT_texture_compression_s3tc_srgb"sv);
		ext->EXT_texture_compression_rgtc = exts.has("GL_EXT_texture_compression_rgtc"sv);
		ext->EXT_texture_compression_bptc = exts.has("GL_EXT_texture_compression_bptc"sv);
		ext->EXT_texture_cube_map_array = exts.has("GL_EXT_texture_cube_map_array"sv) || exts.has("GL_OES_texture_cube_map_array"sv);
		ext->GOOGLE_cpp_style_line_directive = exts.has("GL_GOOGLE_cpp_style_line_directive"sv);
		ext->KHR_debug = exts.has("GL_KHR_debug"sv);
		ext->KHR_parallel_shader_compile = exts.has("GL_KHR_parallel_shader_compile"sv);
		ext->KHR_texture_compression_astc_hdr = exts.has("GL_KHR_texture_compression_astc_hdr"sv);
		ext->KHR_texture_compression_astc_ldr = exts.has("GL_KHR_texture_compression_astc_ldr"sv);
		ext->OES_depth_texture = exts.has("GL_OES_depth_texture"sv);
		ext->OES_depth24 = exts.has("GL_OES_depth24"sv);
		ext->OES_packed_depth_stencil = exts.has("GL_OES_packed_depth_stencil"sv);
		ext->OES_EGL_image_external_essl3 = exts.has("GL_OES_EGL_image_external_essl3"sv);
		ext->OES_rgb8_rgba8 = exts.has("GL_OES_rgb8_rgba8"sv);
		ext->OES_standard_derivatives = exts.has("GL_OES_standard_derivatives"sv);
		ext->OES_texture_npot = exts.has("GL_OES_texture_npot"sv);
		ext->OES_vertex_array_object = exts.has("GL_OES_vertex_array_object"sv);
		ext->OVR_multiview2 = exts.has("GL_OVR_multiview2"sv);
		ext->WEBGL_compressed_texture_etc = exts.has("WEBGL_compressed_texture_etc"sv);
		ext->WEBGL_compressed_texture_s3tc = exts.has("WEBGL_compressed_texture_s3tc"sv);
		ext->WEBGL_compressed_texture_s3tc_srgb = exts.has("WEBGL_compressed_texture_s3tc_srgb"sv);

		// ES 3.2 implies EXT_color_buffer_float
		if (major > 3 || (major == 3 && minor >= 2)) {
			ext->EXT_color_buffer_float = true;
		}
		// ES 3.x implies EXT_discard_framebuffer and OES_vertex_array_object
		if (major >= 3) {
			ext->EXT_discard_framebuffer = true;
			ext->OES_vertex_array_object = true;
		}
	}

#endif // BACKEND_OPENGL_VERSION_GLES


	void OpenGLContext::initExtensionsGL(Extensions* ext, GLint major, GLint minor) noexcept {
		GLUtils::unordered_string_set exts;
		GLint n = 0;
		glGetIntegerv(GL_NUM_EXTENSIONS, &n);
		for (GLint i = 0; i < n; i++) {
			exts.emplace((const char*)glGetStringi(GL_EXTENSIONS, (GLuint)i));
		}
		if (true) {
			for (auto extension : exts) {
				std::cout << "\"" << std::string_view(extension) << "\"\n";
			}

		}

		using namespace std::literals;
		ext->APPLE_color_buffer_packed_float = true;  // Assumes core profile.
		ext->ARB_shading_language_packing = exts.has("GL_ARB_shading_language_packing"sv);
		ext->EXT_color_buffer_float = true;  // Assumes core profile.
		ext->EXT_color_buffer_half_float = true;  // Assumes core profile.
		ext->EXT_clip_cull_distance = true;
		ext->EXT_debug_marker = exts.has("GL_EXT_debug_marker"sv);
		ext->EXT_depth_clamp = true;
		ext->EXT_discard_framebuffer = false;
		ext->EXT_disjoint_timer_query = true;
		ext->EXT_multisampled_render_to_texture = false;
		ext->EXT_multisampled_render_to_texture2 = false;
		ext->EXT_shader_framebuffer_fetch = exts.has("GL_EXT_shader_framebuffer_fetch"sv);
		ext->EXT_texture_compression_bptc = exts.has("GL_EXT_texture_compression_bptc"sv);
		ext->EXT_texture_compression_etc2 = exts.has("GL_ARB_ES3_compatibility"sv);
		ext->EXT_texture_compression_rgtc = exts.has("GL_EXT_texture_compression_rgtc"sv);
		ext->EXT_texture_compression_s3tc = exts.has("GL_EXT_texture_compression_s3tc"sv);
		ext->EXT_texture_compression_s3tc_srgb = exts.has("GL_EXT_texture_compression_s3tc_srgb"sv);
		ext->EXT_texture_cube_map_array = true;
		ext->EXT_texture_filter_anisotropic = exts.has("GL_EXT_texture_filter_anisotropic"sv);
		ext->EXT_texture_sRGB = exts.has("GL_EXT_texture_sRGB"sv);
		ext->GOOGLE_cpp_style_line_directive = exts.has("GL_GOOGLE_cpp_style_line_directive"sv);
		ext->KHR_parallel_shader_compile = exts.has("GL_KHR_parallel_shader_compile"sv);
		ext->KHR_texture_compression_astc_hdr = exts.has("GL_KHR_texture_compression_astc_hdr"sv);
		ext->KHR_texture_compression_astc_ldr = exts.has("GL_KHR_texture_compression_astc_ldr"sv);
		ext->OES_depth_texture = true;
		ext->OES_depth24 = true;
		ext->OES_EGL_image_external_essl3 = false;
		ext->OES_rgb8_rgba8 = true;
		ext->OES_standard_derivatives = true;
		ext->OES_texture_npot = true;
		ext->OES_vertex_array_object = true;
		ext->OVR_multiview2 = exts.has("GL_OVR_multiview2"sv);
		ext->WEBGL_compressed_texture_etc = false;
		ext->WEBGL_compressed_texture_s3tc = false;
		ext->WEBGL_compressed_texture_s3tc_srgb = false;

		// OpenGL 4.2 implies ARB_shading_language_packing
		if (major > 4 || (major == 4 && minor >= 2)) {
			ext->ARB_shading_language_packing = true;
		}
		// OpenGL 4.3 implies EXT_discard_framebuffer
		if (major > 4 || (major == 4 && minor >= 3)) {
			ext->EXT_discard_framebuffer = true;
			ext->KHR_debug = true;
		}
		// OpenGL 4.5 implies EXT_clip_control
		if (major > 4 || (major == 4 && minor >= 5)) {
			ext->EXT_clip_control = true;
		}
	}



	GLuint OpenGLContext::bindFramebuffer(GLenum target, GLuint buffer) noexcept {
		if ((buffer == 0)) {
			// we're binding the default frame buffer, resolve its actual name
			auto& defaultFboForThisContext = mDefaultFbo[contextIndex];
			if ((!defaultFboForThisContext.has_value())) {
				assert(false);
				// defaultFboForThisContext = GLuint(mPlatform.getDefaultFramebufferObject());
			}
			buffer = defaultFboForThisContext.value();
		}
		bindFramebufferResolved(target, buffer);
		return buffer;
	}

	void OpenGLContext::unbindFramebuffer(GLenum target) noexcept {
		bindFramebufferResolved(target, 0);
	}

	void OpenGLContext::bindFramebufferResolved(GLenum target, GLuint buffer) noexcept {
		switch (target) {
		case GL_FRAMEBUFFER:
			if (state.draw_fbo != buffer || state.read_fbo != buffer) {
				state.draw_fbo = state.read_fbo = buffer;
				glBindFramebuffer(target, buffer);
			}
			break;
#ifndef FILAMENT_SILENCE_NOT_SUPPORTED_BY_ES2
		case GL_DRAW_FRAMEBUFFER:
			if (state.draw_fbo != buffer) {
				state.draw_fbo = buffer;
				glBindFramebuffer(target, buffer);
			}
			break;
		case GL_READ_FRAMEBUFFER:
			if (state.read_fbo != buffer) {
				state.read_fbo = buffer;
				glBindFramebuffer(target, buffer);
			}
			break;
#endif
		default:
			break;
		}
	}

	void OpenGLContext::bindBuffer(GLenum target, GLuint buffer) noexcept {
		if (target == GL_ELEMENT_ARRAY_BUFFER) {
			size_t targetIndex = getIndexForBufferTarget(GL_ELEMENT_ARRAY_BUFFER);
			// GL_ELEMENT_ARRAY_BUFFER is a special case, where the currently bound VAO remembers
			// the index buffer, unless there are no VAO bound (see: bindVertexArray)
			assert(state.vao.p);
			if (state.buffers.genericBinding[targetIndex] != buffer
				|| ((state.vao.p != &mDefaultVAO) && (state.vao.p->elementArray != buffer))) {
				state.buffers.genericBinding[targetIndex] = buffer;
				if (state.vao.p != &mDefaultVAO) {
					state.vao.p->elementArray = buffer;
				}
				glBindBuffer(target, buffer);
			}
		}
		else {
			size_t const targetIndex = getIndexForBufferTarget(target);
			update_state(state.buffers.genericBinding[targetIndex], buffer, [&]() {
				glBindBuffer(target, buffer);
				});
		}
	}

	void OpenGLContext::pixelStore(GLenum pname, GLint param) noexcept {
		GLint* pcur;

		// Note: GL_UNPACK_SKIP_PIXELS, GL_UNPACK_SKIP_ROWS and
		//       GL_PACK_SKIP_PIXELS, GL_PACK_SKIP_ROWS
		// are actually provided as conveniences to the programmer; they provide no functionality
		// that cannot be duplicated at the call site (e.g. glTexImage2D or glReadPixels)

		switch (pname) {
		case GL_PACK_ALIGNMENT:
			pcur = &state.pack.alignment;
			break;
		case GL_UNPACK_ALIGNMENT:
			pcur = &state.unpack.alignment;
			break;
#ifndef FILAMENT_SILENCE_NOT_SUPPORTED_BY_ES2
		case GL_UNPACK_ROW_LENGTH:
			assert(state.major > 2);
			pcur = &state.unpack.row_length;
			break;
#endif
		default:
			goto default_case;
		}

		if ((*pcur != param)) {
			*pcur = param;
		default_case:
			glPixelStorei(pname, param);
		}
	}

	void OpenGLContext::unbindTexture(
		[[maybe_unused]] GLenum target, GLuint texture_id) noexcept {
		// unbind this texture from all the units it might be bound to
		// no need unbind the texture from FBOs because we're not tracking that state (and there is
		// no need to).
		// Never attempt to unbind texture 0. This could happen with external textures w/ streaming if
		// never populated.
		if (texture_id) {

			for (GLuint unit = 0; unit < MAX_TEXTURE_UNIT_COUNT; unit++) {
				if (state.textures.units[unit].id == texture_id) {
					// if this texture is bound, it should be at the same target
					assert(state.textures.units[unit].target == target);
					unbindTextureUnit(unit);
				}
			}
		}
	}

	void OpenGLContext::unbindTextureUnit(GLuint unit) noexcept {
		update_state(state.textures.units[unit].id, 0u, [&]() {
			activeTexture(unit);
			glBindTexture(state.textures.units[unit].target, 0u);
			});
	}

	void OpenGLContext::unbindSampler(GLuint sampler) noexcept {
		// unbind this sampler from all the units it might be bound to
		 // clang generates >800B of code!!!
		for (GLuint unit = 0; unit < MAX_TEXTURE_UNIT_COUNT; unit++) {
			if (state.textures.units[unit].sampler == sampler) {
				bindSampler(unit, 0);
			}
		}
	}

	void OpenGLContext::deleteBuffer(GLuint buffer, GLenum target) noexcept {
		glDeleteBuffers(1, &buffer);

		// bindings of bound buffers are reset to 0
		size_t const targetIndex = getIndexForBufferTarget(target);
		auto& genericBinding = state.buffers.genericBinding[targetIndex];
		if (genericBinding == buffer) {
			genericBinding = 0;
		}

		if ((bugs.rebind_buffer_after_deletion)) {
			if (genericBinding) {
				glBindBuffer(target, genericBinding);
			}
		}

#ifndef FILAMENT_SILENCE_NOT_SUPPORTED_BY_ES2
		assert((target != GL_UNIFORM_BUFFER && target != GL_TRANSFORM_FEEDBACK_BUFFER));

		if (target == GL_UNIFORM_BUFFER || target == GL_TRANSFORM_FEEDBACK_BUFFER) {
			auto& indexedBinding = state.buffers.targets[targetIndex];

			for (auto& entry : indexedBinding.buffers) {
				if (entry.name == buffer) {
					entry.name = 0;
					entry.offset = 0;
					entry.size = 0;
				}
			}
		}
#endif
	}

	void OpenGLContext::deleteVertexArray(GLuint vao) noexcept {
		if ((vao)) {
			procs.deleteVertexArrays(1, &vao);
			// if the destroyed VAO is bound, clear the binding.
			if (state.vao.p->vao[contextIndex] == vao) {
				bindVertexArray(nullptr);
			}
		}
	}

#ifndef FILAMENT_SILENCE_NOT_SUPPORTED_BY_ES2
	GLuint OpenGLContext::getSamplerSlow(SamplerParams params) const noexcept {
		assert(mSamplerMap.find(params) == mSamplerMap.end());

		using namespace GLUtils;

		GLuint s;
		glGenSamplers(1, &s);
		glSamplerParameteri(s, GL_TEXTURE_MIN_FILTER, (GLint)getTextureFilter(params.filterMin));
		glSamplerParameteri(s, GL_TEXTURE_MAG_FILTER, (GLint)getTextureFilter(params.filterMag));
		glSamplerParameteri(s, GL_TEXTURE_WRAP_S, (GLint)getWrapMode(params.wrapS));
		glSamplerParameteri(s, GL_TEXTURE_WRAP_T, (GLint)getWrapMode(params.wrapT));
		glSamplerParameteri(s, GL_TEXTURE_WRAP_R, (GLint)getWrapMode(params.wrapR));
		glSamplerParameteri(s, GL_TEXTURE_COMPARE_MODE, (GLint)getTextureCompareMode(params.compareMode));
		glSamplerParameteri(s, GL_TEXTURE_COMPARE_FUNC, (GLint)getTextureCompareFunc(params.compareFunc));

#if defined(GL_EXT_texture_filter_anisotropic)
		if (ext.EXT_texture_filter_anisotropic &&
			!bugs.texture_filter_anisotropic_broken_on_sampler) {
			GLfloat const anisotropy = float(1u << params.anisotropyLog2);
			glSamplerParameterf(s, GL_TEXTURE_MAX_ANISOTROPY_EXT,
				std::min(gets.max_anisotropy, anisotropy));
		}
#endif
		CHECK_GL_ERROR
			mSamplerMap[params] = s;
		return s;
	}
#endif


	void OpenGLContext::resetState() noexcept {
		// Force GL state to match the Filament state

		// increase the state version so other parts of the state know to reset
		state.age++;

		if (state.major > 2) {
#ifndef FILAMENT_SILENCE_NOT_SUPPORTED_BY_ES2
			glBindFramebuffer(GL_DRAW_FRAMEBUFFER, state.draw_fbo);
			glBindFramebuffer(GL_READ_FRAMEBUFFER, state.read_fbo);
#endif
		}
		else {
			assert(state.read_fbo == state.draw_fbo);
			glBindFramebuffer(GL_FRAMEBUFFER, state.draw_fbo);
			state.read_fbo = state.draw_fbo;
		}


		// state.program
		glUseProgram(state.program.use);

		// state.vao
		state.vao.p = nullptr;
		bindVertexArray(nullptr);

		// state.raster
		glFrontFace(state.raster.frontFace);
		glCullFace(state.raster.cullFace);
		glBlendEquationSeparate(state.raster.blendEquationRGB, state.raster.blendEquationA);
		glBlendFuncSeparate(
			state.raster.blendFunctionSrcRGB,
			state.raster.blendFunctionDstRGB,
			state.raster.blendFunctionSrcA,
			state.raster.blendFunctionDstA
		);
		glColorMask(
			state.raster.colorMask,
			state.raster.colorMask,
			state.raster.colorMask,
			state.raster.colorMask
		);
		glDepthMask(state.raster.depthMask);
		glDepthFunc(state.raster.depthFunc);

		// state.stencil
		glStencilFuncSeparate(
			GL_FRONT,
			state.stencil.front.func.func,
			state.stencil.front.func.ref,
			state.stencil.front.func.mask
		);
		glStencilFuncSeparate(
			GL_BACK,
			state.stencil.back.func.func,
			state.stencil.back.func.ref,
			state.stencil.back.func.mask
		);
		glStencilOpSeparate(
			GL_FRONT,
			state.stencil.front.op.sfail,
			state.stencil.front.op.dpfail,
			state.stencil.front.op.dppass
		);
		glStencilOpSeparate(
			GL_BACK,
			state.stencil.back.op.sfail,
			state.stencil.back.op.dpfail,
			state.stencil.back.op.dppass
		);
		glStencilMaskSeparate(GL_FRONT, state.stencil.front.stencilMask);
		glStencilMaskSeparate(GL_BACK, state.stencil.back.stencilMask);

		// state.polygonOffset
		glPolygonOffset(state.polygonOffset.factor, state.polygonOffset.units);

		// state.enables
		setDefaultState();

		// state.buffers
		// Reset state.buffers to its default state to avoid the complexity and error-prone
		// nature of resetting the GL state to its existing state
		state.buffers = {};

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

		if (state.major > 2) {
#ifndef FILAMENT_SILENCE_NOT_SUPPORTED_BY_ES2
			for (auto const target : {
				GL_UNIFORM_BUFFER,
					GL_TRANSFORM_FEEDBACK_BUFFER,
#if defined(BACKEND_OPENGL_LEVEL_GLES31)
					GL_SHADER_STORAGE_BUFFER,
#endif
					GL_PIXEL_PACK_BUFFER,
					GL_PIXEL_UNPACK_BUFFER,
			}) {
				glBindBuffer(target, 0);
			}

			for (size_t bufferIndex = 0; bufferIndex < MAX_BUFFER_BINDINGS; ++bufferIndex) {
				if (bufferIndex < (size_t)gets.max_uniform_buffer_bindings) {
					glBindBufferBase(GL_UNIFORM_BUFFER, bufferIndex, 0);
				}

				if (bufferIndex < (size_t)gets.max_transform_feedback_separate_attribs) {
					glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, bufferIndex, 0);
				}
			}
#endif
		}

		// state.textures
		// Reset state.textures to its default state to avoid the complexity and error-prone
		// nature of resetting the GL state to its existing state
		state.textures = {};
		const std::pair<GLuint, bool> textureTargets[] = {
				{ GL_TEXTURE_2D,                true },
				{ GL_TEXTURE_2D_ARRAY,          true },
				{ GL_TEXTURE_CUBE_MAP,          true },
				{ GL_TEXTURE_3D,                true },

				{ GL_TEXTURE_2D_MULTISAMPLE,    true },

	#if !defined(__EMSCRIPTEN__)
	#if defined(GL_OES_EGL_image_external)
				{ GL_TEXTURE_EXTERNAL_OES,      ext.OES_EGL_image_external_essl3 },
	#endif

				{ GL_TEXTURE_CUBE_MAP_ARRAY,    ext.EXT_texture_cube_map_array },

	#endif
		};
		for (GLint unit = 0; unit < gets.max_combined_texture_image_units; ++unit) {
			glActiveTexture(GL_TEXTURE0 + unit);
			if (state.major > 2) {
#ifndef FILAMENT_SILENCE_NOT_SUPPORTED_BY_ES2
				glBindSampler(unit, 0);
#endif
			}
			for (auto [target, available] : textureTargets) {
				if (available) {
					glBindTexture(target, 0);
				}
			}
		}
		glActiveTexture(GL_TEXTURE0 + state.textures.active);

		// state.unpack
		glPixelStorei(GL_UNPACK_ALIGNMENT, state.unpack.alignment);
		if (state.major > 2) {
#ifndef FILAMENT_SILENCE_NOT_SUPPORTED_BY_ES2
			glPixelStorei(GL_UNPACK_ROW_LENGTH, state.unpack.row_length);
#endif
		}


		// state.pack
		glPixelStorei(GL_PACK_ALIGNMENT, state.pack.alignment);
		if (state.major > 2) {
#ifndef FILAMENT_SILENCE_NOT_SUPPORTED_BY_ES2
			glPixelStorei(GL_PACK_ROW_LENGTH, 0); // we rely on GL_PACK_ROW_LENGTH being zero
#endif
		}

		// state.window
		//glScissor(
		//	state.window.scissor.x,
		//	state.window.scissor.y,
		//	state.window.scissor.z,
		//	state.window.scissor.w
		//);
		//glViewport(
		//	state.window.viewport.x,
		//	state.window.viewport.y,
		//	state.window.viewport.z,
		//	state.window.viewport.w
		//);
		//glDepthRangef(state.window.depthRange.x, state.window.depthRange.y);
	}

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

} // namesapce filament
