#include "pch.h"
#include "Model.h"
#include "Animation.h"
#include "Application.h"
#include "GeometryGenerator.h"

/*
==========
Model
==========
*/

UModel::UModel()
{
}

UModel::~UModel()
{
	CleanUp();
}

AkBool UModel::Initialize(UApplication* pApp)
{
	_pApp = pApp;

	_pRenderer = _pApp->GetRenderer();

	return AK_TRUE;
}

void UModel::Render()
{
	if (_pMeshObj)
	{
		switch (_eType)
		{
		case MODEL_RENDER_OBJECT_TYPE::BASIC_MESH_OBJ:
			_pRenderer->RenderBasicMeshObject(_pMeshObj, &_pModelContextTable[_uModelCtxTableIndex]->mWorldRow);
			
			break;
		case MODEL_RENDER_OBJECT_TYPE::SKINNED_MESH_OBJ:
			_pRenderer->RenderSkinnedMeshObject(_pMeshObj, &_pModelContextTable[_uModelCtxTableIndex]->mWorldRow, _pModelContextTable[_uModelCtxTableIndex]->pAnimation->GetFinalTransforms());
			break;
		default:
			__debugbreak();
			break;
		}
	}
}

void UModel::RenderNormal()
{
	if (_pMeshObj)
	{
		switch (_eType)
		{
		case MODEL_RENDER_OBJECT_TYPE::BASIC_MESH_OBJ:
			_pRenderer->RenderNormalOfBasicMeshObject(_pMeshObj, &_pModelContextTable[_uModelCtxTableIndex]->mWorldRow);
			break;
		case MODEL_RENDER_OBJECT_TYPE::SKINNED_MESH_OBJ:
			_pRenderer->RenderNormalOfSkinnedMeshObject(_pMeshObj, &_pModelContextTable[_uModelCtxTableIndex]->mWorldRow, _pModelContextTable[_uModelCtxTableIndex]->pAnimation->GetFinalTransforms());
			break;
		default:
			__debugbreak();
			break;
		}
	}
}

void UModel::RenderShadow()
{
	if (_pMeshObj)
	{
		switch (_eType)
		{
		case MODEL_RENDER_OBJECT_TYPE::BASIC_MESH_OBJ:
			_pRenderer->RenderShadowOfBasicMeshObject(_pMeshObj, &_pModelContextTable[_uModelCtxTableIndex]->mWorldRow);
			break;
		case MODEL_RENDER_OBJECT_TYPE::SKINNED_MESH_OBJ:
			_pRenderer->RenderShadowOfSkinnedMeshObject(_pMeshObj, &_pModelContextTable[_uModelCtxTableIndex]->mWorldRow, _pModelContextTable[_uModelCtxTableIndex]->pAnimation->GetFinalTransforms());
			break;
		default:
			__debugbreak();
			break;
		}
	}
}

void UModel::Release()
{
	AkU32 uRefCount = --_uRefCount;
	if (!uRefCount)
	{
		delete this;
	}
}

AkU32 UModel::AddRef()
{
	AkU32 uRefCount = _uRefCount++;
	return uRefCount;
}

void UModel::SetWorldMatrix(AkU32 uModelCtxTableIndex, const Matrix* pWorld)
{
	_pModelContextTable[uModelCtxTableIndex]->mWorldRow = *pWorld;
}

void UModel::SetRenderObject(IMeshObject* pMeshObj, MODEL_RENDER_OBJECT_TYPE eType)
{
	_pMeshObj = pMeshObj;
	_eType = eType;
}

const Matrix* UModel::GetWorldMatrix()
{
	return &_pModelContextTable[_uModelCtxTableIndex]->mWorldRow;
}

AkBool UModel::PlayAnimation(const AkF32 fDeltaTime, const wchar_t* wcClipName, AkBool bInPlace)
{
	AkBool bIsEnd = AK_FALSE;

	ModelContext_t* pCurModelCtx = GetCurrentModelContext();
	UAnimation* pAnimation = pCurModelCtx->pAnimation;

	return pAnimation->PlayAnimation(fDeltaTime, wcClipName, bInPlace);
}

AkBool UModel::CreateStaticMeshObject()
{
	if (_pMeshObj)
	{
		__debugbreak();
		return AK_FALSE;
	}

	_pMeshObj = _pRenderer->CreateBasicMeshObject();

	return AK_TRUE;;
}

AkBool UModel::CreateSkinnedMeshObject()
{
	if (_pMeshObj)
	{
		__debugbreak();
		return AK_FALSE;
	}

	_pMeshObj = _pRenderer->CreateSkinnedMeshObject();

	return AK_TRUE;
}

void UModel::CreateContextTable(Matrix* pWorldRow, UAnimation* pAnim)
{
	if (_uModelCtxNum >= MAX_MODEL_CONTEXT_NUM)
	{
		__debugbreak();
		return;
	}

	ModelContext_t* pContext = new ModelContext_t;
	pContext->mWorldRow = *pWorldRow;
	pContext->pAnimation = pAnim;
	if (pAnim)
	{
		pAnim->AddRef();
	}

	_pModelContextTable[_uModelCtxNum] = pContext;
	_uModelCtxNum++;
}

void UModel::CreateMeshBuffersForDraw(MeshData_t* pMeshData, AkU32 uMeshDataNum)
{
	if (pMeshData && uMeshDataNum > 0)
	{
		_pMeshObj->CreateMeshBuffers(pMeshData, uMeshDataNum);
	}
	else
	{
		__debugbreak();
	}
}

void UModel::DestroyContextTable()
{
	for (AkU32 i = 0; i < _uModelCtxNum; i++)
	{
		if (_pModelContextTable[i])
		{
			if (_pModelContextTable[i]->pAnimation)
			{
				_pModelContextTable[i]->pAnimation->Release();
				_pModelContextTable[i]->pAnimation = nullptr;
			}

			delete _pModelContextTable[i];
			_pModelContextTable[i] = nullptr;
		}
	}
}

void UModel::DestroyMeshObject()
{
	if (_pMeshObj)
	{
		_pMeshObj->Release();
		_pMeshObj = nullptr;
	}
}

void UModel::CleanUp()
{
	DestroyMeshObject();

	DestroyContextTable();
}













