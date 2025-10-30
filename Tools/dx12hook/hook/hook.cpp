#include "hook/hook.h"
#include "Platform/Public/Platform.h"
#include "hook/utils.h"
#include "imgui/imgui.h"
#include "ImGui/imgui_impl_win32.h"
#include "ImGui/imgui_impl_dx12.h"
IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
namespace MOON {

	VTableHook vtcommandQueue;
	VTableHook vtSwapChain;
	void* commandQueueCtx = nullptr;
	void* swapchainCtx = nullptr;

	typedef HRESULT(__stdcall* Present)(IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT Flags);
	typedef void(APIENTRY* ExecuteCommandLists)(ID3D12CommandQueue* queue, UINT NumCommandLists, ID3D12CommandList* ppCommandLists);
	typedef HRESULT(WINAPI* Resize)(IDXGISwapChain* This, UINT BufferCount, UINT Width, UINT Height, DXGI_FORMAT NewFormat, UINT SwapChainFlags);

	LRESULT WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
		switch (msg)
		{
		case WM_KEYUP:

			break;
		case WM_LBUTTONDOWN:

			break;
		case WM_RBUTTONDOWN:

			break;
		case WM_MBUTTONDOWN:

			break;

		case WM_XBUTTONDOWN:

			break;
		}



		if ( ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
			return TRUE;

		return CallWindowProcA((WNDPROC)Hook::instance()->getWndProc(), hWnd, msg, wParam, lParam);
	}
	HRESULT APIENTRY HookPresent(IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT Flags)
	{
		if(!Hook::instance()->isInit()){
			if(!Hook::instance()->getDevice(pSwapChain))
			return vtSwapChain.GetOriginal<Present>(8)(pSwapChain, SyncInterval, Flags);
		}
		if (Hook::instance()->getCommandQueue()==nullptr){
			return vtSwapChain.GetOriginal<Present>(8)(pSwapChain, SyncInterval, Flags);
		}

		ImGui_ImplDX12_NewFrame();
		ImGui_ImplWin32_NewFrame();

		ImGui::NewFrame();

		//to draw menu
		ImGui::Text("Hello hook");

		ImGui::EndFrame();
        auto commandList = Hook::instance()->getCommandList();
		auto descHeap = Hook::instance()->getDescriptorHeap();
		auto commandQueue = Hook::instance()->getCommandQueue();
		//	Flush scene
		UINT bufferIndex = Hook::instance()->getSwapChain3()->GetCurrentBackBufferIndex();
		_FrameContext& frameCtx=Hook::instance()->getFrameContext(bufferIndex);
		D3D12_RESOURCE_BARRIER barrier = {};
		barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
		barrier.Transition.pResource = frameCtx.Resource;
		barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
		barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
		barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;

		//frameCtx.CommandAllocator->Reset();
		std::cout << "present" << std::endl;
		commandList->Reset(frameCtx.CommandAllocator, NULL);
		commandList->ResourceBarrier(1, &barrier);
		commandList->OMSetRenderTargets(1, &frameCtx.DescriptorHandle, FALSE, NULL);
		commandList->SetDescriptorHeaps(1, &descHeap);

		ImGui::Render();
		ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), commandList);

		barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
		barrier.Transition.pResource = frameCtx.Resource;
		barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
		barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
		barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;

		
		commandList->ResourceBarrier(1, &barrier);
		commandList->Close();
		commandQueue->ExecuteCommandLists(1, (ID3D12CommandList* const*)&commandList);

		return vtSwapChain.GetOriginal<Present>(8)(pSwapChain, SyncInterval, Flags);

	}
	void HookExecuteCommandLists(ID3D12CommandQueue* queue, UINT NumCommandLists, ID3D12CommandList* ppCommandLists)
	{
		if (Hook::instance()->getCommandQueue() == nullptr) {
			Hook::instance()->setCommandQueue(queue);
		}
		vtcommandQueue.GetOriginal<ExecuteCommandLists>(10)(queue, NumCommandLists, ppCommandLists);
	}
	HRESULT APIENTRY HookResize(IDXGISwapChain* _this, UINT BufferCount, UINT Width, UINT Height, DXGI_FORMAT NewFormat, UINT SwapChainFlags)
	{
		Hook::instance()->resetRenderState();
		return vtSwapChain.GetOriginal<Resize>(13)(_this, BufferCount, Width, Height, NewFormat, SwapChainFlags);
	}
	class Hook::HookInternal {
	public:
		HookInternal(Hook* hook):mSelf(hook) {
		
		}
		void start(const char* pClassName, const char* pWindowName)
		{
			mHwnd = FindWindowA(pClassName, pWindowName);
			while (!mHwnd)
			{
				this->mHwnd = FindWindowA(pClassName, pWindowName);
				Sleep(100);
			}
			Sleep(5000);
			//4D 8B 0C 24 49 8B CC 4C 8B 44 24
			//commandQueueCtx = PlatformWindows::FindPattern("4D 8B 0C 24 49 8B CC 4C 8B 44 24");
			commandQueueCtx = PlatformWindows::FindPattern("FF 95 A8 04 00 00 90 48 8B 05 2C FC 0C 00");
			swapchainCtx = PlatformWindows::FindPattern("FF 95 A8 04 00 00 89 85 B4 02 00 00");
			uintptr_t nextAddr = (uintptr_t)commandQueueCtx + 14;  //
			uintptr_t pptr=nextAddr + 0x000CFC2C;
			
			if (!commandQueueCtx || !swapchainCtx)
			{
				MessageBoxA(0, "无法找到交换链或命令队列", "错误", MB_ICONINFORMATION);
				return;
			}
			std::cout << "已经找到交换链和命令队列" << std::endl;
		
			LPVOID JMPAddr = VirtualAlloc(0, 0x1000, MEM_COMMIT, PAGE_EXECUTE_READWRITE);

			if (!JMPAddr)
			{
				return;
			}

			UCHAR JmpComCode[] =
			{
				0x48, 0xB8,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, // mov rax, <JMP> (10 字节)
				0xFF, 0xE0 // jmp rax
			};
			UCHAR JmpSwapCode[] =
			{
				0x48, 0xB8,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, // mov rax, <JMP> (10 字节)
				0xFF, 0xE0, // jmp rax 
			};

			UCHAR OriComCode[] =
			{
				0xFF, 0x95 ,0xA8, 0x04 ,0x00, 0x00,
                0x90, 0x48, 0x8B, 0x05, 0x2C, 0xFC,0x0C,0x00//rcx,qword ptr [g_pd3dCommandQueue (07FF781FC3348h)]
			};
			//FF 95 A8 04 00 00    call        qword ptr[rbp + 4A8h]
			//89 85 B4 02 00 00    mov         dword ptr[rbp + 2B4h], eax
			// ！！！根据你找到的新地址更新原始指令
			UCHAR OriSwapCode[] =
			{
				0xFF, 0x95, 0xA8,       // mov r8d, esi    (修正)
				0x04, 0x00, 0x00,       // mov edx, r12d   (修正)
				0x89, 0x85, 0xB4,       // mov rax, [rcx]
				0x02, 0x00, 0x00        // call qword ptr [rax+40]
			};

			// 定义 Shellcode 的机器码
			UCHAR ShellCode[] = {
				// ------------------- commandQueueCtx 的 Hook 处理代码 (37 字节) -------------------
				// 保存 RCX 到 JMPAddr + 0x500
				0x48, 0xB8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // mov rax, <JMPAddr + 0x500> (10 字节)
				0x48, 0x89, 0x08,                           // mov [rax], rcx
				// 执行原始指令 
				0xFF, 0x95 ,0xA8, 0x04 ,0x00, 0x00,
				 0x90, 0x48, 0x8B, 0x05, 0x2C, 0xFC,0x0C,0x00,//rcx,qword ptr [g_pd3dCommandQueue (07FF781FC3348h)]
				// 跳转回 commandQueueCtx + 12 
				0x48, 0xB8,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, // mov rax, <JMP> (10 字节)
				0xFF, 0xE0, // jmp rax

				// ------------------- swapchainCtx 的 Hook 处理代码 (42 字节) -------------------
				// 保存 [RCX] 到 JMPAddr + 0x600 (RCX是swapchain的this指针)
				0x48, 0xB8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,   // mov rax, <JMPAddr + 0x600> (10 字节)
				0x48, 0x89, 0x08,                           // mov [rax], rcx
				// ！！！执行原始指令 (根据新地址更新)
				0xFF, 0x95, 0xA8,                           // mov r8d, esi     (修正)
				0x04, 0x00, 0x00,                        // mov edx, r12d    (修正)
				0x89, 0x85, 0xB4,                           // mov rax, [rcx]
				0x02, 0x00, 0x00,                           // call qword ptr [rax+40]
				// 跳转回 swapchainCtx + 12 
				0x48, 0xB8,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, // mov rax, <JMP> (10 字节)
				0xFF, 0xE0, // jmp rax
			};
			//先填充 JmpAddr +500  和  JmpAddr +600
			uintptr_t jmpAddr500 = (uintptr_t)JMPAddr + 0x500;
			uintptr_t jmpAddr600 = (uintptr_t)JMPAddr + 0x600;
			memcpy(&ShellCode[2], &jmpAddr500, 8);   // 填充commandQueue的存储地址
			memcpy(&ShellCode[41], &jmpAddr600, 8);  // 填充swapchain的存储地址

			// 计算 Shellcode 各部分偏移
			uint64_t shellcodeBase = (uint64_t)JMPAddr;
			uint64_t commandHandler = shellcodeBase;          // commandQueueCtx 处理代码入口
			uint64_t swapchainHandler = shellcodeBase + 39;   // swapchainCtx 处理代码入口

			//拷贝ShellCode到JMPAddr
			memcpy(JMPAddr, &ShellCode, sizeof(ShellCode));

			// 设置commandQueue shellcode中的跳回地址
			uint64_t jmpBackCommandAddr = commandHandler + 27; // 跳转指令的起始地址
			uintptr_t Addr = (uintptr_t)commandQueueCtx + 14;  // 覆盖了12字节，跳回到下一条指令
			memcpy((void*)(jmpBackCommandAddr + 2), &Addr, 8); // 填充到 0x48, 0xB8 后

			// 设置swapchain shellcode中的跳回地址
			Addr = (uintptr_t)swapchainCtx + 12; // 覆盖了12字节，跳回到下一条指令
			uint64_t jmpBackSwapchainAddr = swapchainHandler + 25; // 跳转指令的起始地址
			memcpy((void*)(jmpBackSwapchainAddr + 2), &Addr, 8); // 填充到 0x48, 0xB8 后

			//Hook点的跳转指令：设置JmpCode跳转到我们的Shellcode
			memcpy(JmpComCode + 2, &commandHandler, 8);
			memcpy(JmpSwapCode + 2, &swapchainHandler, 8);

			//拷贝到Hook点
			DWORD commandQueueoldProtect = 0;
			DWORD swapchainoldProtect = 0;
			//VirtualProtect(commandQueueCtx, sizeof(JmpComCode), PAGE_EXECUTE_READWRITE, &commandQueueoldProtect);
			VirtualProtect(swapchainCtx, sizeof(JmpSwapCode), PAGE_EXECUTE_READWRITE, &swapchainoldProtect);
			std::cout << "替换目标函数指令" << std::endl;
			//memcpy(commandQueueCtx, JmpComCode, sizeof(JmpComCode));
			memcpy(swapchainCtx, JmpSwapCode, sizeof(JmpSwapCode));
			// 等待Hook捕获到目标指针
			while (true)
			{
				Sleep(100);
				//uintptr_t jmp500 = *(uintptr_t*)jmpAddr500;
				uintptr_t jmp600 = *(uintptr_t*)jmpAddr600;

				if (jmp600)
					break;
			}
			std::cout << "找到目标地址" << std::endl;

			//恢复原始指令
			//memcpy(commandQueueCtx, OriComCode, sizeof(OriComCode));
			memcpy(swapchainCtx, OriSwapCode, sizeof(OriSwapCode));

			//VirtualProtect(commandQueueCtx, sizeof(JmpComCode), commandQueueoldProtect, &commandQueueoldProtect);
			VirtualProtect(swapchainCtx, sizeof(JmpSwapCode), swapchainoldProtect, &swapchainoldProtect);
			// 初始化VTable Hook
			vtcommandQueue.Initialize(*(uintptr_t**)pptr);
			vtSwapChain.Initialize(*(uintptr_t**)jmpAddr600);


			while (!vtcommandQueue.GetTarget() && !vtSwapChain.GetTarget())
			{
				Sleep(100);
			}

			std::cout << "成功hook到虚函数" << std::endl;
			VirtualFree(JMPAddr, 0, MEM_RELEASE);
			// 安装VTable Hook
			vtcommandQueue.Bind(10, HookExecuteCommandLists);
			vtSwapChain.Bind(8, HookPresent);
			vtSwapChain.Bind(13, HookResize);
		}

		bool getDevice(IDXGISwapChain* pSwapChain)
		{
			pSwapChain->QueryInterface(IID_PPV_ARGS(&this->mSwapchain3));
			if (this->mSwapchain3 == nullptr)
				return FALSE;

			if (pSwapChain->GetDevice(IID_PPV_ARGS(&this->mDevice)) != S_OK)
				return FALSE;


			pSwapChain->GetDesc(&this->mSwapDesc);
			mFrameContext.clear();
			this->mBuffersCounts = this->mSwapDesc.BufferCount;
			mFrameContext.resize(this->mBuffersCounts);
			D3D12_DESCRIPTOR_HEAP_DESC desc = {};
			desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
			desc.NumDescriptors = this->mBuffersCounts;//1;
			desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;

			if (this->mDevice->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&this->mDescriptorHeap)) != S_OK)//imguirender
			{
				//this->m_Device->Release();
				return FALSE;
			}

			desc = D3D12_DESCRIPTOR_HEAP_DESC();
			desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
			desc.NumDescriptors = this->mBuffersCounts;
			desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
			desc.NodeMask = 1;

			if (this->mDevice->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&this->mrtvDescriptorHeap)) != S_OK)//DescriptorHeapBackBuffers
			{
				this->mDescriptorHeap->Release();
				return FALSE;
			}

			ID3D12CommandAllocator* Allocator;
			if (this->mDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&Allocator)) != S_OK)
			{
				this->mDescriptorHeap->Release();
				this->mrtvDescriptorHeap->Release();
				return FALSE;
			}

			if (this->mDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, Allocator, NULL, IID_PPV_ARGS(&this->mCommandList)) != S_OK || this->mCommandList->Close() != S_OK)
			{
				this->mDescriptorHeap->Release();
				this->mrtvDescriptorHeap->Release();
				return FALSE;
			}

			SIZE_T						rtvDescriptorSize = this->mDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
			D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = this->mrtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();

			//this->FrameContext = new _FrameContext[this->m_BuffersCounts];
			for (UINT i = 0; i < this->mBuffersCounts; ++i)
			{
				ID3D12Resource* pBackBuffer = nullptr;
				this->mFrameContext[i].CommandAllocator = Allocator;
				this->mFrameContext[i].DescriptorHandle = rtvHandle;
				pSwapChain->GetBuffer(i, IID_PPV_ARGS(&pBackBuffer));
				this->mDevice->CreateRenderTargetView(pBackBuffer, NULL, rtvHandle);
				this->mFrameContext[i].Resource = pBackBuffer;
				rtvHandle.ptr += rtvDescriptorSize;
			}

			//	Finished
			this->moWndProc = SetWindowLongPtrW(this->mHwnd, GWLP_WNDPROC, (LONG_PTR)WndProc);
			this->mInitFlag = TRUE;
			this->initializeImGui(this->mDevice);
			return TRUE;
		}
		void initializeImGui(ID3D12Device* pDevice)
		{
			ImGui::CreateContext();

			// 完全禁用ini文件
			ImGuiIO& io = ImGui::GetIO();
			io.IniFilename = nullptr;
			io.LogFilename = nullptr;

			// 现代化深色风格
			ImGui::StyleColorsDark();
			ImGuiStyle& style = ImGui::GetStyle();
			ImVec4* colors = style.Colors;

			// 去除所有圆角 - 直角设计
			style.WindowRounding = 0.0f;
			style.FrameRounding = 0.0f;
			style.PopupRounding = 0.0f;
			style.GrabRounding = 0.0f;
			style.TabRounding = 0.0f;
			style.ScrollbarRounding = 0.0f;
			style.ChildRounding = 0.0f;

			// 紧凑的内边距
			style.WindowPadding = ImVec2(10, 10);
			style.FramePadding = ImVec2(8, 6);
			style.ItemSpacing = ImVec2(8, 6);
			style.ItemInnerSpacing = ImVec2(6, 4);

			// 推荐：深绿色风格（专业、护眼、现代）
			colors[ImGuiCol_Text] = ImVec4(0.95f, 0.96f, 0.98f, 1.00f);
			colors[ImGuiCol_TextDisabled] = ImVec4(0.50f, 0.52f, 0.55f, 1.00f);

			// 窗口背景 - 深灰绿色
			colors[ImGuiCol_WindowBg] = ImVec4(0.12f, 0.14f, 0.12f, 0.98f);
			colors[ImGuiCol_ChildBg] = ImVec4(0.10f, 0.12f, 0.10f, 0.98f);
			colors[ImGuiCol_PopupBg] = ImVec4(0.14f, 0.16f, 0.14f, 0.99f);

			// 边框 - 深绿色
			colors[ImGuiCol_Border] = ImVec4(0.20f, 0.25f, 0.20f, 0.80f);

			// 按钮 - 翠绿色渐变
			colors[ImGuiCol_Button] = ImVec4(0.15f, 0.45f, 0.25f, 0.90f);
			colors[ImGuiCol_ButtonHovered] = ImVec4(0.20f, 0.55f, 0.30f, 0.90f);
			colors[ImGuiCol_ButtonActive] = ImVec4(0.25f, 0.65f, 0.35f, 1.00f);

			// 复选框 - 亮绿色
			colors[ImGuiCol_CheckMark] = ImVec4(0.30f, 0.80f, 0.40f, 1.00f);

			// 复选框背景
			colors[ImGuiCol_FrameBg] = ImVec4(0.18f, 0.20f, 0.18f, 0.80f);
			colors[ImGuiCol_FrameBgHovered] = ImVec4(0.22f, 0.24f, 0.22f, 0.90f);
			colors[ImGuiCol_FrameBgActive] = ImVec4(0.25f, 0.28f, 0.25f, 1.00f);

			// 滑块
			colors[ImGuiCol_SliderGrab] = ImVec4(0.30f, 0.80f, 0.40f, 1.00f);
			colors[ImGuiCol_SliderGrabActive] = ImVec4(0.35f, 0.85f, 0.45f, 1.00f);

			// 其他保持深色系
			colors[ImGuiCol_TitleBg] = ImVec4(0.08f, 0.20f, 0.10f, 1.00f);
			colors[ImGuiCol_TitleBgActive] = ImVec4(0.10f, 0.25f, 0.12f, 1.00f);
			colors[ImGuiCol_Header] = ImVec4(0.20f, 0.45f, 0.25f, 0.80f);
			colors[ImGuiCol_HeaderHovered] = ImVec4(0.25f, 0.55f, 0.30f, 0.90f);
			colors[ImGuiCol_HeaderActive] = ImVec4(0.30f, 0.65f, 0.35f, 1.00f);

			// 对齐设置
			style.SeparatorTextAlign = ImVec2(0.5f, 0.5f);
			style.WindowTitleAlign = ImVec2(0.5f, 0.5f);

			io.ConfigFlags |= ImGuiConfigFlags_NoMouseCursorChange;

			// 加载中文字体
			io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\simhei.ttf", 16.0f, nullptr, io.Fonts->GetGlyphRangesChineseFull());

			ImGui_ImplWin32_Init(mHwnd);
			ImGui_ImplDX12_Init(pDevice, this->mBuffersCounts, DXGI_FORMAT_R8G8B8A8_UNORM, nullptr,
				this->mDescriptorHeap->GetCPUDescriptorHandleForHeapStart(),
				this->mDescriptorHeap->GetGPUDescriptorHandleForHeapStart());
			ImGui_ImplDX12_CreateDeviceObjects();
		}
		void resetRenderState()
		{
			if (!this->mInitFlag)
				return;

			for (UINT i = 0; i < mBuffersCounts; ++i)
			{
				if (this->mFrameContext[0].CommandAllocator != nullptr)
					this->mFrameContext[0].CommandAllocator->Release();

				this->mFrameContext[i].CommandAllocator = nullptr;
				this->mFrameContext[i].Resource->Release();
				this->mFrameContext[i].Resource = nullptr;
			}

			this->mDescriptorHeap->Release();
			this->mrtvDescriptorHeap->Release();

			ImGui_ImplDX12_Shutdown();
			ImGui_ImplWin32_Shutdown();
			ImGui::DestroyContext();
			SetWindowLongPtr(this->mHwnd, GWLP_WNDPROC, (LONG_PTR)this->moWndProc);
			this->mInitFlag = false;
		}
		ID3D12CommandQueue* getCommandQueue()
		{
			return mCommandQueue;
		}
		void setCommandQueue(ID3D12CommandQueue* queue)
		{
			mCommandQueue = queue;
		}
		ID3D12GraphicsCommandList* getCommandList() {
			return mCommandList;
		}
		void setCommandList(ID3D12GraphicsCommandList* commandList) {
			mCommandList = commandList;
		}
		IDXGISwapChain3* getSwapChain3() {
			return mSwapchain3;
		}
		_FrameContext& getFrameContext(int index)
		{
			return mFrameContext[index];
		}
		ID3D12DescriptorHeap* getDescriptorHeap() {
			return mDescriptorHeap;
		}
		uint64_t getWndProc() {
			return moWndProc;
		}
	private:
		friend Hook;
		Hook* mSelf = nullptr;
		bool mInitFlag = false;
		HWND mHwnd = nullptr;
		uint64_t moWndProc = 0;
		IDXGISwapChain3* mSwapchain3 = nullptr;
		std::vector<_FrameContext> mFrameContext;

		DXGI_SWAP_CHAIN_DESC mSwapDesc{};
		ID3D12Device* mDevice = nullptr;
		uint64_t mBuffersCounts = -1;
		ID3D12DescriptorHeap* mDescriptorHeap = nullptr;
		ID3D12DescriptorHeap* mrtvDescriptorHeap = nullptr;
		ID3D12CommandAllocator** mCommandAllocator = nullptr;
		ID3D12GraphicsCommandList* mCommandList = nullptr;
		ID3D12CommandQueue* mCommandQueue = nullptr;
		ID3D12Resource** mBackBuffer = nullptr;
	};

	Hook* Hook::instance() {
		static Hook hook;
		return &hook;
	}
	void Hook::start(const char* pClassName, const char* pWindowName)
	{
		mInternal->start(pClassName,pWindowName);
    }
	bool Hook::getDevice(IDXGISwapChain* pSwapChain)
	{
		return mInternal->getDevice(pSwapChain);
	}
	void Hook::initializeImGui(ID3D12Device* pDevice)
	{
		mInternal->initializeImGui(pDevice);
	}
	void Hook::resetRenderState()
	{
		mInternal->resetRenderState();
	}
	ID3D12GraphicsCommandList* Hook::getCommandList() {
		return mInternal->getCommandList();
	}
	IDXGISwapChain3* Hook::getSwapChain3() {
		return mInternal->getSwapChain3();
	}
	ID3D12DescriptorHeap* Hook::getDescriptorHeap() {
		return mInternal->getDescriptorHeap();
	}
	ID3D12CommandQueue* Hook::getCommandQueue()
	{
		return mInternal->getCommandQueue();
	}
	void Hook::setCommandQueue(ID3D12CommandQueue* queue)
	{
		mInternal->setCommandQueue(queue);
	}
	bool Hook::isInit()
	{
		return mInternal->mInitFlag;
	}
	void Hook::setInit(bool flag)
	{
		mInternal->mInitFlag = flag;
	}
	_FrameContext& Hook::getFrameContext(int index)
	{
		return mInternal->getFrameContext(index);
	}
	uint64_t Hook::getWndProc() {
		return mInternal->getWndProc();
	}
	Hook::Hook():mInternal(new HookInternal(this))
	{
	}
}