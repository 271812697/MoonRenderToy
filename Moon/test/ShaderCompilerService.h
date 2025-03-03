#pragma once

//#include "gl_headers.h"

//#include "CallbackManager.h"
#include "CompilerThreadPool.h"
//#include "OpenGLBlobCache.h"

//#include <backend/CallbackHandler.h>
#include "Program.h"//<backend/>

#include "utils/CString.h"
#include "utils/FixedCapacityVector.h"//<>
#include "utils/Invocable.h"//<>
//#include <utils/JobSystem.h>

#include <atomic>
#include <condition_variable>
#include <deque>
#include <functional>
#include <memory>
#include <mutex>
#include <thread>
#include <utility>
#include <vector>

namespace TEST {

	class Driver;
	class OpenGLContext;
	//class OpenGLPlatform;
	class Program;
	//class CallbackHandler;

	/*
	 * A class handling shader compilation that supports asynchronous compilation.
	 */
	class ShaderCompilerService {
		struct OpenGLProgramToken;

	public:
		using program_token_t = std::shared_ptr<OpenGLProgramToken>;

		explicit ShaderCompilerService(Driver& driver);

		ShaderCompilerService(ShaderCompilerService const& rhs) = delete;
		ShaderCompilerService(ShaderCompilerService&& rhs) = delete;
		ShaderCompilerService& operator=(ShaderCompilerService const& rhs) = delete;
		ShaderCompilerService& operator=(ShaderCompilerService&& rhs) = delete;

		~ShaderCompilerService() noexcept;

		bool isParallelShaderCompileSupported() const noexcept;

		void init() noexcept;
		void terminate() noexcept;

		// creates a program (compile + link) asynchronously if supported
		program_token_t createProgram(utils::CString const& name, Program&& program);

		// Return the GL program, blocks if necessary. The Token is destroyed and becomes invalid.
		unsigned int getProgram(program_token_t& token);

		// Must be called at least once per frame.
		void tick();

		// Destroys a valid token and all associated resources. Used to "cancel" a program compilation.
		static void terminate(program_token_t& token);

		// stores a user data pointer in the token
		static void setUserData(const program_token_t& token, void* user) noexcept;

		// retrieves the user data pointer stored in the token
		static void* getUserData(const program_token_t& token) noexcept;

		// call the callback when all active programs are ready
		//void notifyWhenAllProgramsAreReady(
		//	CallbackHandler* handler, CallbackHandler::Callback callback, void* user);

	private:
		struct Job {
			template<typename FUNC>
			Job(FUNC&& fn) : fn(std::forward<FUNC>(fn)) {}
			Job(std::function<bool(Job const& job)> fn,
				void* user)
				: fn(std::move(fn)), user(user) {
			}
			std::function<bool(Job const& job)> fn;
			//CallbackHandler* handler = nullptr;
			void* user = nullptr;
			//CallbackHandler::Callback callback{};
		};

		enum class Mode {
			UNDEFINED,      // init() has not been called yet.
			SYNCHRONOUS,    // synchronous shader compilation
			THREAD_POOL,    // asynchronous shader compilation using a thread-pool (most common)
			ASYNCHRONOUS    // asynchronous shader compilation using KHR_parallel_shader_compile
		};

		Driver& mDriver;
		//OpenGLBlobCache mBlobCache;
		//CallbackManager mCallbackManager;
		//CompilerThreadPool mCompilerThreadPool;

		uint32_t mShaderCompilerThreadCount = 0u;
		Mode mMode = Mode::UNDEFINED; // valid after init() is called

		using ContainerType = std::tuple<CompilerPriorityQueue, program_token_t, Job>;
		std::vector<ContainerType> mRunAtNextTickOps;

		unsigned int initialize(ShaderCompilerService::program_token_t& token) noexcept;

		static void getProgramFromCompilerPool(program_token_t& token) noexcept;

		static void compileShaders(
			OpenGLContext& context,
			Program::ShaderSource shadersSource,
			utils::FixedCapacityVector<Program::SpecializationConstant> const& specializationConstants,
			bool multiview,
			std::array<unsigned int, Program::SHADER_TYPE_COUNT>& outShaders,
			std::array<utils::CString, Program::SHADER_TYPE_COUNT>& outShaderSourceCode) noexcept;

		static void process_GOOGLE_cpp_style_line_directive(OpenGLContext& context,
			char* source, size_t len) noexcept;

		static void process_OVR_multiview2(OpenGLContext& context, int32_t eyeCount,
			char* source, size_t len) noexcept;

		static std::string_view process_ARB_shading_language_packing(OpenGLContext& context) noexcept;

		static std::array<std::string_view, 3> splitShaderSource(std::string_view source) noexcept;

		static unsigned int linkProgram(OpenGLContext& context,
			std::array<unsigned int, Program::SHADER_TYPE_COUNT> shaders,
			utils::FixedCapacityVector<std::pair<utils::CString, uint8_t>> const& attributes) noexcept;

		static bool checkProgramStatus(program_token_t const& token) noexcept;

		void runAtNextTick(CompilerPriorityQueue priority,
			const program_token_t& token, Job job) noexcept;
		void executeTickOps() noexcept;
		bool cancelTickOp(program_token_t token) noexcept;
		// order of insertion is important
	};

}