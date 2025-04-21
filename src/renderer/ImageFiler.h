#pragma once

/*
============
Image Filer
============
*/

class FRenderer;

class FImageFilter
{
public:
	static const AkU32 MAX_CPU_HANDLE_COUNT = 3;

	FImageFilter();
	~FImageFilter();

	AkBool Initialize(FRenderer* pRenderer, AkU32 uWidth, AkU32 uHeight, ID3D12RootSignature* pRootSignature, ID3D12PipelineState* pPSO);
	void Draw(AkU32 uThreadIndex, ID3D12GraphicsCommandList* pCmdList, CD3DX12_CPU_DESCRIPTOR_HANDLE* pCPU, CD3DX12_GPU_DESCRIPTOR_HANDLE* pGPU);

	void SetSrvCpu(D3D12_CPU_DESCRIPTOR_HANDLE* pSrvCpuList, AkU32 uNum = 1);
	void SetRtvCpu(D3D12_CPU_DESCRIPTOR_HANDLE* pRtvCpuList, AkU32 uNum = 1);

private:
	void CleanUp();

private:
	FRenderer* _pRenderer = nullptr;
	D3D12_CPU_DESCRIPTOR_HANDLE	_pSrvCpuList[MAX_CPU_HANDLE_COUNT] = {};
	D3D12_CPU_DESCRIPTOR_HANDLE	_pRtvCpuList[MAX_CPU_HANDLE_COUNT] = {};
	ID3D12RootSignature* _pRootSignature = nullptr;
	ID3D12PipelineState* _pPSO = nullptr;
	D3D12_VIEWPORT _tViewPort = {};
	D3D12_RECT _tScissorRect = {};
	AkU32 _uSrvNum = 0;
	AkU32 _uRtvNum = 0;
	AkU32 _uWidth = 0;
	AkU32 _uHeight = 0;
};

