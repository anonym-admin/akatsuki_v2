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
	static const AkU32 DESCRIPTOR_COUNT_PER_OBJ = 4; // t0, t1, b0, b1

	FPostEffect();
	~FPostEffect();

	AkBool Initialize(FRenderer* pRenderer);
	void Process(AkU32 uThreadIndex, FCommandListPool* pCmdListPool, ID3D12CommandQueue* pCmdQueue, const D3D12_VIEWPORT* pViewport, const D3D12_RECT* pScissorRect);
	void SetFogStrength(AkF32 fFogStrength) { _fFogStrength = fFogStrength; }
	void SetDepthScale(AkF32 fDepthScale) { _fDepthScale = fDepthScale; }
	void SetPostEffectMode(AkI32 iMode) { _iEffectMode = iMode; }

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

	AkF32 _fFogStrength = 0.0f;
	AkF32 _fDepthScale = 0.1f;
	AkI32 _iEffectMode = 1; // 1: Post Effect Render, 2: Depth Map.
};

