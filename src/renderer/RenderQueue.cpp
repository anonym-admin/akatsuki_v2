#include "pch.h"
#include "RenderQueue.h"
#include "Renderer.h"
#include "CommandListPool.h"
#include "SkinnedMeshObject.h"
#include "SpriteObject.h"
#include "SkyboxObject.h"
#include "LineObject.h"
#include "BillboardObject.h"
#include "TerrainObject.h"
#include "OceanObject.h"
#include "CloudObject.h"

/*
==============
RenderQueue
==============
*/

FRenderQueue::FRenderQueue()
{
}

FRenderQueue::~FRenderQueue()
{
    CleanUp();
}

AkBool FRenderQueue::Initialize(FRenderer* pRenderer, DWORD dwMaxItemNum)
{
	_pRenderer = pRenderer;
	_uMaxBufferSize = sizeof(RenderItem_t) * dwMaxItemNum;
	_pBuffer = (AkU8*)malloc(_uMaxBufferSize);
	memset(_pBuffer, 0, _uMaxBufferSize);

	return AK_TRUE;
}

AkBool FRenderQueue::Add(const RenderItem_t* pItem)
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

DWORD FRenderQueue::Process(DWORD uThreadIndex, FCommandListPool* pCmdListPool, ID3D12CommandQueue* pCmdQueue, DWORD dwProcessCountPerCommandList, D3D12_CPU_DESCRIPTOR_HANDLE hRTV, D3D12_CPU_DESCRIPTOR_HANDLE hDSV, const D3D12_VIEWPORT* pViewport, const D3D12_RECT* pScissorRect)
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
			case RENDER_ITEM_TYPE::RENDER_ITEM_TYPE_MESH_OBJ:
			{
				FBasicMeshObject* pMeshObj = (FBasicMeshObject*)pItem->pObjHandle;
				// Draw normal.
				if (pItem->tMeshObjParam.bDrawNormal)
				{
					pMeshObj->DrawNormal(uThreadIndex, pCmdList, &pItem->tMeshObjParam.mWorld);
				}
				else
				{
					pMeshObj->Draw(uThreadIndex, pCmdList, &pItem->tMeshObjParam.mWorld);
				}
			}
			break;
			case RENDER_ITEM_TYPE::RENDER_ITEM_TYPE_SKINNED_MESH_OBJ:
			{
				FSkinnedMeshObject* pMeshObj = (FSkinnedMeshObject*)pItem->pObjHandle;
				// Draw normal with skinned mesh obj.
				if (pItem->tSkinnedMeshObjParam.bDrawNormal)
				{
					pMeshObj->DrawNormal(uThreadIndex, pCmdList, &pItem->tSkinnedMeshObjParam.mWorld, pItem->tSkinnedMeshObjParam.pBonesTransform);
				}
				else
				{
					pMeshObj->Draw(uThreadIndex, pCmdList, &pItem->tSkinnedMeshObjParam.mWorld, pItem->tSkinnedMeshObjParam.pBonesTransform);
				}
			}
			break;
			case RENDER_ITEM_TYPE::RENDER_ITEM_TYPE_SKYBOX_OBJ:
			{
				FSkyboxObject* pSkyboxObj = (FSkyboxObject*)pItem->pObjHandle;
				TextureHandle_t* pEnvHDR = (TextureHandle_t*)pItem->tSkyboxObjParam.pEnvHDR;
				TextureHandle_t* pDiffuseHDR = (TextureHandle_t*)pItem->tSkyboxObjParam.pDiffuseHDR;
				TextureHandle_t* pSpecularHDR = (TextureHandle_t*)pItem->tSkyboxObjParam.pSpecularHDR;

				pSkyboxObj->Draw(uThreadIndex, pCmdList, &pItem->tSkyboxObjParam.mWorld, pEnvHDR, pDiffuseHDR, pSpecularHDR);
			}
			break;
			case RENDER_ITEM_TYPE::RENDER_ITEM_TYPE_LINE_OBJ:
			{
				FLineObject* pLineObj = (FLineObject*)pItem->pObjHandle;
				pLineObj->Draw(uThreadIndex, pCmdList, &pItem->tLineObjParam.mWorld);
			}
			break;
			case RENDER_ITEM_TYPE::RENDER_ITEM_TYPE_BILLBOARD:
			{
				FBillboardObjects* pBillboards = (FBillboardObjects*)pItem->pObjHandle;

				pBillboards->Draw(uThreadIndex, pCmdList, &pItem->tBillboardParam.mWorld);
			}
			break;
			case RENDER_ITEM_TYPE::RENDER_ITEM_TYPE_TERRAIN_OBJ:
			{
				FTerrainObject* pTerrainObj = (FTerrainObject*)pItem->pObjHandle;
				// Draw normal.
				if (pItem->tTerrianParam.bDrawNormal)
				{
					pTerrainObj->DrawNormal(uThreadIndex, pCmdList, &pItem->tTerrianParam.mWorld);
				}
				else
				{
					pTerrainObj->Draw(uThreadIndex, pCmdList, &pItem->tTerrianParam.mWorld, pItem->tTerrianParam.pBrush);
				}
			}
			break;
			case RENDER_ITEM_TYPE::RENDER_ITEM_TYPE_OCEAN:
			{
				FOceanObject* pOceanObj = (FOceanObject*)pItem->pObjHandle;
				pOceanObj->Draw(uThreadIndex, pCmdList, pItem->tOceanParam.fTime, &pItem->tOceanParam.mWorld);
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

void FRenderQueue::Reset()
{
	_uAllocatedSize = 0;
	_uReadBufferPos = 0;
}

void FRenderQueue::CleanUp()
{
	if (_pBuffer)
	{
		free(_pBuffer);
		_pBuffer = nullptr;
	}
}

const RenderItem_t* FRenderQueue::Dispatch()
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
