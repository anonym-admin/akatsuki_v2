#include "pch.h"
#include "ConstantBufferPool.h"
#include "D3DUtils.h"

/*
=====================
FConstantBufferPool
=====================
*/

FConstantBufferPool::FConstantBufferPool()
{
}

FConstantBufferPool::~FConstantBufferPool()
{
	CleanUp();
}

AkBool FConstantBufferPool::Initialize(ID3D12Device* pDevice, ConstantBufferProperty_t* pConstBufProp, AkU32 uMaxCBVNum)
{
	_eCBType = pConstBufProp->eCBType;
	_uMaxCBVNum = uMaxCBVNum;
	_uSizePerCBV = FD3DUtils::AlignConstantBufferSize(pConstBufProp->uCBTypeSize);

	AkU32 uByteWidth = _uSizePerCBV * _uMaxCBVNum;

	// Create the constant buffer.
	if (FAILED(pDevice->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD), D3D12_HEAP_FLAG_NONE, &CD3DX12_RESOURCE_DESC::Buffer(uByteWidth), D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&_pCBResource))))
	{
		__debugbreak();
		return AK_FALSE;
	}

	// Create descriptor heap.
	D3D12_DESCRIPTOR_HEAP_DESC tHeapDesc = {};
	tHeapDesc.NumDescriptors = _uMaxCBVNum;
	tHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	tHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	if (FAILED(pDevice->CreateDescriptorHeap(&tHeapDesc, IID_PPV_ARGS(&_pCBVHeap))))
	{
		__debugbreak();
		return AK_FALSE;
	}
	CD3DX12_RANGE tWriteRange(0, 0);		// We do not intend to write from this resource on the CPU.
	_pCBResource->Map(0, &tWriteRange, reinterpret_cast<void**>(&_pSystemMemAddr));

	// Create CB containers.
	_pCBContainers = new CBContainer_t[_uMaxCBVNum];

	D3D12_CONSTANT_BUFFER_VIEW_DESC tCBVDesc = {};
	tCBVDesc.BufferLocation = _pCBResource->GetGPUVirtualAddress();
	tCBVDesc.SizeInBytes = _uSizePerCBV;

	AkU8* pSystemMemAddr = _pSystemMemAddr;
	CD3DX12_CPU_DESCRIPTOR_HANDLE hCBVHeap(_pCBVHeap->GetCPUDescriptorHandleForHeapStart());
	AkU32 uDescriptorSize = pDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	for (AkU32 i = 0; i < _uMaxCBVNum; i++)
	{
		pDevice->CreateConstantBufferView(&tCBVDesc, hCBVHeap);

		_pCBContainers[i].hCPU = hCBVHeap;
		_pCBContainers[i].pGPUMemAddr = tCBVDesc.BufferLocation;
		_pCBContainers[i].pSystemMemAddr = pSystemMemAddr;

		hCBVHeap.Offset(1, uDescriptorSize);
		tCBVDesc.BufferLocation += _uSizePerCBV;
		pSystemMemAddr += _uSizePerCBV;
	}

	return AK_TRUE;
}

CBContainer_t* FConstantBufferPool::Alloc()
{
	CBContainer_t* pCBContainer = nullptr;
	
	if (_uAllocateCBVNum >= _uMaxCBVNum)
	{
		__debugbreak();
		return pCBContainer;
	}

	pCBContainer = _pCBContainers + _uAllocateCBVNum;

	_uAllocateCBVNum++;

	return pCBContainer;
}

void FConstantBufferPool::Reset()
{
	_uAllocateCBVNum = 0;
}

void FConstantBufferPool::CleanUp()
{
	if (_pCBContainers)
	{
		delete[] _pCBContainers;
		_pCBContainers = nullptr;
	}
	if (_pCBResource)
	{
		_pCBResource->Release();
		_pCBResource = nullptr;
	}
	if (_pCBVHeap)
	{
		_pCBVHeap->Release();
		_pCBVHeap = nullptr;
	}
}
