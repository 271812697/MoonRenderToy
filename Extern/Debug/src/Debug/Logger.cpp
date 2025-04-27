

#include "Debug/Logger.h"
#include "../../tools/Date.h"

Tools::Eventing::Event<const Debug::LogData&> Debug::Logger::LogEvent;

std::map<std::string, Debug::ConsoleHandler>	Debug::Logger::CONSOLE_HANDLER_MAP;
std::map<std::string, Debug::FileHandler>		Debug::Logger::FILE_HANDLER_MAP;
std::map<std::string, Debug::HistoryHandler>	Debug::Logger::HISTORY_HANDLER_MAP;

void Debug::Logger::Log(const std::string& p_message, ELogLevel p_logLevel, ELogMode p_logMode, std::string p_handlerId)
{
	LogData logData{ p_message, p_logLevel, Tools::Time::Date::GetDateAsString() };

	switch (p_logMode)
	{
	case ELogMode::DEFAULT:
	case ELogMode::CONSOLE: LogToHandlerMap<ConsoleHandler>(CONSOLE_HANDLER_MAP, logData, p_handlerId); break;
	case ELogMode::FILE:	LogToHandlerMap<FileHandler>(FILE_HANDLER_MAP, logData, p_handlerId);		break;
	case ELogMode::HISTORY: LogToHandlerMap<HistoryHandler>(HISTORY_HANDLER_MAP, logData, p_handlerId);	break;
	case ELogMode::ALL:
		LogToHandlerMap<ConsoleHandler>(CONSOLE_HANDLER_MAP, logData, p_handlerId);
		LogToHandlerMap<FileHandler>(FILE_HANDLER_MAP, logData, p_handlerId);
		LogToHandlerMap<HistoryHandler>(HISTORY_HANDLER_MAP, logData, p_handlerId);
		break;
	}

	LogEvent.Invoke(logData);
}

Debug::ConsoleHandler& Debug::Logger::CreateConsoleHandler(std::string p_id)
{
	CONSOLE_HANDLER_MAP.emplace(p_id, Debug::ConsoleHandler());
	return CONSOLE_HANDLER_MAP[p_id];
}

Debug::FileHandler& Debug::Logger::CreateFileHandler(std::string p_id)
{
	FILE_HANDLER_MAP.emplace(p_id, Debug::FileHandler());
	return FILE_HANDLER_MAP[p_id];
}

Debug::HistoryHandler& Debug::Logger::CreateHistoryHandler(std::string p_id)
{
	HISTORY_HANDLER_MAP.emplace(p_id, Debug::HistoryHandler());
	return HISTORY_HANDLER_MAP[p_id];
}

Debug::ConsoleHandler& Debug::Logger::GetConsoleHandler(std::string p_id)
{
	return CONSOLE_HANDLER_MAP[p_id];
}

Debug::FileHandler& Debug::Logger::GetFileHandler(std::string p_id)
{
	return FILE_HANDLER_MAP[p_id];
}

Debug::HistoryHandler& Debug::Logger::GetHistoryHandler(std::string p_id)
{
	return HISTORY_HANDLER_MAP[p_id];
}
