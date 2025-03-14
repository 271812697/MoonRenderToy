#include<glad/glad.h>
#include "ShaderCompilerService.h"
#include "Driver.h"
#include "Program.h"
#include "utils/CString.h"
#include "utils/FixedCapacityVector.h"
#include "core/log.h"
#include <array>
#include <cctype>
#include <chrono>
#include <mutex>
#include <memory>
#include <string>
#include <string_view>
#include <thread>
#include <utility>
#include <variant>

#include <stddef.h>
#include <stdint.h>
#include <iostream>

namespace TEST {

	using namespace utils;

	// ------------------------------------------------------------------------------------------------

	static void logCompilationError(
		ShaderStage shaderType, const char* name,
		unsigned int shaderId, CString const& sourceCode) noexcept;

	static void logProgramLinkError(
		const char* name, unsigned int program) noexcept;

	static inline std::string to_string(bool b) noexcept {
		return b ? "true" : "false";
	}

	static inline std::string to_string(int i) noexcept {
		return std::to_string(i);
	}

	static inline std::string to_string(float f) noexcept {
		return "float(" + std::to_string(f) + ")";
	}

	// ------------------------------------------------------------------------------------------------

	struct ShaderCompilerService::OpenGLProgramToken : ProgramToken {
		struct ProgramData {
			unsigned int program{};
			std::array<unsigned int, Program::SHADER_TYPE_COUNT> shaders{};
		};

		~OpenGLProgramToken() override;

		OpenGLProgramToken(ShaderCompilerService& compiler, utils::CString const& name) noexcept
			: compiler(compiler), name(name) {
		}

		ShaderCompilerService& compiler;
		utils::CString const& name;
		utils::FixedCapacityVector<std::pair<utils::CString, uint8_t>> attributes;
		std::array<utils::CString, Program::SHADER_TYPE_COUNT> shaderSourceCode;
		void* user = nullptr;
		struct {
			std::array<unsigned int, Program::SHADER_TYPE_COUNT> shaders{};
			unsigned int program = 0;
		} gl; // 12 bytes


		// Sets the programData, typically from the compiler thread, and signal the main thread.
		// This is similar to std::promise::set_value.
		void set(ProgramData const& data) noexcept {
			std::unique_lock const l(lock);
			programData = data;
			signaled = true;
			cond.notify_one();
		}

		// Get the programBinary, wait if necessary.
		// This is similar to std::future::get
		ProgramData const& get() const noexcept {
			std::unique_lock l(lock);
			cond.wait(l, [this]() { return signaled; });
			return programData;
		}

		void wait() const noexcept {
			std::unique_lock l(lock);
			cond.wait(l, [this]() { return signaled; });
		}

		// Checks if the programBinary is ready.
		// This is similar to std::future::wait_for(0s)
		bool isReady() const noexcept {
			std::unique_lock l(lock);
			using namespace std::chrono_literals;
			return cond.wait_for(l, 0s, [this]() { return signaled; });
		}

		//CallbackManager::Handle handle{};
		//BlobCacheKey key;
		mutable std::mutex lock;
		mutable utils::Condition cond;
		ProgramData programData;
		bool signaled = false;

		bool canceled = false; // not part of the signaling
	};

	ShaderCompilerService::OpenGLProgramToken::~OpenGLProgramToken() = default;

	void ShaderCompilerService::setUserData(const program_token_t& token, void* user) noexcept {
		token->user = user;
	}

	void* ShaderCompilerService::getUserData(const program_token_t& token) noexcept {
		return token->user;
	}

	// ------------------------------------------------------------------------------------------------

	ShaderCompilerService::ShaderCompilerService(Driver& driver)
		: mDriver(driver)//,
		//mBlobCache(driver.getContext()),
		//mCallbackManager(driver) 
	{
	}

	ShaderCompilerService::~ShaderCompilerService() noexcept = default;

	bool ShaderCompilerService::isParallelShaderCompileSupported() const noexcept {
		assert(mMode != Mode::UNDEFINED);
		return mMode != Mode::SYNCHRONOUS;
	}

	void ShaderCompilerService::init() noexcept {
		if ((true)) {
			// user disabled parallel shader compile
			mMode = Mode::SYNCHRONOUS;
			return;
		}



		if (mMode == Mode::THREAD_POOL) {
			//to do:
		}
	}

	void ShaderCompilerService::terminate() noexcept {
		// Finally stop the thread pool immediately. Pending jobs will be discarded. We guarantee by
		// construction that nobody is waiting on a token (because waiting is only done on the main
		// backend thread, and if we're here, we're on the backend main thread).
		//mCompilerThreadPool.terminate();

		mRunAtNextTickOps.clear();

		// We could have some pending callbacks here, we need to execute them.
		// This is equivalent to calling cancelTickOp() on all active tokens.
		//mCallbackManager.terminate();
	}

