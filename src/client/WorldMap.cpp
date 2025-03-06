#include "pch.h"
#include "WorldMap.h"
#include "Application.h"
#include "GeometryGenerator.h"
#include "Collider.h"
#include "KDTree.h"
#include "CollisionManager.h"

UWorldMapContainer::UWorldMapContainer()
{
}

UWorldMapContainer::~UWorldMapContainer()
{
	CleanUp();
}

AkBool UWorldMapContainer::Initialize(UApplication* pApp)
{
	if (!UActor::Initialize(pApp))
	{
		__debugbreak();
		return AK_FALSE;
	}

	_pApp = pApp;
	_pRenderer = pApp->GetRenderer();

	return AK_TRUE;
}

void UWorldMapContainer::Update(const AkF32 fDeltaTime)
{
}

void UWorldMapContainer::FinalUpdate(const AkF32 fDeltaTime)
{
}

void UWorldMapContainer::RenderShadow()
{
	for (AkU32 i = 0; i < _uMsshObjListNum; i++)
	{
		for (AkU32 j = 0; j < _uMeshObjNum[i]; j++)
		{
			_pRenderer->RenderShadowOfBasicMeshObject(_pMeshObj[i], _pWorldRows[i] + j);
		}
	}
}

void UWorldMapContainer::Render()
{
	for (AkU32 i = 0; i < _uMsshObjListNum; i++)
	{
		for (AkU32 j = 0; j < _uMeshObjNum[i]; j++)
		{
			_pRenderer->RenderBasicMeshObject(_pMeshObj[i], _pWorldRows[i] + j);
		}
	}

	if (_bUseKDTree && _bDrawKDTreeNode)
	{
		RenderKDTreeNode(_pKDTreeNode, _pRenderer, _pKDTreeBoxObj);
	}
}

void UWorldMapContainer::OnCollision(UCollider* pOther)
{
}

void UWorldMapContainer::OnCollisionEnter(UCollider* pOther)
{
}

void UWorldMapContainer::OnCollisionExit(UCollider* pOther)
{

}

void UWorldMapContainer::BindMeshData(MeshData_t* pMeshData, AkU32 uMeshDataNum)
{
	_ppMeshDataChunk[_uMeshDataChunkNum] = pMeshData;
	_pMeshDataNum[_uMeshDataChunkNum] = uMeshDataNum;

	_pDrawTable[_uMeshDataChunkNum] = AK_TRUE;

	_uMeshDataChunkNum++;
}

void UWorldMapContainer::BindMeshObj(IMeshObject* pMeshObj, AkU32 uMeshObjNum, const Matrix* pWorldRows)
{
	if (_uMsshObjListNum >= MAX_MESH_OBJ_LIST_COUNT)
	{
		__debugbreak();
	}

	_pMeshObj[_uMsshObjListNum] = pMeshObj;
	_uMeshObjNum[_uMsshObjListNum] = uMeshObjNum;
	_pWorldRows[_uMsshObjListNum] = pWorldRows;

	_uMsshObjListNum++;
}

