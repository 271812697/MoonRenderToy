#pragma once
#include <dxgi.h>
#include <d3d12.h>
#include <dxgi1_4.h>
#include <vector>
#include <memory>
#include <thread>

namespace MOON {	
	struct _FrameContext
	{
		ID3D12CommandAllocator* CommandAllocator;
		ID3D12Resource* Resource;
		D3D12_CPU_DESCRIPTOR_HANDLE DescriptorHandle;
	};
	class Hook {
	public:
		static Hook* instance();
		void start(const char* pClassName = NULL, const char* pWindowName = NULL);
		bool getDevice(IDXGISwapChain* pSwapChain);
		void initializeImGui(ID3D12Device* pDevice);
		void resetRenderState();
		ID3D12GraphicsCommandList* getCommandList();
		ID3D12CommandQueue* getCommandQueue();
		IDXGISwapChain3* getSwapChain3();
		void setCommandQueue(ID3D12CommandQueue* queue);
		bool isInit();
		void setInit(bool flag);
		_FrameContext& getFrameContext(int index);
		ID3D12DescriptorHeap* getDescriptorHeap();
		uint64_t getWndProc();
	private:
		Hook();
	private:
		class HookInternal;
		HookInternal* mInternal = nullptr;

	};
}