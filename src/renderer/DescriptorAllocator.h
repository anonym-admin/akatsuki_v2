#pragma once

/*
=====================
FDesriptorAllocator
=====================
*/

class FDescriptorAllocator
{
public:
	FDescriptorAllocator();
	~FDescriptorAllocator();

	AkBool Initialize(ID3D12Device* pDevice, AkU32 uMaxNum);
	AkBool AllocDescriptorHandle(D3D12_CPU_DESCRIPTOR_HANDLE* pCPU);
	void FreeDescriptorHandle(D3D12_CPU_DESCRIPTOR_HANDLE hCPU);

	D3D12_GPU_DESCRIPTOR_HANDLE GetGPUHandleFromCPUHandle(D3D12_CPU_DESCRIPTOR_HANDLE hCPU);

private:
	void CleanUp();

private:
	ID3D12Device* _pDevice = nullptr;
	ID3D12DescriptorHeap* _pDescriptorHeap = nullptr;
	IndexGenerator_t* _pIndexGenerator = nullptr;
	AkU32 _uDescriptorSize = 0;
};

