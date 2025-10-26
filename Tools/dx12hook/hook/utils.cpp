#include "Utils.h"
#include "hook/lazy_importer.hpp"
namespace MOON
{
	void VTableHook::Initialize(void* pTarget)
	{
		this->pTarget = pTarget;
		this->oVTable = *(void***)pTarget;
		LI_FN(VirtualProtect)(oVTable, 1, PAGE_EXECUTE_READWRITE, &Protect);

		uint32_t Size = CalcVTableSize();

		this->VTable = new void* [Size];
		LI_FN(memcpy)(VTable, oVTable, Size * sizeof(void*));
		*(void***)pTarget = VTable;
		LI_FN(VirtualProtect)(oVTable, 1, Protect, &Protect);
	}

	void VTableHook::Swap(void* pTarget, int Index, void* pFunc)
	{
		void** vmtable = *(void***)pTarget;
		HookList.push_back({ vmtable, vmtable[Index], pFunc, Index });

		LI_FN(VirtualProtect)(vmtable, 1, PAGE_EXECUTE_READWRITE, &Protect);
		vmtable[Index] = pFunc;
		LI_FN(VirtualProtect)(vmtable, 1, Protect, &Protect);
	}

	void* VTableHook::GetLastestOriginal()
	{
		return HookList.back().original;
	}

	void VTableHook::UnSwap()
	{

		for (const auto& vmt : HookList)
		{
			LI_FN(VirtualProtect)(vmt.vtable, 1, PAGE_EXECUTE_READWRITE, &Protect);
		
			vmt.vtable[vmt.index] = vmt.original;
			LI_FN(VirtualProtect)(vmt.vtable, 1, Protect, &Protect);
		}
	}

	void VTableHook::Bind(uint32_t Index, void* Function)
	{
		VTable[Index] = Function;
	}

	void VTableHook::UnBind(uint32_t Index)
	{
		VTable[Index] = oVTable[Index];
	}

	void VTableHook::UnAllBind()
	{
		LI_FN(VirtualProtect)(oVTable, 1, PAGE_EXECUTE_READWRITE, &Protect);
		*(void***)pTarget = oVTable;
		LI_FN(VirtualProtect)(oVTable, 1, Protect, &Protect);
	}

	uint32_t VTableHook::CalcVTableSize()
	{
		for (uint32_t i = 0; true; i++)
		{
			if (oVTable[i] == nullptr)
				return i;
		}

		return 0;
	}
	
	void Int3Hook::Initialize(void* HookAddr, int Size, PVECTORED_EXCEPTION_HANDLER VehFunc)
	{
		hVEH = AddVectoredExceptionHandler(1, VehFunc);
		this->HookAddr = (BYTE*)HookAddr;
		oHookByte = *(BYTE*)HookAddr;
		JmpAddr = (BYTE*)(this->HookAddr + Size); //读入下一条指令的地址
		oJmpByte = *(BYTE*)JmpAddr;
	}

	void Int3Hook::Hook()
	{
		DWORD Protect;
		VirtualProtect(HookAddr, 1, PAGE_EXECUTE_READWRITE, &Protect); //可读可写
		*HookAddr = 0xCC;  //设置Int3 异常
		VirtualProtect(HookAddr, 1, Protect, &Protect);  //还原读写保护
	}

	void Int3Hook::HookJmpAddr()
	{
		DWORD Protect;
		VirtualProtect(HookAddr, 1, PAGE_EXECUTE_READWRITE, &Protect); //可读可写
		*JmpAddr = 0xCC;  //设置Int3 异常
		VirtualProtect(HookAddr, 1, Protect, &Protect);  //还原读写保护
	}

	void Int3Hook::UnHook()
	{
		DWORD Protect;
		VirtualProtect(HookAddr, 1, PAGE_EXECUTE_READWRITE, &Protect); //可读可写
		*HookAddr = oHookByte;  //
		VirtualProtect(HookAddr, 1, Protect, &Protect);  //还原读写保护
	}

	void Int3Hook::UnHookJmpAddr()
	{
		DWORD Protect;
		VirtualProtect(HookAddr, 1, PAGE_EXECUTE_READWRITE, &Protect); //可读可写
		*JmpAddr = oJmpByte;  //
		VirtualProtect(HookAddr, 1, Protect, &Protect);  //还原读写保护
	}

	void* Int3Hook::GetHookAddr()
	{
		return HookAddr;
	}

	BYTE* Int3Hook::GetJmpAddr()
	{
		return JmpAddr;
	}

	void Int3Hook::Remove()
	{
		UnHook();
		UnHookJmpAddr();
		RemoveVectoredExceptionHandler(hVEH);
	}

}





