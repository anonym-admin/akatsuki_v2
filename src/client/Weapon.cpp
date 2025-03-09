#include "pch.h"
#include "Weapon.h"
#include "Gravity.h"
#include "Transform.h"
#include "Application.h"

/*
==========
Weapon
==========
*/

Weapon::Weapon()
{
	if (!Initialize())
	{
		__debugbreak();
	}
}

Weapon::~Weapon()
{
	CleanUp();
}

AkBool Weapon::Initialize()
{
	// Create Transform.
	_pTransform = CreateTransform();

	// Create Collider.
	// _pCollider = CreateCollider();

	// Create Gravity.
	_pGravity = CreateGravity();

	// Create Rigidbody.
	_pRigidBody = CreateRigidBody();

	return AK_TRUE;
}

void Weapon::SetOwnerRotationY(AkF32 fRot)
{
	_fOwnerRotY = fRot;
}

void Weapon::SetRelativeRotationX(AkF32 fRot)
{
	_vRelativeRot.x = fRot;
}

void Weapon::SetRelativeRotationY(AkF32 fRot)
{
	_vRelativeRot.y = fRot;
}

void Weapon::SetRelativeRotationZ(AkF32 fRot)
{
	_vRelativeRot.z = fRot;
}

void Weapon::SetRelativePosition(AkF32 fX, AkF32 fY, AkF32 fZ)
{
	_vRelativePos = Vector3(fX, fY, fZ);
}

void Weapon::SetRelativeRotation(const Vector3* pYawPitchRoll)
{
	_vRelativeRot = *pYawPitchRoll;
}

void Weapon::SetRelativePosition(const Vector3* pPos)
{
	_vRelativePos = *pPos;
}

void Weapon::SetAnimTransform(Matrix* pMat)
{
	_mAnimTransform = *pMat;
}

void Weapon::CleanUp()
{
	AkU32 uRefCount = _uInstanceCount - 1;
	if (uRefCount)
	{
		return;
	}
}

//void UWeapon::UpdateModelTransform()
//{
//	using namespace DirectX;
//
//	MODEL_CONTEXT_INDEX eModelCtxIndex = GetModelContextIndex();
//	UModel* pModel = (UModel*)GetModel(eModelCtxIndex);
//
//	Matrix mWorld = Matrix::CreateScale(_vScale) *
//					Matrix::CreateRotationX(XMConvertToRadians(_vRelativeRot.x)) *
//					Matrix::CreateRotationY(XMConvertToRadians(_vRelativeRot.y)) *
//					Matrix::CreateRotationZ(XMConvertToRadians(_vRelativeRot.z)) *
//					Matrix::CreateTranslation(_vRelativePos);
//
//	mWorld = mWorld *_mAnimTransform;
//	if (_pOwner)
//	{
//		mWorld = mWorld * Matrix::CreateRotationX(_pOwner->GetRotationX()) * 
//						  Matrix::CreateRotationY(_fOwnerRotY) * 
//						  Matrix::CreateRotationZ(_pOwner->GetRotationZ()) *
//						  Matrix::CreateTranslation(_pOwner->GetPosition());
//	}
//
//	pModel->SetWorldMatrix((AkU32)eModelCtxIndex, &mWorld);
//}
