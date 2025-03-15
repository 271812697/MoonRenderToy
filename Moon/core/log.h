#pragma once
#include <spdlog/spdlog.h>

#define CORE_INFO(...)  MOON::Log::GetLogger()->info(__VA_ARGS__)
#define CORE_WARN(...)  MOON::Log::GetLogger()->warn(__VA_ARGS__)
#define CORE_DEBUG(...) MOON::Log::GetLogger()->debug(__VA_ARGS__)
#define CORE_TRACE(...) MOON::Log::GetLogger()->trace(__VA_ARGS__)
#define CORE_ERROR(...) MOON::Log::GetLogger()->error(__VA_ARGS__)


#if defined(_DEBUG) || defined(DEBUG)
#define CORE_ASERT(cond, ...) { if (!(cond)) { MOON::Log::GetLogger()->critical(__VA_ARGS__); __debugbreak(); } }
#else
#define CORE_ASERT(cond, ...)
#endif

namespace MOON {

	class Log {
	private:
		static std::shared_ptr<spdlog::logger> logger;

	public:
		static std::shared_ptr<spdlog::logger> GetLogger() { return logger; }

	public:
		static void Init();
		static void Shutdown();
	};

}
