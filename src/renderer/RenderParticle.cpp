#include "pch.h"
#include "RenderParticle.h"
#include "RenderQueue.h"
#include "Renderer.h"
#include "CommandListPool.h"
#include "Particle.h"

/*
===================
Render Particle
===================
*/

FRenderParticle::FRenderParticle()
{
}

FRenderParticle::~FRenderParticle()
{
    CleanUp();
}

AkBool FRenderParticle::Initialize(FRenderer* pRenderer, DWORD dwMaxItemNum)
{
    _pRenderer = pRenderer;
    _uMaxBufferSize = sizeof(RenderItem_t) * dwMaxItemNum;
    _pBuffer = (AkU8*)malloc(_uMaxBufferSize);
    memset(_pBuffer, 0, _uMaxBufferSize);

    return AK_TRUE;
}

AkBool FRenderParticle::Add(const RenderItem_t* pItem)
{
	AkBool bResult = AK_FALSE;
	if (_uAllocatedSize + sizeof(RenderItem_t) > _uMaxBufferSize)
	{
		__debugbreak();
		return bResult;
	}

	AkU8* pDest = _pBuffer + _uAllocatedSize;
	memcpy(pDest, pItem, sizeof(RenderItem_t));
	_uAllocatedSize += sizeof(RenderItem_t);
	_uItemCount++;

	bResult = AK_TRUE;

	return bResult;
}

DWORD FRenderParticle::Process(DWORD uThreadIndex, FCommandListPool* pCmdListPool, ID3D12CommandQueue* pCmdQueue, DWORD dwProcessCountPerCommandList, D3D12_CPU_DESCRIPTOR_HANDLE hRTV, D3D12_CPU_DESCRIPTOR_HANDLE hDSV, const D3D12_VIEWPORT* pViewport, const D3D12_RECT* pScissorRect)
{
	ID3D12Device* pDevice = _pRenderer->GetDevice();

	ID3D12GraphicsCommandList* ppCmdLists[64] = {};
	AkU32 uCommandListCount = 0;

	ID3D12GraphicsCommandList* pCmdList = nullptr;
	DWORD dwProcessedCount = 0;
	DWORD dwProcessedCountPerCommandList = 0;
	const RenderItem_t* pItem = nullptr;
	while (pItem = Dispatch())
	{
		pCmdList = pCmdListPool->GetCurrentCmdList();
		pCmdList->RSSetViewports(1, pViewport);
		pCmdList->RSSetScissorRects(1, pScissorRect);
		pCmdList->OMSetRenderTargets(1, &hRTV, AK_FALSE, &hDSV);

		switch (pItem->eItemType)
		{
			case RENDER_ITEM_TYPE::RENDER_ITEM_TYPE_PARTICLE:
			{
				FParticle* pParticle = (FParticle*)pItem->pObjHandle;
				
				if(pItem->tParticleParam.pWorldRow)
				{
					// Draw Spark
					pParticle->Draw(uThreadIndex, pCmdList, pItem->tParticleParam.pWorldRow, pItem->tParticleParam.pDBHandle, pItem->tParticleParam.uParticleNum, pItem->tParticleParam.fTime, pItem->tParticleParam.fDuration, pItem->tParticleParam.pStartSize, pItem->tParticleParam.pStartDirection, pItem->tParticleParam.fSizeOverLifeTime, pItem->tParticleParam.pRotOverLifeTime, pItem->tParticleParam.pTotalColor, pItem->tParticleParam.pColorOverLifeTime);
				}
				else
				{
					// Draw Sprite
					pParticle->Draw(uThreadIndex, pCmdList, pItem->tParticleParam.pDBHandle, pItem->tParticleParam.pMaxFrame, pItem->tParticleParam.pCurFrame);
				}
			}
			break;
			default:
			{
				__debugbreak();
			}
			break;
		}
		dwProcessedCount++;
		dwProcessedCountPerCommandList++;
		if (dwProcessedCountPerCommandList > dwProcessCountPerCommandList)
		{
			//pCommandListPool->CloseAndExecute(pCommandQueue);
			pCmdListPool->Close();
			ppCmdLists[uCommandListCount] = pCmdList;
			uCommandListCount++;
			pCmdList = nullptr;
			dwProcessedCountPerCommandList = 0;
		}
	}
	// 남은 렌더링아이템 처리
	if (dwProcessedCountPerCommandList)
	{
		//pCommandListPool->CloseAndExecute(pCommandQueue);
		pCmdListPool->Close();
		ppCmdLists[uCommandListCount] = pCmdList;
		uCommandListCount++;
		pCmdList = nullptr;
		dwProcessedCountPerCommandList = 0;
	}
	if (uCommandListCount)
	{
		pCmdQueue->ExecuteCommandLists(uCommandListCount, (ID3D12CommandList**)ppCmdLists);
	}
	_uItemCount = 0;
	return dwProcessedCount;
}

void FRenderParticle::Reset()
{
	_uAllocatedSize = 0;
	_uReadBufferPos = 0;
}

void FRenderParticle::CleanUp()
{
	if (_pBuffer)
	{
		free(_pBuffer);
		_pBuffer = nullptr;
	}
}

const RenderItem_t* FRenderParticle::Dispatch()
{
	const RenderItem_t* pItem = nullptr;
	if (_uReadBufferPos + sizeof(RenderItem_t) > _uAllocatedSize)
	{
		return pItem;
	}

	pItem = (const RenderItem_t*)(_pBuffer + _uReadBufferPos);
	_uReadBufferPos += sizeof(RenderItem_t);

	return pItem;
}
