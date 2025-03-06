#pragma once

/*
======================
FDescriptorPool
======================
*/

class FDescriptorPool
{
public:
	FDescriptorPool();
	~FDescriptorPool();

	AkBool Initialize(ID3D12Device* pDevice, AkU32 uMaxDescriptorNum);
	AkBool AllocDescriptorTable(D3D12_CPU_DESCRIPTOR_HANDLE* pCPUhandle, D3D12_GPU_DESCRIPTOR_HANDLE* pGPUhandle, AkU32 uDescriptorNum);
	void Reset();

	ID3D12DescriptorHeap* GetDescriptorHeap() { return _pDescriptorHeap; }
	AkU32 GetDescriptorTypeSize() { return _uDescriptorTypeSize; }

private:
	void CleanUp();

private:
	ID3D12Device* _pDevice = nullptr;
	ID3D12DescriptorHeap* _pDescriptorHeap = nullptr;
	D3D12_CPU_DESCRIPTOR_HANDLE _hCPU = {};
	D3D12_GPU_DESCRIPTOR_HANDLE _hGPU = {};
	AkU32 _uMaxDescriptorNum = 0;
	AkU32 _uAllocatedDescriptorNum = 0;
	AkU32 _uDescriptorTypeSize = 0;
};

