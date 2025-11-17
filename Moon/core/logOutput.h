#pragma once
#include <string>
namespace MOON {

	class LogOutput
	{
	public:
		enum Level
		{
			LL_INVALID = -1,
			LL_DEBUG,
			LL_INFO,
			LL_WARNING,
			LL_ERROR,
			LL_FATAL,
		};

		// log message
		virtual void logMessage(Level level, const std::string& msg) = 0;

	public:
		LogOutput(const std::string& name) : m_name(name) {}

		// get log name
		virtual const std::string& getName() const { return m_name; }

	private:
		std::string				m_name;
	};
}
