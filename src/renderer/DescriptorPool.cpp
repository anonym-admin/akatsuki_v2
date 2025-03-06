#include "pch.h"
#include "DescriptorPool.h"

/*
======================
FDescriptorPool
======================
*/

FDescriptorPool::FDescriptorPool()
{
}

FDescriptorPool::~FDescriptorPool()
{
	CleanUp();
}

AkBool FDescriptorPool::Initialize(ID3D12Device* pDevice, AkU32 uMaxDescriptorNum)
{
	_pDevice = pDevice;

	_uMaxDescriptorNum = uMaxDescriptorNum;
	_uDescriptorTypeSize = _pDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	// Create descriptor heap.
	D3D12_DESCRIPTOR_HEAP_DESC tCommonHeapDesc = {};
	tCommonHeapDesc.NumDescriptors = _uMaxDescriptorNum;
	tCommonHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	tCommonHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	if (FAILED(_pDevice->CreateDescriptorHeap(&tCommonHeapDesc, IID_PPV_ARGS(&_pDescriptorHeap))))
	{
		__debugbreak();
		return AK_FALSE;
	}

	_hCPU = _pDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	_hGPU = _pDescriptorHeap->GetGPUDescriptorHandleForHeapStart();

	return AK_TRUE;
}

AkBool FDescriptorPool::AllocDescriptorTable(D3D12_CPU_DESCRIPTOR_HANDLE* pCPUhandle, D3D12_GPU_DESCRIPTOR_HANDLE* pGPUhandle, AkU32 uDescriptorNum)
{
	if (_uAllocatedDescriptorNum + uDescriptorNum > _uMaxDescriptorNum)
	{
		__debugbreak();
		return AK_FALSE;
	}

	*pCPUhandle = CD3DX12_CPU_DESCRIPTOR_HANDLE(_hCPU, _uAllocatedDescriptorNum, _uDescriptorTypeSize);
	*pGPUhandle = CD3DX12_GPU_DESCRIPTOR_HANDLE(_hGPU, _uAllocatedDescriptorNum, _uDescriptorTypeSize);
	_uAllocatedDescriptorNum += uDescriptorNum;

	return AK_TRUE;
}

void FDescriptorPool::Reset()
{
	_uAllocatedDescriptorNum = 0;
}

void FDescriptorPool::CleanUp()
{
	if (_pDescriptorHeap)
	{
		_pDescriptorHeap->Release();
		_pDescriptorHeap = nullptr;
	}
}
