#pragma once

/*
==============
UI Render
==============
*/

class FRenderer;
struct RenderItem_t;
class FCommandListPool;

class FRenderUI
{
public:
	FRenderUI();
	~FRenderUI();

	AkBool Initialize(FRenderer* pRenderer, DWORD dwMaxItemNum);
	AkBool Add(const RenderItem_t* pItem);
	DWORD Process(DWORD uThreadIndex, FCommandListPool* pCmdListPool, ID3D12CommandQueue* pCmdQueue, DWORD dwProcessCountPerCommandList, D3D12_CPU_DESCRIPTOR_HANDLE hRTV, D3D12_CPU_DESCRIPTOR_HANDLE hDSV, const D3D12_VIEWPORT* pViewport, const D3D12_RECT* pScissorRect);
	void Reset();

private:
	void CleanUp();

	const RenderItem_t* Dispatch();

private:
	FRenderer* _pRenderer = nullptr;
	AkU8* _pBuffer = nullptr;
	AkU32 _uMaxBufferSize = 0;
	AkU32 _uAllocatedSize = 0;
	AkU32 _uReadBufferPos = 0;
	AkU32 _uItemCount = 0;
};

