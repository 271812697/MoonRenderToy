#include <spdlog/sinks/stdout_color_sinks.h>
#include "log.h"
#include <stdarg.h>

namespace MOON {
	std::shared_ptr<spdlog::logger> Log::logger;
	Log& Log::intance()
	{
		static Log self;
		return self;
	}
	bool Log::addOutput(LogOutput* pLog)
	{
		for (size_t i = 0; i < logArr.size(); i++)
		{
			if (logArr[i] == pLog)
				return false;
		}

		logArr.push_back(pLog);

		return true;
	}
	void Log::logMessage(LogOutput::Level level, const QString& msg)
	{
		if (LogOutput::LL_INVALID != level)
		{
			for (LogOutput* output : logArr)
			{
				output->logMessage(level, msg.toUtf8().constData());
			}
		}
	}
	void Log::logMessage(LogOutput::Level level, const char* msg)
	{
		if (LogOutput::LL_INVALID != level)
		{
			for (LogOutput* output : logArr)
			{
				output->logMessage(level, msg);
			}
		}
	}
	void Log::logMessage(LogOutput::Level level, const std::wstring& message)
	{
		if (LogOutput::LL_INVALID != level)
		{
			std::string msg(message.begin(), message.end());
			for (LogOutput* output : logArr)
			{
				output->logMessage(level, msg);
			}
		}
	}
	void Log::logMessageExt(LogOutput::Level level, const char* formats, ...)
	{
		if (LogOutput::LL_INVALID != level)
		{
			const int bufferLength = 8192;
			char szBuffer[bufferLength] = {};
			int numforwrite = 0;
			va_list args;
			va_start(args, formats);
			numforwrite = _vsnprintf(szBuffer, bufferLength, formats, args);
			va_end(args);
			szBuffer[bufferLength - 1] = 0;
			std::string msg = szBuffer;
			for (LogOutput* output : logArr)
			{
				output->logMessage(level, msg);
			}
		}
	}
	void Log::logMessageExt(LogOutput::Level level, const QString& msg)
	{
		if (LogOutput::LL_INVALID != level)
		{
			for (LogOutput* output : logArr)
			{
				output->logMessage(level, msg.toUtf8().constData());
			}
		}
	}
	void Log::Init() {
		using wincolor_sink_ptr = std::shared_ptr<spdlog::sinks::stdout_color_sink_mt>;

		std::vector<wincolor_sink_ptr> sinks;  // pointers to sinks that support setting custom color
		sinks.emplace_back(std::make_shared<spdlog::sinks::stdout_color_sink_mt>());  // console sink
		sinks[0]->set_pattern("%^%T > [%L] %v%$");  // e.g. 23:55:59 > [I] sample message


		logger = std::make_shared<spdlog::logger>("MOON", begin(sinks), end(sinks));
		spdlog::register_logger(logger);
		logger->set_level(spdlog::level::trace);  // log level less than this will be silently ignored
		logger->flush_on(spdlog::level::trace);   // the minimum log level that will trigger automatic flush
	}

	void Log::Shutdown() {
		spdlog::shutdown();
	}

}