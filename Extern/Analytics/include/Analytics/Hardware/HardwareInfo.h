

#pragma once

#include <chrono>


#include "Analytics/Hardware/HardwareReport.h"
#include "Analytics/Hardware/CPUInfo.h"
#include "Analytics/Hardware/GPUInfo.h"
#include "Analytics/Hardware/RAMInfo.h"

namespace Analytics::Hardware
{

	class HardwareInfo final
	{
	public:

		HardwareInfo(double p_timeInterval = 1.0);


		HardwareReport GenerateReport();

		void Tick();

	private:
		void Update();

		double m_timeInterval;
		double m_timer;

		std::chrono::steady_clock::time_point	m_previousTime;
		std::chrono::steady_clock::time_point	m_currentTime;
		std::chrono::duration<double>			m_elapsedTime;

		float m_cpuUsage;
		float m_gpuUsage;
		
		CPUInfo m_cpuInfo;
		GPUInfo m_gpuInfo;
		RAMInfo m_ramInfo;
	};
}