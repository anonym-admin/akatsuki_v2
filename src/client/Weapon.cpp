#include "pch.h"
#include "Weapon.h"
#include "Player.h"
#include "Collider.h"
#include "RigidBody.h"
#include "Gravity.h"
#include "Model.h"
#include "BRS_74_Model.h"
#include "Application.h"

/*
==========
Weapon
==========
*/

UWeapon::UWeapon()
{
}

UWeapon::~UWeapon()
{
	CleanUp();
}

AkBool UWeapon::Initialize(UApplication* pApp, const Vector3* pExtent, const Vector3* pCenter)
{
	if (!UActor::Initialize(pApp))
	{
		__debugbreak();
		return AK_FALSE;
	}

	// Create Collider.
	UCollider* pCollider = CreateCollider();

	// Create Rigid Body.
	URigidBody* pRigidBody = CreateRigidBody();
	pRigidBody->SetFrictionCoef(0.0f);

	// Create Gravity.
	UGravity* pGravity = CreateGravity();

	return AK_TRUE;
}

void UWeapon::Update(const AkF32 fDeltaTime)
{
}

void UWeapon::FinalUpdate(const AkF32 fDeltaTime)
{
	UCollider* pCollider = GetCollider();

	pCollider->Update(fDeltaTime);

	UpdateModelTransform();
}

void UWeapon::Render()
{
	UApplication* pApp = GetApp();
	UCollider* pCollider = GetCollider();

	// Render Model.
	RenderModel();

	// Render Normal.
	if (IsDrawNoraml())
	{
		RenderNormal();
	}

	if (pApp->EnableEditor())
	{
		pCollider->Render();
	}
}

void UWeapon::OnCollision(UCollider* pOther)
{
}

void UWeapon::OnCollisionEnter(UCollider* pOther)
{
}

void UWeapon::OnCollisionExit(UCollider* pOther)
{
}

void UWeapon::SetOwnerRotationY(AkF32 fRot)
{
	_fOwnerRotY = fRot;
}

void UWeapon::SetRelativeRotationX(AkF32 fRot)
{
	_vRelativeRot.x = fRot;
}

void UWeapon::SetRelativeRotationY(AkF32 fRot)
{
	_vRelativeRot.y = fRot;
}

void UWeapon::SetRelativeRotationZ(AkF32 fRot)
{
	_vRelativeRot.z = fRot;
}

void UWeapon::SetRelativePosition(AkF32 fX, AkF32 fY, AkF32 fZ)
{
	_vRelativePos = Vector3(fX, fY, fZ);

	Vector3 vOwnerPos = _pOwner->GetPosition();
}

void UWeapon::SetAnimTransform(Matrix* pMat)
{
	_mAnimTransform = *pMat;
}

void UWeapon::AttachOwner(UPlayer* pOwner)
{
	_pOwner = pOwner;
}

void UWeapon::UpdateModelTransform()
{
	using namespace DirectX;

	MODEL_CONTEXT_INDEX eModelCtxIndex = GetModelContextIndex();
	UModel* pModel = (UModel*)GetModel(eModelCtxIndex);

	Matrix mWorld = Matrix::CreateScale(_vScale) *
					Matrix::CreateRotationX(XMConvertToRadians(_vRelativeRot.x)) *
					Matrix::CreateRotationY(XMConvertToRadians(_vRelativeRot.y)) *
					Matrix::CreateRotationZ(XMConvertToRadians(_vRelativeRot.z)) *
					Matrix::CreateTranslation(_vRelativePos);

	mWorld = mWorld *_mAnimTransform;
	if (_pOwner)
	{
		mWorld = mWorld * Matrix::CreateRotationX(_pOwner->GetRotationX()) * 
						  Matrix::CreateRotationY(_fOwnerRotY) * 
						  Matrix::CreateRotationZ(_pOwner->GetRotationZ()) *
						  Matrix::CreateTranslation(_pOwner->GetPosition());
	}

	pModel->SetWorldMatrix((AkU32)eModelCtxIndex, &mWorld);
}

void UWeapon::CleanUp()
{
	DestroyGravity();

	DesteoyRigidBody();

	DestroyCollider();
}