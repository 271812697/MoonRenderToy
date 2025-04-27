

#define _CRT_SECURE_NO_WARNINGS // Enable getenv

#include "../../tools/Date.h"

#include "Debug/FileHandler.h"

#include <fstream>
#include <iostream>

std::string const Debug::FileHandler::__DEFAULT_HEADER;
std::string const Debug::FileHandler::__INFO_HEADER		= "[INFO] ";
std::string const Debug::FileHandler::__WARNING_HEADER	= "[WARNING] ";
std::string const Debug::FileHandler::__ERROR_HEADER		= "[ERROR] ";
std::string Debug::FileHandler::__APP_LAUNCH_DATE			= Tools::Time::Date::GetDateAsString();
std::string const Debug::FileHandler::__LOG_EXTENSION		= ".ovlog";

std::ofstream Debug::FileHandler::OUTPUT_FILE;
std::string Debug::FileHandler::LOG_FILE_PATH = std::string(getenv("APPDATA")) + std::string("\\OverloadTech\\OvEditor\\Log\\") + __APP_LAUNCH_DATE + __LOG_EXTENSION;

void Debug::FileHandler::Log(const LogData& p_logData)
{
	if (!OUTPUT_FILE.is_open())
	{
		OUTPUT_FILE.open(LOG_FILE_PATH, std::ios_base::app);
	}

	if (OUTPUT_FILE.is_open())
		OUTPUT_FILE << GetLogHeader(p_logData.logLevel) << p_logData.date << " " << p_logData.message << std::endl;
	else
		std::cout << "Unable to create log file" << std::endl;
}

std::string& Debug::FileHandler::GetLogFilePath()
{
	return LOG_FILE_PATH;
}

void Debug::FileHandler::SetLogFilePath(const std::string& p_path)
{
	int i = static_cast<int>(p_path.size() - 1);
	while (p_path[i] != '/') --i;

	std::string fileName = p_path.substr(i + 1, p_path.size() - 1);

	LOG_FILE_PATH = p_path.substr(0, i + 1) + __APP_LAUNCH_DATE + fileName;
}

std::string Debug::FileHandler::GetLogHeader(ELogLevel p_logLevel)
{
	switch (p_logLevel)
	{
	case ELogLevel::LOG_DEFAULT:	return __DEFAULT_HEADER;
	case ELogLevel::LOG_INFO:		return __INFO_HEADER;
	case ELogLevel::LOG_WARNING:	return __WARNING_HEADER;
	case ELogLevel::LOG_ERROR:		return __ERROR_HEADER;
	}

	return "";
}