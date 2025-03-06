#pragma once

/*
===================
PostProcess
===================
*/

class FRenderer;

class FPostProcess
{
public:
	FPostProcess();
	~FPostProcess();

	AkBool Initialize(FRenderer* pRenderer);
	void ApplyPostProcess(AkU32 uThreadIndex, ID3D12GraphicsCommandList* pCmdList, D3D12_CPU_DESCRIPTOR_HANDLE hBackBufferRTV, const D3D12_VIEWPORT* pViewport, const D3D12_RECT* pScissorRect);

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
	ID3D12PipelineState* _pPostProcessPSO = nullptr;
	ID3D12Resource* _pVertexBuffer = nullptr;
	ID3D12Resource* _pIndexBuffer = nullptr;
	D3D12_VERTEX_BUFFER_VIEW _tVertexBufferView = {};
	D3D12_INDEX_BUFFER_VIEW _tIndexBufferView = {};
};