	ShaderCompilerService::program_token_t ShaderCompilerService::createProgram(
		utils::CString const& name, Program&& program) {
		auto& gl = mDriver.getContext();
		auto token = std::make_shared<OpenGLProgramToken>(*this, name);
		CompilerPriorityQueue const priorityQueue = program.getPriorityQueue();
		if (mMode == Mode::THREAD_POOL) {
			//to do:
		}
		else {
			// this cannot fail because we check compilation status after linking the program
			// shaders[] is filled with id of shader stages present.
			compileShaders(gl,
				std::move(program.getShadersSource()),
				program.getSpecializationConstants(),
				program.isMultiview(),
				token->gl.shaders,
				token->shaderSourceCode);

			runAtNextTick(priorityQueue, token, [this, token](Job const&) {
				assert(mMode != Mode::THREAD_POOL);
				if (mMode == Mode::ASYNCHRONOUS) {
					// don't attempt to link this program if all shaders are not done compiling
					int status;
					if (token->gl.program) {
						glGetProgramiv(token->gl.program, GL_COMPLETION_STATUS_ARB, &status);
						if (status == GL_FALSE) {
							return false;
						}
					}
					else {
						for (auto shader : token->gl.shaders) {
							if (shader) {
								glGetShaderiv(shader, GL_COMPLETION_STATUS_ARB, &status);
								if (status == GL_FALSE) {
									return false;
								}
							}
						}
					}
				}

				if (!token->gl.program) {
					// link the program, this also cannot fail because status is checked later.
					token->gl.program = linkProgram(mDriver.getContext(),
						token->gl.shaders, token->attributes);
					if (mMode == Mode::ASYNCHRONOUS) {
						// wait until the link finishes...
						return false;
					}
				}
				assert(token->gl.program);
				return true;
				});
		}

		return token;
	}

	unsigned int ShaderCompilerService::getProgram(ShaderCompilerService::program_token_t& token) {
		unsigned int const program = initialize(token);
		assert(token == nullptr);
#if !FILAMENT_ENABLE_MATDBG
		assert(program);
#endif
		return program;
	}

	void ShaderCompilerService::terminate(program_token_t& token) {
		assert(token);

		token->canceled = true;

		bool const canceled = token->compiler.cancelTickOp(token);

		if (token->compiler.mMode == Mode::THREAD_POOL) {
			//to do:
		}
		else if (canceled) {
			// Since the tick op was canceled, we need to .put the token here.
			//token->compiler.mCallbackManager.put(token->handle);
		}

		for (unsigned int& shader : token->gl.shaders) {
			if (shader) {
				if (token->gl.program) {
					glDetachShader(token->gl.program, shader);
				}
				glDeleteShader(shader);
				shader = 0;
			}
		}
		if (token->gl.program) {
			glDeleteProgram(token->gl.program);
		}

		token.reset();
	}

	void ShaderCompilerService::tick() {
		// we don't need to run executeTickOps() if we're using the thread-pool
		if ((mMode != Mode::THREAD_POOL)) {
			executeTickOps();
		}
	}

	//void ShaderCompilerService::notifyWhenAllProgramsAreReady(
	//	CallbackHandler* handler, CallbackHandler::Callback callback, void* user) {
	//	if (callback) {
	//		mCallbackManager.setCallback(handler, callback, user);
	//	}
	//}

	// ------------------------------------------------------------------------------------------------

	void ShaderCompilerService::getProgramFromCompilerPool(program_token_t& token) noexcept {
		OpenGLProgramToken::ProgramData const& programData{ token->get() };
		if (!token->canceled) {
			token->gl.shaders = programData.shaders;
			token->gl.program = programData.program;
		}
	}

