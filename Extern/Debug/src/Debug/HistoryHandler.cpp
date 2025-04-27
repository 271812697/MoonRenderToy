
#include "Debug/HistoryHandler.h"

std::queue<Debug::LogData> Debug::HistoryHandler::LOG_QUEUE;

void Debug::HistoryHandler::Log(const LogData& p_logData)
{
	LOG_QUEUE.push(p_logData);
}

std::queue<Debug::LogData>& Debug::HistoryHandler::GetLogQueue()
{
	return LOG_QUEUE;
}