#include "pch.h"
#include "DescriptorAllocator.h"

/*
=====================
FDesriptorAllocator
=====================
*/

FDescriptorAllocator::FDescriptorAllocator()
{
}

FDescriptorAllocator::~FDescriptorAllocator()
{
	CleanUp();
}

AkBool FDescriptorAllocator::Initialize(ID3D12Device* pDevice, AkU32 uMaxNum)
{
	_pDevice = pDevice;

	D3D12_DESCRIPTOR_HEAP_DESC tHeapDesc = {};
	tHeapDesc.NumDescriptors = uMaxNum;
	tHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	tHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

	if (FAILED(_pDevice->CreateDescriptorHeap(&tHeapDesc, IID_PPV_ARGS(&_pDescriptorHeap))))
	{
		__debugbreak();
		return AK_FALSE;
	}

	_uDescriptorSize = _pDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	_pIndexGenerator = CreateIndexGenerator(uMaxNum);

	return AK_TRUE;
}

AkBool FDescriptorAllocator::AllocDescriptorHandle(D3D12_CPU_DESCRIPTOR_HANDLE* pCPU)
{
	AkU32 uIndex = AllocIndex(_pIndexGenerator);

	if (-1 == uIndex)
	{
		__debugbreak();
		return AK_FALSE;
	}

	CD3DX12_CPU_DESCRIPTOR_HANDLE hCPU(_pDescriptorHeap->GetCPUDescriptorHandleForHeapStart(), uIndex, _uDescriptorSize);
	*pCPU = hCPU;

	return AK_TRUE;
}

void FDescriptorAllocator::FreeDescriptorHandle(D3D12_CPU_DESCRIPTOR_HANDLE hCPU)
{
	D3D12_CPU_DESCRIPTOR_HANDLE hBase = _pDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	if (hBase.ptr > hCPU.ptr)
	{
		__debugbreak();
	}
	
	AkU32 uIndex = static_cast<AkU32>(hCPU.ptr - hBase.ptr) / _uDescriptorSize;
	FreeIndex(_pIndexGenerator, uIndex);
}

D3D12_GPU_DESCRIPTOR_HANDLE FDescriptorAllocator::GetGPUHandleFromCPUHandle(D3D12_CPU_DESCRIPTOR_HANDLE hCPU)
{
	D3D12_CPU_DESCRIPTOR_HANDLE hBase = _pDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	if (hBase.ptr > hCPU.ptr)
	{
		__debugbreak();
	}
	AkU32 uIndex = static_cast<AkU32>(hCPU.ptr - hBase.ptr) / _uDescriptorSize;
	CD3DX12_GPU_DESCRIPTOR_HANDLE hGPU(_pDescriptorHeap->GetGPUDescriptorHandleForHeapStart(), uIndex, _uDescriptorSize);

	return hGPU;
}

void FDescriptorAllocator::CleanUp()
{
	if (_pIndexGenerator)
	{
		DestroyIndexGenerator(_pIndexGenerator);
		_pIndexGenerator = nullptr;
	}
	if (_pDescriptorHeap)
	{
		_pDescriptorHeap->Release();
		_pDescriptorHeap = nullptr;
	}
}
