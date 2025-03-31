#pragma once

/*
=============
Post Effect
=============
*/

class FRenderer;
class FCommandListPool;

class FPostEffect
{
public:
	static const AkU32 DESCRIPTOR_COUNT_PER_OBJ = 3; // t0, t1, b0

	FPostEffect();
	~FPostEffect();

	AkBool Initialize(FRenderer* pRenderer);
	void Process(AkU32 uThreadIndex, FCommandListPool* pCmdListPool, ID3D12CommandQueue* pCmdQueue, const D3D12_VIEWPORT* pViewport, const D3D12_RECT* pScissorRect);

private:
	void CleanUp();

	AkBool CreateRootSignature();
	AkBool CreatePipelineState();
	AkBool CreateMeshBuffers();
	void DestroyRootSignature();
	void DestroyPipelineState();
	void DestroyMeshBuffers();

private:
	FRenderer* _pRenderer = nullptr;
	ID3D12RootSignature* _pRootSignature = nullptr;
	ID3D12PipelineState* _pPostEffectPSO = nullptr;
	ID3D12Resource* _pVertexBuffer = nullptr;
	ID3D12Resource* _pIndexBuffer = nullptr;
	D3D12_VERTEX_BUFFER_VIEW _tVertexBufferView = {};
	D3D12_INDEX_BUFFER_VIEW _tIndexBufferView = {};
};

