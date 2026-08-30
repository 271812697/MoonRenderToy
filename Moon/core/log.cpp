#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/base_sink.h>
#include "log.h"
#include <stdarg.h>
#include <algorithm>

namespace MOON {
	namespace {
		// Forwards every spdlog message to the registered LogOutputs (e.g. the
		// LogPanel), so CORE_* macros show up in the editor log panel too.
		class PanelLogSink final : public spdlog::sinks::base_sink<std::mutex> {
		protected:
			void sink_it_(const spdlog::details::log_msg& msg) override {
				auto level = LogOutput::LL_INFO;
				switch (msg.level) {
					case spdlog::level::trace:
					case spdlog::level::debug:
						level = LogOutput::LL_DEBUG;
						break;
					case spdlog::level::info:
						level = LogOutput::LL_INFO;
						break;
					case spdlog::level::warn:
						level = LogOutput::LL_WARNING;
						break;
					case spdlog::level::err:
					case spdlog::level::critical:
						level = LogOutput::LL_ERROR;
						break;
					default:
						break;
				}
				const std::string payload(msg.payload.data(), msg.payload.size());
				Log::intance().logMessage(level, payload.c_str());
			}
			void flush_() override {
			}
		};
	}

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
	void Log::removeOutput(LogOutput* pLog)
	{
		logArr.erase(std::remove(logArr.begin(), logArr.end(), pLog), logArr.end());
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
		auto consoleSink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
		consoleSink->set_pattern("%^%T > [%L] %v%$");  // e.g. 23:55:59 > [I] sample message

		logger = std::make_shared<spdlog::logger>(
			"MOON", spdlog::sinks_init_list{ consoleSink, std::make_shared<PanelLogSink>() });
		spdlog::register_logger(logger);
		logger->set_level(spdlog::level::trace);  // log level less than this will be silently ignored
		logger->flush_on(spdlog::level::trace);   // the minimum log level that will trigger automatic flush
	}

	void Log::Shutdown() {
		spdlog::shutdown();
	}

}