	unsigned int ShaderCompilerService::initialize(program_token_t& token) noexcept {

		if (!token->gl.program) {
			if (mMode == Mode::THREAD_POOL) {
				//to do:
			}
			else if (mMode == Mode::ASYNCHRONOUS) {
				// we force the program link -- which might stall, either here or below in
				// checkProgramStatus(), but we don't have a choice, we need to use the program now.
				token->compiler.cancelTickOp(token);

				token->gl.program = linkProgram(mDriver.getContext(),
					token->gl.shaders, token->attributes);

				assert(token->gl.program);

				//mCallbackManager.put(token->handle);

				//if (token->key) {
					//mBlobCache.insert(mDriver.mPlatform, token->key, token->gl.program);
				//}
			}
			else {
				// if we don't have a program yet, block until we get it.
				tick();
			}
		}

		// by this point we must have a GL program
		assert(token->gl.program);

		unsigned int program = 0;

		// check status of program linking and shader compilation, logs error and free all resources
		// in case of error.
		bool const success = checkProgramStatus(token);

		// Unless we have matdbg, we panic if a program is invalid. Otherwise, we'd get a UB.
		// The compilation error has been logged to log.e by this point.
		//FILAMENT_CHECK_POSTCONDITION(FILAMENT_ENABLE_MATDBG || success)
		  //      << "OpenGL program " << token->name.c_str_safe() << " failed to link or compile";

		if ((success)) {
			program = token->gl.program;
			// no need to keep the shaders around

			for (unsigned int& shader : token->gl.shaders) {
				if (shader) {
					glDetachShader(program, shader);
					glDeleteShader(shader);
					shader = 0;
				}
			}
		}

		// and destroy all temporary init data
		token = nullptr;

		return program;
	}


	/*
	 * Compile shaders in the ShaderSource. This cannot fail because compilation failures are not
	 * checked until after the program is linked.
	 * This always returns the GL shader IDs or zero a shader stage is not present.
	 */
	void ShaderCompilerService::compileShaders(OpenGLContext& context,
		Program::ShaderSource shadersSource,
		utils::FixedCapacityVector<Program::SpecializationConstant> const& specializationConstants,
		bool multiview,
		std::array<unsigned int, Program::SHADER_TYPE_COUNT>& outShaders,
		std::array<CString, Program::SHADER_TYPE_COUNT>& outShaderSourceCode) noexcept {
		std::string specializationConstantString;
		int32_t numViews = 2;
		// build all shaders
		for (size_t i = 0; i < Program::SHADER_TYPE_COUNT; i++) {
			const ShaderStage stage = static_cast<ShaderStage>(i);
			GLenum glShaderType{};
			switch (stage) {
			case ShaderStage::VERTEX:
				glShaderType = GL_VERTEX_SHADER;
				CORE_INFO("Begin to compile:ShaderStage::VERTEX");
				break;
			case ShaderStage::FRAGMENT:
				glShaderType = GL_FRAGMENT_SHADER;
				CORE_INFO("Begin to compile:ShaderStage::FRAGMENT");
				break;
			case ShaderStage::COMPUTE:
				glShaderType = GL_COMPUTE_SHADER;
				CORE_INFO("Begin to compile:ShaderStage::COMPUTE");
				break;
			}

			if ((!shadersSource[i].empty())) {
				Program::ShaderBlob& shader = shadersSource[i];
				char* shader_src = reinterpret_cast<char*>(shader.data());
				size_t shader_len = shader.size();
				auto [version, prolog, body] = splitShaderSource({ shader_src, shader_len });
				std::array<std::string_view, 4> sources = {
					version,
					prolog,
					specializationConstantString,
					{ body.data(), body.size() - 1 }  // null-terminated
				};

				auto partitionPoint = std::stable_partition(
					sources.begin(), sources.end(), [](std::string_view s) { return !s.empty(); });
				size_t count = std::distance(sources.begin(), partitionPoint);

				std::array<const char*, 4> shaderStrings;
				std::array<int, 4> lengths;
				for (size_t i = 0; i < count; i++) {
					shaderStrings[i] = sources[i].data();
					lengths[i] = sources[i].size();
				}

				unsigned int const shaderId = glCreateShader(glShaderType);
				glShaderSource(shaderId, count, shaderStrings.data(), lengths.data());
				glCompileShader(shaderId);
				int status;
				glGetShaderiv(shaderId, GL_COMPILE_STATUS, &status);
				if (status == GL_FALSE) {
					int info_log_length;
					glGetShaderiv(shaderId, GL_INFO_LOG_LENGTH, &info_log_length);
					GLchar* info_log = new GLchar[info_log_length + 1];
					glGetShaderInfoLog(shaderId, info_log_length, NULL, info_log);
					CORE_ERROR("Failed to compile shader: {0}", info_log);
					CORE_WARN("The source is:\n{0}", shader_src);
					delete[] info_log;
					glDeleteShader(shaderId);  // prevent shader leak
				}

				outShaderSourceCode[i] = { shader_src, shader_len };
				outShaders[i] = shaderId;
			}
		}
	}

