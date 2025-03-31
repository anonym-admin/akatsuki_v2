#pragma once

/*
===================
PostProcess
===================
*/

class FRenderer;
class FImageFilter;
class FCommandListPool;

class FPostProcess
{
public:
	static const AkU32 DESCRIPTOR_COUNT_PER_BLOOM = 2; // t0, b0
	static const AkU32 MAX_BLOOM_LEVEL = 4;
	static const AkU32 DESCCIPTOR_COUNT_COMBINE = 3; // t0, t1, b0
	static const AkU32 MAX_DESCRIPTOR_COUNT = DESCRIPTOR_COUNT_PER_BLOOM * MAX_BLOOM_LEVEL + DESCCIPTOR_COUNT_COMBINE;

	FPostProcess();
	~FPostProcess();

	AkBool Initialize(FRenderer* pRenderer, AkU32 uBloomLevels, AkU32 uWidth, AkU32 uHeight);
	void Process(AkU32 uThreadIndex, FCommandListPool* pCmdListPool, ID3D12CommandQueue* pCmdQueue, D3D12_CPU_DESCRIPTOR_HANDLE hBackBufferRTV, const D3D12_VIEWPORT* pViewport, const D3D12_RECT* pScissorRect);
	AkBool CreateBuffers(AkU32 uWidth, AkU32 uHeight);
	void RenderImageFilter(AkU32 uThreadIndex, FCommandListPool* pCmdListPool, ID3D12GraphicsCommandList* pCmdList, ID3D12CommandQueue* pCmdQueue, FImageFilter* pImageFilter, CD3DX12_CPU_DESCRIPTOR_HANDLE* pCPU, CD3DX12_GPU_DESCRIPTOR_HANDLE* pGPU);

private:
	void CleanUp();

	AkBool CreateRootSignature();
	AkBool CreatePipelineState();
	AkBool CreateMeshBuffers();
	AkBool CreateImageFilters(AkU32 uWidth, AkU32 uHeight);
	void DestroyRootSignature();
	void DestroyPipelineState();
	void DestroyMeshBuffers();
	void DestroyImageFilters();

	void CreateBuffer(AkU32 uWidth, AkU32 uHeight, ID3D12Resource** ppOutBuffer, D3D12_CPU_DESCRIPTOR_HANDLE* pOutSrvCpu, D3D12_CPU_DESCRIPTOR_HANDLE* pOutRtvCpu, AkU32 uIndex);
	void DestroyBuffer();

private:
	FRenderer* _pRenderer = nullptr;
	ID3D12RootSignature* _pUpDownFilterRootSignature = nullptr;
	ID3D12RootSignature* _pCombineRootSignature = nullptr;
	ID3D12PipelineState* _pCombinePSO = nullptr;
	ID3D12PipelineState* _pBloomDownPSO = nullptr;
	ID3D12PipelineState* _pBloomUpPSO = nullptr;
	ID3D12Resource* _pVertexBuffer = nullptr;
	ID3D12Resource* _pIndexBuffer = nullptr;
	D3D12_VERTEX_BUFFER_VIEW _tVertexBufferView = {};
	D3D12_INDEX_BUFFER_VIEW _tIndexBufferView = {};
	ID3D12Resource** _ppBloomBuffers = nullptr;

	D3D12_CPU_DESCRIPTOR_HANDLE _hSrvCpu[MAX_BLOOM_LEVEL] = {};
	D3D12_CPU_DESCRIPTOR_HANDLE _hRtvCpu[MAX_BLOOM_LEVEL] = {};

	FImageFilter** _ppBloomDownFilters = nullptr;
	FImageFilter** _ppBloomUpFilters = nullptr;
	FImageFilter* _pCombineFilter = nullptr;

	AkU32 _uBloomLevel = 0;
};

