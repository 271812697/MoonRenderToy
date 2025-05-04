
#pragma once

#include <string>
#include <map>

#include <Tools/Eventing/Event.h>//"../../tools/Event.h"

#include "Debug/ILogHandler.h"
#include "Debug/ConsoleHandler.h"
#include "Debug/FileHandler.h"
#include "Debug/HistoryHandler.h"

#define OVLOG(message)			Debug::Logger::Log(message, Debug::ELogLevel::LOG_DEFAULT,	Debug::ELogMode::CONSOLE)
#define OVLOG_INFO(message)		Debug::Logger::Log(message, Debug::ELogLevel::LOG_INFO,		Debug::ELogMode::CONSOLE)
#define OVLOG_WARNING(message)	Debug::Logger::Log(message, Debug::ELogLevel::LOG_WARNING,	Debug::ELogMode::CONSOLE)
#define OVLOG_ERROR(message)	Debug::Logger::Log(message, Debug::ELogLevel::LOG_ERROR,	Debug::ELogMode::CONSOLE)

#define OVFLOG(message)			Debug::Logger::Log(message, Debug::ELogLevel::LOG_DEFAULT, 	Debug::ELogMode::CONSOLE); Debug::Logger::Log(message, Debug::ELogLevel::LOG_DEFAULT, Debug::ELogMode::FILE)
#define OVFLOG_INFO(message)	Debug::Logger::Log(message, Debug::ELogLevel::LOG_INFO, 	Debug::ELogMode::CONSOLE); Debug::Logger::Log(message, Debug::ELogLevel::LOG_INFO, 	Debug::ELogMode::FILE)
#define OVFLOG_WARNING(message)	Debug::Logger::Log(message, Debug::ELogLevel::LOG_WARNING, 	Debug::ELogMode::CONSOLE); Debug::Logger::Log(message, Debug::ELogLevel::LOG_WARNING, Debug::ELogMode::FILE)
#define OVFLOG_ERROR(message)	Debug::Logger::Log(message, Debug::ELogLevel::LOG_ERROR, 	Debug::ELogMode::CONSOLE); Debug::Logger::Log(message, Debug::ELogLevel::LOG_ERROR, 	Debug::ELogMode::FILE)

namespace Debug
{
	/*
	* Static class to display error messages on console or file
	*/
	class Logger
	{
	public:

		/**
		* Disabled constructor
		*/
		Logger() = delete;

		/**
		* Display error massage to target destination
		* @param p_message
		* @param p_logLevel
		* @param p_logMode
		* @param p_handlerID
		*/
		static void Log(const std::string& p_message, ELogLevel p_logLevel = ELogLevel::LOG_DEFAULT, ELogMode p_logMode = ELogMode::DEFAULT, std::string p_handlerId = "default");

		/**
		* Create console handler
		* @param p_id
		*/
		static ConsoleHandler& CreateConsoleHandler(std::string p_id);

		/**
		* Create console handler
		* @param p_id
		*/
		static FileHandler& CreateFileHandler(std::string p_id);

		/**
		* Create console handler
		* @param p_id
		*/
		static HistoryHandler& CreateHistoryHandler(std::string p_id);

		/**
		* Return target console handler
		* @param p_id
		*/
		static ConsoleHandler& GetConsoleHandler(std::string p_id);

		/**
		* Return target file handler
		* @param p_id
		*/
		static FileHandler& GetFileHandler(std::string p_id);

		/**
		* Return target history handler
		* @param p_id
		*/
		static HistoryHandler& GetHistoryHandler(std::string p_id);

	private:
		template<typename T>
		static void LogToHandlerMap(std::map<std::string, T>& p_map, const LogData& p_data, std::string p_id);

	public:
		static Tools::Eventing::Event<const LogData&> LogEvent;

	private:
		static std::map<std::string, ConsoleHandler>	CONSOLE_HANDLER_MAP;
		static std::map<std::string, FileHandler>		FILE_HANDLER_MAP;
		static std::map<std::string, HistoryHandler>	HISTORY_HANDLER_MAP;
	};
}

#include "Debug/Logger.inl"