	// If usages of the Google-style line directive are present, remove them, as some
	// drivers don't allow the quotation marks. This source modification happens in-place.
	void ShaderCompilerService::process_GOOGLE_cpp_style_line_directive(OpenGLContext& context,
		char* source, size_t len) noexcept {
		if (!context.ext.GOOGLE_cpp_style_line_directive) {
			//if ((requestsGoogleLineDirectivesExtension({ source, len }))) {
			//	removeGoogleLineDirectives(source, len); // length is unaffected
			//}
		}
	}

	// Look up the `source` to replace the number of eyes for multiview with the given number. This is
	// necessary for OpenGL because OpenGL relies on the number specified in shader files to determine
	// the number of views, which is assumed as a single digit, for multiview.
	// This source modification happens in-place.
	void ShaderCompilerService::process_OVR_multiview2(OpenGLContext& context,
		int32_t eyeCount, char* source, size_t len) noexcept {
		// We don't use regular expression in favor of performance.
		if (context.ext.OVR_multiview2) {
			const std::string_view shader{ source, len };
			const std::string_view layout = "layout";
			const std::string_view num_views = "num_views";
			size_t found = 0;
			while (true) {
				found = shader.find(layout, found);
				if (found == std::string_view::npos) {
					break;
				}
				found = shader.find_first_not_of(' ', found + layout.size());
				if (found == std::string_view::npos || shader[found] != '(') {
					continue;
				}
				found = shader.find_first_not_of(' ', found + 1);
				if (found == std::string_view::npos) {
					continue;
				}
				if (shader.compare(found, num_views.size(), num_views) != 0) {
					continue;
				}
				found = shader.find_first_not_of(' ', found + num_views.size());
				if (found == std::string_view::npos || shader[found] != '=') {
					continue;
				}
				found = shader.find_first_not_of(' ', found + 1);
				if (found == std::string_view::npos) {
					continue;
				}
				// We assume the value should be one-digit number.
				assert(eyeCount < 10);
				assert(!::isdigit(source[found + 1]));
				source[found] = '0' + eyeCount;
				break;
			}
		}
	}

	// Tragically, OpenGL 4.1 doesn't support unpackHalf2x16 (appeared in 4.2) and
	// macOS doesn't support GL_ARB_shading_language_packing
	// Also GLES3.0 didn't have the full set of packing/unpacking functions
	std::string_view ShaderCompilerService::process_ARB_shading_language_packing(OpenGLContext& context) noexcept {
		using namespace std::literals;

		return ""sv;
	}

	// split shader source code in three:
	// - the version line
	// - extensions
	// - everything else
	std::array<std::string_view, 3> ShaderCompilerService::splitShaderSource(std::string_view source) noexcept {
		auto version_start = source.find("#version");
		assert(version_start != std::string_view::npos);

		auto version_eol = source.find('\n', version_start) + 1;
		assert(version_eol != std::string_view::npos);

		auto prolog_start = version_eol;
		auto prolog_eol = source.rfind("\n#extension"); // last #extension line
		if (prolog_eol == std::string_view::npos) {
			prolog_eol = prolog_start;
		}
		else {
			prolog_eol = source.find('\n', prolog_eol + 1) + 1;
		}
		auto body_start = prolog_eol;

		std::string_view const version = source.substr(version_start, version_eol - version_start);
		std::string_view const prolog = source.substr(prolog_start, prolog_eol - prolog_start);
		std::string_view const body = source.substr(body_start, source.length() - body_start);
		return { version, prolog, body };
	}

	/*
	 * Create a program from the given shader IDs and links it. This cannot fail because errors
	 * are checked later. This always returns a valid GL program ID (which doesn't mean the
	 * program itself is valid).
	 */
	unsigned int ShaderCompilerService::linkProgram(OpenGLContext& context,
		std::array<unsigned int, Program::SHADER_TYPE_COUNT> shaders,
		utils::FixedCapacityVector<std::pair<utils::CString, uint8_t>> const& attributes) noexcept {



		unsigned int const program = glCreateProgram();
		for (auto shader : shaders) {
			if (shader) {
				glAttachShader(program, shader);
			}
		}

		//if (UTILS_UNLIKELY(context.isES2())) {
		//	for (auto const& [name, loc] : attributes) {
		//		glBindAttribLocation(program, loc, name.c_str());
		//	}
		//}

		glLinkProgram(program);

		return program;
	}

	// ------------------------------------------------------------------------------------------------

