

#include "Analytics/Profiling/ProfilerSpy.h"
#include "Analytics/Profiling/Profiler.h"

Analytics::Profiling::ProfilerSpy::ProfilerSpy(const std::string & p_name) :
	name(p_name),
	start(std::chrono::steady_clock::now())
{

}

Analytics::Profiling::ProfilerSpy::~ProfilerSpy()
{
	end = std::chrono::steady_clock::now();
	Analytics::Profiling::Profiler::Save(*this);
}