

#pragma once

#include <Windows.h>
#include <stdint.h>

namespace Analytics::Hardware
{

	class CPUInfo final
	{
	public:

		float CalculateCPULoad();

	private:
		float CalculateCPULoad(uint64_t idleTicks, uint64_t totalTicks);
		uint64_t FileTimeToInt64(const FILETIME& ft);

	private:
		uint64_t m_cpuPreviousTotalTicks = 0;
		uint64_t m_cpuPreviousIdleTicks = 0;
	};
}