	void ShaderCompilerService::runAtNextTick(CompilerPriorityQueue priority,
		const program_token_t& token, Job job) noexcept {
		// insert items in order of priority and at the end of the range
		auto& ops = mRunAtNextTickOps;
		auto const pos = std::lower_bound(ops.begin(), ops.end(), priority,
			[](ContainerType const& lhs, CompilerPriorityQueue priorityQueue) {
				return std::get<0>(lhs) < priorityQueue;
			});
		ops.emplace(pos, priority, token, std::move(job));

		//SYSTRACE_CONTEXT();
		//SYSTRACE_VALUE32("ShaderCompilerService Jobs", mRunAtNextTickOps.size());
	}

	bool ShaderCompilerService::cancelTickOp(program_token_t token) noexcept {
		// We do a linear search here, but this is rare, and we know the list is pretty small.
		auto& ops = mRunAtNextTickOps;
		auto pos = std::find_if(ops.begin(), ops.end(), [&](const auto& item) {
			return std::get<1>(item) == token;
			});
		if (pos != ops.end()) {
			ops.erase(pos);
			return true;
		}
		//SYSTRACE_CONTEXT();
		//SYSTRACE_VALUE32("ShaderCompilerService Jobs", ops.size());
		return false;
	}

	void ShaderCompilerService::executeTickOps() noexcept {
		auto& ops = mRunAtNextTickOps;
		auto it = ops.begin();
		while (it != ops.end()) {
			Job const& job = std::get<2>(*it);
			bool const remove = job.fn(job);
			if (remove) {
				it = ops.erase(it);
			}
			else {
				++it;
			}
		}
		//SYSTRACE_CONTEXT();
		//SYSTRACE_VALUE32("ShaderCompilerService Jobs", ops.size());
	}

	// ------------------------------------------------------------------------------------------------

	/*
	 * Checks a program link status and logs errors and frees resources on failure.
	 * Returns true on success.
	 */
	bool ShaderCompilerService::checkProgramStatus(program_token_t const& token) noexcept {

		//SYSTRACE_CALL();

		assert(token->gl.program);

		int status;
		glGetProgramiv(token->gl.program, GL_LINK_STATUS, &status);
		if ((status == GL_TRUE)) {
			return true;
		}

		// only if the link fails, we check the compilation status

		for (size_t i = 0; i < Program::SHADER_TYPE_COUNT; i++) {
			const ShaderStage type = static_cast<ShaderStage>(i);
			const unsigned int shader = token->gl.shaders[i];
			if (shader) {
				glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
				if (status != GL_TRUE) {
					logCompilationError(type,
						token->name.c_str_safe(), shader, token->shaderSourceCode[i]);
				}
				glDetachShader(token->gl.program, shader);
				glDeleteShader(shader);
				token->gl.shaders[i] = 0;
			}
		}
		// log the link error as well
		logProgramLinkError(token->name.c_str_safe(), token->gl.program);
		glDeleteProgram(token->gl.program);
		token->gl.program = 0;
		return false;
	}


	void logCompilationError(ShaderStage shaderType,
		const char* name, unsigned int shaderId,
		CString const& sourceCode) noexcept {

		auto to_string = [](ShaderStage type) -> const char* {
			switch (type) {
			case ShaderStage::VERTEX:   return "vertex";
			case ShaderStage::FRAGMENT: return "fragment";
			case ShaderStage::COMPUTE:  return "compute";
			}
			};

		{ // scope for the temporary string storage
			int length = 0;
			glGetShaderiv(shaderId, GL_INFO_LOG_LENGTH, &length);

			CString infoLog(length);
			glGetShaderInfoLog(shaderId, length, nullptr, infoLog.data());

			std::cout << "Compilation error in " << to_string(shaderType) << " shader \"" << name << "\":\n"
				<< "\"" << infoLog.c_str() << "\""
				<< std::endl;;
		}

#ifndef NDEBUG
		std::string_view const shader{ sourceCode.data(), sourceCode.size() };
		size_t lc = 1;
		size_t start = 0;
		std::string line;
		while (true) {
			size_t const end = shader.find('\n', start);
			if (end == std::string::npos) {
				line = shader.substr(start);
			}
			else {
				line = shader.substr(start, end - start);
			}
			std::cout << lc++ << ":   " << line.c_str() << '\n';
			if (end == std::string::npos) {
				break;
			}
			start = end + 1;
		}
		std::cout << std::endl;
#endif
	}

	void logProgramLinkError(char const* name, unsigned int program) noexcept {
		int length = 0;
		glGetProgramiv(program, GL_INFO_LOG_LENGTH, &length);

		CString infoLog(length);
		glGetProgramInfoLog(program, length, nullptr, infoLog.data());

		std::cout << "Link error in \"" << name << "\":\n"
			<< "\"" << infoLog.c_str() << "\""
			<< std::endl;
	}


} // namespace filament::backend
