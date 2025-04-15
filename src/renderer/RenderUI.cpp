#include "pch.h"
#include "RenderUI.h"
#include "Renderer.h"
#include "RenderQueue.h"
#include "CommandListPool.h"
#include "SpriteObject.h"

/*
==============
UI Render
==============
*/

FRenderUI::FRenderUI()
{
}

FRenderUI::~FRenderUI()
{
	CleanUp();
}

AkBool FRenderUI::Initialize(FRenderer* pRenderer, DWORD dwMaxItemNum)
{
	_pRenderer = pRenderer;
	_uMaxBufferSize = sizeof(RenderItem_t) * dwMaxItemNum;
	_pBuffer = (AkU8*)malloc(_uMaxBufferSize);
	memset(_pBuffer, 0, _uMaxBufferSize);
	return AK_TRUE;
}

AkBool FRenderUI::Add(const RenderItem_t* pItem)
{
	AkBool bResult = AK_FALSE;
	if (_uAllocatedSize + sizeof(RenderItem_t) > _uMaxBufferSize)
	{
		__debugbreak();
		return bResult;
	}

	// 정렬을 이곳에서?? 아니면 Client 단에서 시도?

	AkU8* pDest = _pBuffer + _uAllocatedSize;
	memcpy(pDest, pItem, sizeof(RenderItem_t));
	_uAllocatedSize += sizeof(RenderItem_t);
	_uItemCount++;

	bResult = AK_TRUE;

	return bResult;
}

DWORD FRenderUI::Process(DWORD uThreadIndex, FCommandListPool* pCmdListPool, ID3D12CommandQueue* pCmdQueue, DWORD dwProcessCountPerCommandList, D3D12_CPU_DESCRIPTOR_HANDLE hRTV, D3D12_CPU_DESCRIPTOR_HANDLE hDSV, const D3D12_VIEWPORT* pViewport, const D3D12_RECT* pScissorRect)
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
			case RENDER_ITEM_TYPE::RENDER_ITEM_TYPE_SPRITE_OBJ:
			{
				FSpriteObject* pSpriteObj = (FSpriteObject*)pItem->pObjHandle;
				TextureHandle_t* pTexureHandle = (TextureHandle_t*)pItem->tSpriteObjParam.pTexHandle;
				AkF32 fZ = pItem->tSpriteObjParam.fZ;

				if (pTexureHandle)
				{
					Vector2 vPos = { (AkF32)pItem->tSpriteObjParam.iPosX, (AkF32)pItem->tSpriteObjParam.iPosY };
					Vector2 vScale = { pItem->tSpriteObjParam.fScaleX, pItem->tSpriteObjParam.fScaleY };

					const RECT* pRect = nullptr;
					if (pItem->tSpriteObjParam.bUseRect)
					{
						pRect = &pItem->tSpriteObjParam.tRect;
					}

					if (pTexureHandle->pUploadBuffer)
					{
						if (pTexureHandle->bUpdated)
						{
							FD3DUtils::UpdateTexture(pDevice, pCmdList, pTexureHandle->pTextureResource, pTexureHandle->pUploadBuffer);
						}
						else
						{
							// For Debugging.
							AkI32 a = 0;
						}
						pTexureHandle->bUpdated = FALSE;
					}
					pSpriteObj->DrawWithTex(uThreadIndex, pCmdList, &vPos, &vScale, pRect, fZ, pTexureHandle, pItem->tSpriteObjParam.bUseBlend);
				}
				else
				{
					FSpriteObject* pSpriteObj = (FSpriteObject*)pItem->pObjHandle;
					Vector2 vPos = { (AkF32)pItem->tSpriteObjParam.iPosX, (AkF32)pItem->tSpriteObjParam.iPosY };
					Vector2 vScale = { pItem->tSpriteObjParam.fScaleX, pItem->tSpriteObjParam.fScaleY };

					pSpriteObj->Draw(uThreadIndex, pCmdList, &vPos, &vScale, fZ, pItem->tSpriteObjParam.bUseBlend);
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

void FRenderUI::Reset()
{
	_uAllocatedSize = 0;
	_uReadBufferPos = 0;
}

void FRenderUI::CleanUp()
{
	if (_pBuffer)
	{
		free(_pBuffer);
		_pBuffer = nullptr;
	}
}

const RenderItem_t* FRenderUI::Dispatch()
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
