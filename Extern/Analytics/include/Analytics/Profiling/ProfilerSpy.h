

#pragma once

#include <string>
#include <chrono>


#include "Analytics/Profiling/Profiler.h"
#include "Analytics/Profiling/ProfilerSpy.h"


#define PROFILER_SPY(name)\
		std::unique_ptr<Analytics::Profiling::ProfilerSpy> __profiler_spy__ = \
		Analytics::Profiling::Profiler::IsEnabled() ? std::make_unique<Analytics::Profiling::ProfilerSpy>(name) : nullptr

namespace Analytics::Profiling
{

	struct ProfilerSpy final
	{
		/**
		* Create the profiler spy with the given name.
		* @param p_name
		*/
		ProfilerSpy(const std::string& p_name);

		/**
		* Destroy the profiler spy.
		* On destruction, his collected data will be saved in the profiler
		*/
		~ProfilerSpy();

		const	std::string								name;
		const	std::chrono::steady_clock::time_point	start;
				std::chrono::steady_clock::time_point	end;
	};
}