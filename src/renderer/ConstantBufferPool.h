#pragma once

/*
=====================
FConstantBufferPool
=====================
*/

struct CBContainer_t
{
	D3D12_CPU_DESCRIPTOR_HANDLE hCPU = {};
	D3D12_GPU_VIRTUAL_ADDRESS pGPUMemAddr = {};
	AkU8* pSystemMemAddr = nullptr;
};

class FConstantBufferPool
{
public:
	FConstantBufferPool();
	~FConstantBufferPool();

	AkBool Initialize(ID3D12Device* pDevice, ConstantBufferProperty_t* pConstBufProp, AkU32 uMaxCBVNum);
	CBContainer_t* Alloc();
	void Reset();

private:
	void CleanUp();

private:
	CBContainer_t* _pCBContainers = nullptr;
	CONSTANT_BUFFER_TYPE _eCBType = {};
	ID3D12DescriptorHeap* _pCBVHeap = nullptr;
	ID3D12Resource* _pCBResource = nullptr;
	AkU8* _pSystemMemAddr = nullptr;
	AkU32 _uSizePerCBV = 0;
	AkU32 _uMaxCBVNum = 0;
	AkU32 _uAllocateCBVNum = 0;
};

