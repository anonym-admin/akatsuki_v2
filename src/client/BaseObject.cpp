#include "pch.h"
#include "BaseObject.h"
#include "SkinnedModel.h"
#include "BillboardModel.h"
#include "AssetManager.h"
#include "Transform.h"

/*
==========
BaseObject
==========
*/

BaseObject::BaseObject()
{
	_uInstanceCount = 1;
}

BaseObject::~BaseObject()
{
	CleanUp();
}

Model* BaseObject::CreateModel(AssetMeshDataContainer_t* pMeshDataContainer, const Vector3* pAlbedo, AkF32 fMetallic, AkF32 fRoughness, const Vector3* pEmissive, AkBool bIsSkinned)
{
	Model* pModel = nullptr;
	if (bIsSkinned)
	{
		pModel = new SkinnedModel(pMeshDataContainer, pAlbedo, fMetallic, fRoughness, pEmissive);
	}
	else
	{
		pModel = new Model(pMeshDataContainer, pAlbedo, fMetallic, fRoughness, pEmissive);
	}
	return pModel;
}

Model* BaseObject::CreateModel(MeshData_t* pMeshData, AkU32 uMeshDataNum, const Vector3* pAlbedo, AkF32 fMetallic, AkF32 fRoughness, const Vector3* pEmissive, AkBool bIsSkinned)
{
	Model* pModel = nullptr;
	if (bIsSkinned)
	{
		pModel = new SkinnedModel(pMeshData, uMeshDataNum, pAlbedo, fMetallic, fRoughness, pEmissive);
	}
	else
	{
		pModel = new Model(pMeshData, uMeshDataNum, pAlbedo, fMetallic, fRoughness, pEmissive);
	}
	return pModel;
}

Model* BaseObject::CreateBillboardModel(BillboardVertex_t* pBillboardVertices, AkU32 uPointNum)
{
	Model* pModel = new BillboardModels(pBillboardVertices, uPointNum);
	return pModel;
}

Transform* BaseObject::CreateTransform()
{
	Transform* pTransform = new Transform;
	return pTransform;
}

void BaseObject::CleanUp()
{
	AkU32 uRefCount = --_uInstanceCount;
	if (uRefCount)
	{
		return;
	}

	if (_pTransform)
	{
		delete _pTransform;
		_pTransform = nullptr;
	}
	if (_pModel)
	{
		delete _pModel;
		_pModel = nullptr;
	}
}
