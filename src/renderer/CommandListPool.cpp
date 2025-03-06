#include "pch.h"
#include "CommandListPool.h"

/*
====================
FCommandListPool
====================
*/

FCommandListPool::FCommandListPool()
{
}

FCommandListPool::~FCommandListPool()
{
	CleanUp();
}

AkBool FCommandListPool::Initialize(ID3D12Device* pDevice, D3D12_COMMAND_LIST_TYPE eCmdListType, AkU32 uMaxCmdListNum)
{
	if (uMaxCmdListNum < 2)
	{
		__debugbreak();
		return AK_FALSE;
	}

	_pDevice = pDevice;
	_eCmdListTpye = eCmdListType;
	_uMaxCmdListNum = uMaxCmdListNum;

	return AK_TRUE;
}

ID3D12GraphicsCommandList* FCommandListPool::GetCurrentCmdList()
{
	ID3D12GraphicsCommandList* pCmdList = nullptr;
	if (!_pCurCmdList)
	{
		_pCurCmdList = Alloc();
		if (!_pCurCmdList)
		{
			__debugbreak();
			return pCmdList;
		}
	}

	pCmdList = _pCurCmdList->pCmdList;

	return pCmdList;
}

void FCommandListPool::Close()
{
	if (!_pCurCmdList)
	{
		__debugbreak();
	}
	if (_pCurCmdList->bClosed)
	{
		__debugbreak();
	}
	if (FAILED(_pCurCmdList->pCmdList->Close()))
	{
		__debugbreak();
	}

	_pCurCmdList->bClosed = AK_TRUE;
	_pCurCmdList = nullptr;
}

void FCommandListPool::CloseAndExecute(ID3D12CommandQueue* pCmdQueue)
{
	if (!_pCurCmdList)
	{
		__debugbreak();
	}
	if (_pCurCmdList->bClosed)
	{
		__debugbreak();
	}
	if (FAILED(_pCurCmdList->pCmdList->Close()))
	{
		__debugbreak();
	}

	_pCurCmdList->bClosed = AK_TRUE;

	pCmdQueue->ExecuteCommandLists(1, reinterpret_cast<ID3D12CommandList**>(&_pCurCmdList->pCmdList));

	_pCurCmdList = nullptr;
}

AkBool FCommandListPool::AddCmdList()
{
	CmdListContainer_t* pCmdListContainer = nullptr;

	ID3D12CommandAllocator* pCmdAllocator = nullptr;
	ID3D12GraphicsCommandList* pCmdList = nullptr;

	if (_uTotalCmdListNum >= _uMaxCmdListNum)
	{
#ifdef _DEBUG
		__debugbreak();
#endif
		return AK_FALSE;
	}

	if (FAILED(_pDevice->CreateCommandAllocator(_eCmdListTpye, IID_PPV_ARGS(&pCmdAllocator))))
	{
#ifdef _DEBUG
		__debugbreak();
#endif
		return AK_FALSE;
	}

	if (FAILED(_pDevice->CreateCommandList(0, _eCmdListTpye, pCmdAllocator, nullptr, IID_PPV_ARGS(&pCmdList))))
	{
#ifdef _DEBUG
		__debugbreak();
#endif
		pCmdAllocator->Release();
		pCmdAllocator = nullptr;

		return AK_FALSE;
	}

	pCmdListContainer = new CmdListContainer_t;
	memset(pCmdListContainer, 0, sizeof(CmdListContainer_t));
	pCmdListContainer->tLink.pData = pCmdListContainer;
	pCmdListContainer->pCmdAllocator = pCmdAllocator;
	pCmdListContainer->pCmdList = pCmdList;

	_uTotalCmdListNum++;

	LL_PushBack(&_pAvailableCmdListHead, &_pAvailableCmdListTail, &pCmdListContainer->tLink);
	_uAvailableCmdListNum++;

	return AK_TRUE;
}

CmdListContainer_t* FCommandListPool::Alloc()
{
	CmdListContainer_t* pCmdList = nullptr;

	if (!_pAvailableCmdListHead)
	{
		if (!AddCmdList())
		{
			__debugbreak();
			return pCmdList;
		}
	}

	pCmdList = reinterpret_cast<CmdListContainer_t*>(_pAvailableCmdListHead->pData);

	LL_Delete(&_pAvailableCmdListHead, &_pAvailableCmdListTail, &pCmdList->tLink);
	_uAvailableCmdListNum--;

	LL_PushBack(&_pAllocatedCmdListHead, &_pAllocatedCmdListTail, &pCmdList->tLink);
	_uAllocatedCmdListNum++;

	return pCmdList;
}

void FCommandListPool::Reset()
{
	while (_pAllocatedCmdListHead)
	{
		CmdListContainer_t* pCmdListContainer = reinterpret_cast<CmdListContainer_t*>(_pAllocatedCmdListHead->pData);

		if (FAILED(pCmdListContainer->pCmdAllocator->Reset()))
		{
			__debugbreak();
		}
		if (FAILED(pCmdListContainer->pCmdList->Reset(pCmdListContainer->pCmdAllocator, nullptr)))
		{
			__debugbreak();
		}

		pCmdListContainer->bClosed = AK_FALSE;

		LL_Delete(&_pAllocatedCmdListHead, &_pAllocatedCmdListTail, &pCmdListContainer->tLink);
		_uAllocatedCmdListNum--;

		LL_PushBack(&_pAvailableCmdListHead, &_pAvailableCmdListTail, &pCmdListContainer->tLink);
		_uAvailableCmdListNum++;
	}
}

void FCommandListPool::CleanUp()
{
	Reset();

	while (_pAvailableCmdListHead)
	{
		CmdListContainer_t* pCmdListContainer = reinterpret_cast<CmdListContainer_t*>(_pAvailableCmdListHead->pData);
		pCmdListContainer->pCmdList->Release();
		pCmdListContainer->pCmdList = nullptr;

		pCmdListContainer->pCmdAllocator->Release();
		pCmdListContainer->pCmdAllocator = nullptr;

		_uTotalCmdListNum--;

		LL_Delete(&_pAvailableCmdListHead, &_pAvailableCmdListTail, &pCmdListContainer->tLink);
		_uAvailableCmdListNum--;

		delete pCmdListContainer;
	}
}

