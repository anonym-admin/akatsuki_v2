#pragma once

/*
===================
Render Mirror Pass
===================
*/

class FRenderer;
class FCommandListPool;
struct RenderItem_t;

class FRenderMirror
{
public:
	static const AkU32 DESCRIPTOR_COUNT_PER_OBJ = 2;
	static const AkU32 DESCRIPTOR_COUNT_PER_MESH = 7 + 3 + 1 + 5;

	FRenderMirror();
	~FRenderMirror();

	AkBool Initialize(FRenderer* pRenderer, DWORD dwMaxItemNum);
	AkBool Add(const RenderItem_t* pItem);
	void BeginMirrorRender(DWORD uThreadIndex, FCommandListPool* pCmdListPool, ID3D12CommandQueue* pCmdQueue, DWORD dwProcessCountPerCommandList, D3D12_CPU_DESCRIPTOR_HANDLE hRTV, D3D12_CPU_DESCRIPTOR_HANDLE hDSV, const D3D12_VIEWPORT* pViewport, const D3D12_RECT* pScissorRect);
	void EndMirrorRender(DWORD uThreadIndex, FCommandListPool* pCmdListPool, ID3D12CommandQueue* pCmdQueue, DWORD dwProcessCountPerCommandList, D3D12_CPU_DESCRIPTOR_HANDLE hRTV, D3D12_CPU_DESCRIPTOR_HANDLE hDSV, const D3D12_VIEWPORT* pViewport, const D3D12_RECT* pScissorRect);
	DWORD Process(DWORD uThreadIndex, FCommandListPool* pCmdListPool, ID3D12CommandQueue* pCmdQueue, DWORD dwProcessCountPerCommandList, D3D12_CPU_DESCRIPTOR_HANDLE hRTV, D3D12_CPU_DESCRIPTOR_HANDLE hDSV, const D3D12_VIEWPORT* pViewport, const D3D12_RECT* pScissorRect);
	void Reset();

private:
	void CleanUp();

	const RenderItem_t* Dispatch();

	AkBool CreateCommoneResources();
	AkBool CreateRootSignature();
	AkBool CreatePipelineState();
	AkBool CreateMeshBuffers();

	void DestroyCommonResources();
	void DestroyRootSignature();
	void DestroyPipelineState();
	void DestroyMeshBuffers();

private:
	static ID3D12RootSignature* sm_pRootSignature;
	static ID3D12PipelineState* sm_pStencilMaskPSO;
	static ID3D12PipelineState* sm_pMirrorBlendPSO;
	static ID3D12PipelineState* sm_pBasicSolidPSO;
	static AkU32 sm_uInitRefCount;
	static Mesh_t* sm_pMesh;
	static MaterialConstantBuffer_t* sm_pMaterial;

	FRenderer* _pRenderer = nullptr;
	AkU8* _pBuffer = nullptr;
	AkU32 _uMaxBufferSize = 0;
	AkU32 _uAllocatedSize = 0;
	AkU32 _uReadBufferPos = 0;
	AkU32 _uItemCount = 0;

public:
	AkF32 _fAlpha = 0.8f;
};

