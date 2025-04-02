#include "pch.h"
#include "RenderDepthMap.h"
#include "RenderQueue.h"
#include "Renderer.h"
#include "CommandListPool.h"
#include "BasicMeshObject.h"
#include "SkinnedMeshObject.h"
#include "BillboardObject.h"
#include "TerrainObject.h"

FRenderDepthMap::FRenderDepthMap()
{
}

FRenderDepthMap::~FRenderDepthMap()
{
	CleanUp();
}

AkBool FRenderDepthMap::Initialize(FRenderer* pRenderer, DWORD dwMaxItemNum)
{
	_pRenderer = pRenderer;
	_uMaxBufferSize = sizeof(RenderItem_t) * dwMaxItemNum;
	_pBuffer = (AkU8*)malloc(_uMaxBufferSize);
	memset(_pBuffer, 0, _uMaxBufferSize);

	return AK_TRUE;
}

AkBool FRenderDepthMap::Add(const RenderItem_t* pItem)
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

DWORD FRenderDepthMap::Process(DWORD uThreadIndex, FCommandListPool* pCmdListPool, ID3D12CommandQueue* pCmdQueue, DWORD dwProcessCountPerCommandList, D3D12_CPU_DESCRIPTOR_HANDLE hDSV, const D3D12_VIEWPORT* pViewport, const D3D12_RECT* pScissorRect)
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
		pCmdList->OMSetRenderTargets(0, nullptr, AK_FALSE, &hDSV);

		switch (pItem->eItemType)
		{
		case RENDER_ITEM_TYPE::RENDER_ITEM_TYPE_MESH_OBJ_DEPTH_MAP:
		{
			FBasicMeshObject* pMeshObj = (FBasicMeshObject*)pItem->pObjHandle;
			pMeshObj->DrawDepthMap(uThreadIndex, pCmdList, &pItem->tMeshObjParam.mWorld);
		}
		break;
		case RENDER_ITEM_TYPE::RENDER_ITEM_TYPE_SKINNED_MESH_OBJ_DEPTH_MAP:
		{
			FSkinnedMeshObject* pMeshObj = (FSkinnedMeshObject*)pItem->pObjHandle;
			pMeshObj->DrawDepthMap(uThreadIndex, pCmdList, &pItem->tSkinnedMeshObjParam.mWorld, pItem->tSkinnedMeshObjParam.pBonesTransform);
		}
		break;
		case RENDER_ITEM_TYPE::RENDER_ITEM_TYPE_BILLBOARD_DEPTH_MAP:
		{
			FBillboardObjects* pBillboard = (FBillboardObjects*)pItem->pObjHandle;
			pBillboard->DrawDepthMap(uThreadIndex, pCmdList, &pItem->tBillboardParam.mWorld);
		}
		break;
		case RENDER_ITEM_TYPE::RENDER_ITEM_TYPE_TERRAIN_OBJ_DEPTH_MAP:
		{
			FTerrainObject* pTerrain = (FTerrainObject*)pItem->pObjHandle;
			// pTerrain->DrawDepthMap(uThreadIndex, pCmdList, &pItem->tTerrianParam.mWorld);
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

void FRenderDepthMap::Reset()
{
	_uAllocatedSize = 0;
	_uReadBufferPos = 0;
}

void FRenderDepthMap::CleanUp()
{
	if (_pBuffer)
	{
		free(_pBuffer);
		_pBuffer = nullptr;
	}
}

const RenderItem_t* FRenderDepthMap::Dispatch()
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
