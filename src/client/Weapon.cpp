#include "pch.h"
#include "Weapon.h"
#include "Gravity.h"
#include "Transform.h"
#include "Application.h"
#include "Bullet.h"
#include "SceneInGame.h"

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

	// Create Gravity.
	_pGravity = CreateGravity();

	// Create Rigidbody.
	_pRigidBody = CreateRigidBody();

	return AK_TRUE;
}

Bullet* Weapon::CreateBullet()
{
	Bullet* pBullet = new Bullet(this);
	pBullet->tLink.pData = pBullet;
	pBullet->Name = L"Bullet";
	GSceneManager->GetCurrentScene()->AddGameObject(GAME_OBJECT_GROUP_TYPE::BULLET, pBullet);
	return pBullet;
}

void Weapon::DeleteBullet(Bullet* pBullet)
{
	if (pBullet)
	{
		delete pBullet;
	}
}

void Weapon::CleanUp()
{
	AkU32 uRefCount = _uInstanceCount - 1;
	if (uRefCount)
	{
		// 총알은 각 총마다 고유하게 가지고 있는 데이터.

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
