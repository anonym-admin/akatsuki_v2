#include "pch.h"
#include "BaseObject.h"
#include "SkinnedModel.h"
#include "AssetManager.h"
#include "Animator.h"
#include "Transform.h"

BaseObject::BaseObject()
{
	_uInstanceCount = 1;
}

BaseObject::~BaseObject()
{
	CleanUp();
}

void BaseObject::SetScale(const Vector3* pScale)
{
	_pTransform->Scale = *pScale;
}

void BaseObject::SetRotation(const Vector3* pYawPitchRoll)
{
	_pTransform->Rotation = *pYawPitchRoll;
}

void BaseObject::SetPosition(const Vector3* pPos)
{
	_pTransform->Position = *pPos;
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
