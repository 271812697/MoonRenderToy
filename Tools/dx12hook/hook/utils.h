#pragma once
#include <windows.h>
#include <map>
#include <string>
#include <vector>

using namespace std;

namespace MOON
{


	struct vmthook_t
	{
		vmthook_t(void** vtable, void* original, void* hook, int index) : vtable(vtable), original(original), hook(hook), index(index) {}
		void** vtable = nullptr;
		void* original = nullptr;
		void* hook = nullptr;
		int index = 0;
	};

	class VTableHook
	{
	public:
		//初始化虚表HOOK，替换虚表
		void Initialize(void* pTarget);

		void Swap(void* pTarget, int Index, void* pFunc);
		void* GetLastestOriginal();
		void UnSwap();

		//hook虚表上指定Idx的函数
		void Bind(uint32_t Index, void* Function);

		//卸载绑定
		void UnBind(uint32_t Index);

		//卸载全部hook
		void UnAllBind();

		void* GetTarget()
		{
			return pTarget;
		}

		//获得虚表函数指针
		template <typename T>
		T GetOriginal(uint32_t Index)
		{
			return (T)oVTable[Index];
		}

	private:
		DWORD Protect = 0;
		uint32_t CalcVTableSize();
		void* pTarget = nullptr;
		void** VTable = nullptr; //指向修改虚表
		void** oVTable = nullptr;//指向原始虚表

		vector<vmthook_t> HookList;
	};

	class Int3Hook
	{
	public:
		void Initialize(void* HookAddr, int Size, PVECTORED_EXCEPTION_HANDLER VehFunc);

		void Hook();

		void HookJmpAddr();

		void UnHook();

		void UnHookJmpAddr();

		void* GetHookAddr();

		BYTE* GetJmpAddr();

		void Remove();
	private:
		BYTE oHookByte;   //原始字节
		BYTE oJmpByte;
		void* hVEH;   //异常处理函数句柄
		BYTE* HookAddr; //Hook地址
		BYTE* JmpAddr; //下一条指令地址
	};
}