void UWorldMapContainer::Build(AkBool bUseKDTree)
{
	if (0 == _uMeshDataChunkNum)
	{
		__debugbreak();
	}

	_bUseKDTree = bUseKDTree;

	AkU32 uTriCount = 0;
	for (AkU32 i = 0; i < _uMeshDataChunkNum; i++)
	{
		for (AkU32 j = 0; j < _pMeshDataNum[i]; j++)
		{
			for (AkU32 k = 0; k < _ppMeshDataChunk[i][j].uIndicesNum; k += 3)
			{
				uTriCount++;
			}
		}
	}

	AkTriangle_t* pTriList = new AkTriangle_t[uTriCount];
	_ppColliderList = new UCollider * [uTriCount];
	AkU32 uTriIndex = 0;
	for (AkU32 i = 0; i < _uMeshDataChunkNum; i++)
	{
		for (AkU32 j = 0; j < _pMeshDataNum[i]; j++)
		{
			for (AkU32 k = 0; k < _ppMeshDataChunk[i][j].uIndicesNum; k += 3)
			{
				AkU32 i0 = _ppMeshDataChunk[i][j].pIndices[k];
				AkU32 i1 = _ppMeshDataChunk[i][j].pIndices[k + 1];
				AkU32 i2 = _ppMeshDataChunk[i][j].pIndices[k + 2];

				pTriList[uTriIndex].vP[0] = _ppMeshDataChunk[i][j].pVertices[i0].vPosition;
				pTriList[uTriIndex].vP[1] = _ppMeshDataChunk[i][j].pVertices[i1].vPosition;
				pTriList[uTriIndex].vP[2] = _ppMeshDataChunk[i][j].pVertices[i2].vPosition;

				_ppColliderList[uTriIndex] = CreateCollider();
				_ppColliderList[uTriIndex]->Initialize(this, _pRenderer);
				_ppColliderList[uTriIndex]->CreateTriangle(&pTriList[uTriIndex].vP[0], &pTriList[uTriIndex].vP[1], &pTriList[uTriIndex].vP[2]);

				_ppColliderList[uTriIndex]->_pDraw = _pDrawTable + i;

				if (_ppColliderList[uTriIndex]->GetTriangle()->vNormal.Length() <= 1e-5f)
				{
					delete _ppColliderList[uTriIndex];
					_ppColliderList[uTriIndex] = nullptr;
					continue;
				}

				if (0 == uTriIndex)
				{
					_uInitColliderID = _ppColliderList[uTriIndex]->GetID();
				}

				uTriIndex++;
			}
		}
	}

	if (uTriIndex > uTriCount)
	{
		__debugbreak();
	}

	uTriCount = uTriIndex;
	_uColliderNum = uTriCount;

	if (bUseKDTree)
	{
		_pKDTreeNode = BuildKDTree(this, _ppColliderList, uTriCount, 0);

		AkU32 uMeshDataNum = 0;
		MeshData_t* pBoxMeshData = UGeometryGenerator::MakeCube(&uMeshDataNum, 0.5f);
		Vector3 vAlbedo = Vector3(0.0f);
		Vector3 vEmissive = Vector3(1.0f, 0.0f, 0.0f);
		_pKDTreeBoxObj = _pRenderer->CreateBasicMeshObject();
		_pKDTreeBoxObj->CreateMeshBuffers(pBoxMeshData, uMeshDataNum);
		_pKDTreeBoxObj->EnableWireFrame();
		_pKDTreeBoxObj->UpdateMaterialBuffers(&vAlbedo, 0.0f, 0.0f, &vEmissive);

		UGeometryGenerator::DestroyGeometry(pBoxMeshData, uMeshDataNum);
	}

	if (pTriList)
	{
		delete[] pTriList;
		pTriList = nullptr;
	}
}

void UWorldMapContainer::CleanUp()
{
	if (_pKDTreeBoxObj)
	{
		_pKDTreeBoxObj->Release();
		_pKDTreeBoxObj = nullptr;
	}

	// Destroy KDTree
	if (_pKDTreeNode)
	{
		DestroyKDTree(_pKDTreeNode);
	}

	for (AkU32 i = 0; i < _uMeshDataChunkNum; i++)
	{
		if (_ppMeshDataChunk[i])
		{
			for (AkU32 j = 0; j < _pMeshDataNum[i]; j++)
			{
				if (_ppMeshDataChunk[i][j].pVertices)
				{
					delete[] _ppMeshDataChunk[i][j].pVertices;
					_ppMeshDataChunk[i][j].pVertices = nullptr;
				}
				if (_ppMeshDataChunk[i][j].pIndices)
				{
					delete[] _ppMeshDataChunk[i][j].pIndices;
					_ppMeshDataChunk[i][j].pIndices = nullptr;
				}
			}

			delete[] _ppMeshDataChunk[i];
			_ppMeshDataChunk[i] = nullptr;
		}
	}

	if (_ppColliderList)
	{
		for (AkU32 i = 0; i < _uColliderNum; i++)
		{
			if (_ppColliderList[i])
			{
				delete _ppColliderList[i];
				_ppColliderList[i] = nullptr;
			}
		}

		delete[] _ppColliderList;
		_ppColliderList = nullptr;
	}
}

