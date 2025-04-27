
#pragma once

#include <Windows.h>
#include <psapi.h>

namespace Analytics::Hardware
{

	class RAMInfo final
	{
	public:
		/**
		* Update the RAMInfo
		*/
		void Update();

		/**
		* Return the total physical memory reserved (MB)
		*/
		float GetUsedRAM();

		/**
		* Return the total physical memory free (MB)
		*/
		float GetFreeRAM();

		/**
		* Return the maximum available memory on the system (MB)
		*/
		float GetMaxRAM();

	private:
		MEMORYSTATUSEX m_statex;
	};
}