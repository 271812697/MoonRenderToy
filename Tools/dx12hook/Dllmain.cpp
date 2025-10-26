#include <windows.h>
#include <thread>
#include <iostream>
#include <fstream>
#include "hook/hook.h"

void Start()
{
	MOON::Hook::instance()->start(("Dx12Hook"), nullptr);
}

HANDLE g_hTimerQueue = NULL;
HANDLE g_hTimer = NULL;

VOID CALLBACK TimerProc(PVOID lpParameter, BOOLEAN TimerOrWaitFired)
{
	std::thread([]() {	Start();	}).detach();
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
	
	std::ofstream file("testHook.txt");
	file << "DllMain" << std::endl;file << "bengin" << std::endl;file << "start begin" << std::endl;
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:

		g_hTimerQueue = CreateTimerQueue();
		
		if (g_hTimerQueue)
		{
			
			CreateTimerQueueTimer(&g_hTimer, g_hTimerQueue, TimerProc, NULL, 5000, 0, WT_EXECUTEINTIMERTHREAD);
		}
		
		break;

	case DLL_PROCESS_DETACH:

		if (g_hTimer)
			DeleteTimerQueueTimer(g_hTimerQueue, g_hTimer, NULL);

		if (g_hTimerQueue)
			DeleteTimerQueueEx(g_hTimerQueue, NULL);

		Sleep(500);

		break;
	}
	return TRUE;
}

