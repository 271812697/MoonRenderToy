#pragma once
#include "logOutput.h"
#include <spdlog/spdlog.h>
#include <QString>

#define CORE_INFO(...)  MOON::Log::GetLogger()->info(__VA_ARGS__);

#define LOG_INFO(...) MOON::Log::intance().logMessageExt(MOON::LogOutput::LL_INFO, __VA_ARGS__);

#define CORE_WARN(...)  MOON::Log::GetLogger()->warn(__VA_ARGS__);

#define LOG_WARN(...) MOON::Log::intance().logMessageExt(MOON::LogOutput::LL_WARNING, __VA_ARGS__);

#define CORE_DEBUG(...) MOON::Log::GetLogger()->debug(__VA_ARGS__);

#define LOG_DEBUG(...) MOON::Log::intance().logMessageExt(MOON::LogOutput::LL_DEBUG, __VA_ARGS__);

#define CORE_TRACE(...) MOON::Log::GetLogger()->trace(__VA_ARGS__);

#define LOG_TRACE(...) MOON::Log::intance().logMessageExt(MOON::LogOutput::LL_DEBUG, __VA_ARGS__);

#define CORE_ERROR(...) MOON::Log::GetLogger()->error(__VA_ARGS__);

#define LOG_ERROR(...) MOON::Log::intance().logMessageExt(MOON::LogOutput::LL_ERROR, __VA_ARGS__);


#if defined(_DEBUG) || defined(DEBUG)
#define CORE_ASERT(cond, ...) { if (!(cond)) { MOON::Log::GetLogger()->critical(__VA_ARGS__); __debugbreak(); } }
#else
#define CORE_ASERT(cond, ...)
#endif

namespace MOON {

	class Log {
	private:
		static std::shared_ptr<spdlog::logger> logger;
		std::vector<LogOutput*>logArr;

	public:
		static Log& intance();
		static std::shared_ptr<spdlog::logger> GetLogger() { return logger; }
		// add log
		bool addOutput(LogOutput* pLog);
		void logMessage(LogOutput::Level level, const QString& msg);
		void logMessage(LogOutput::Level level, const char* msg);
		void logMessage(LogOutput::Level level, const std::wstring& message);
		void logMessageExt(LogOutput::Level level, const char* formats, ...);
		void logMessageExt(LogOutput::Level level, const QString& msg);
	public:
		static void Init();
		static void Shutdown();

	};

}
