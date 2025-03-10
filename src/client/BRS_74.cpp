#include "pch.h"
#include "BRS_74.h"
#include "Application.h"
#include "EventManager.h"
#include "AssetManager.h"
#include "Collider.h"
#include "Transform.h"
#include "Model.h"
#include "Bullet.h"

/*
=============
BRS_74
=============
*/

UBRS_74::UBRS_74()
{
	if (!Initialize())
	{
		__debugbreak();
	}
}

AkBool UBRS_74::Initialize()
{
	// Create Model.
	AssetMeshDataContainer_t* pMeshDataContainer = GAssetManager->GetMeshDataContainer(ASSET_MESH_DATA_TYPE::BRS_74);
	Vector3 vAlbedo = Vector3(1.0f);
	Vector3 vEmissive = Vector3(0.0f);
	_pModel = CreateModel(pMeshDataContainer, &vAlbedo, 0.0f, 1.0f, &vEmissive, AK_FALSE);
<<<<<<< HEAD
=======
	GAssetManager->DeleteMeshData(ASSET_MESH_DATA_TYPE::BRS_74);
>>>>>>> f2e096c130c4bec98199dd6ba2311eb016150af2

	// Create Collider.
	Vector3 vMin = Vector3(0.0f);
	Vector3 vMax = Vector3(0.0f);
	CalcColliderMinMax(pMeshDataContainer->pMeshData, pMeshDataContainer->uMeshDataNum, &vMin, &vMax);
	_pCollider = CreateBoxCollider(&vMin, &vMax);

	GAssetManager->DeleteMeshData(ASSET_MESH_DATA_TYPE::BRS_74);

	//// Create Bullet.
	//for (AkU32 i = 0; i < 150; i++)
	//{
	//	Bullet* pBullet = CreateBullet();
	//	pBullet->tFireLink.pData = pBullet;
	//	LL_PushBack(&_pBulletHead, &_pBulletTail, &pBullet->tFireLink);
	//}

	return AK_TRUE;
}

void UBRS_74::Update()
{
	if (_pOwner && _pOwner->Fire)
	{
		List_t* pCur = _pBulletHead;
		while (pCur != nullptr && _uMaxFireBullet > 0)
		{
			Vector3 vDir = _pOwner->GetTransform()->Front();
			Vector3 vVelocity = vDir;
			vVelocity.Normalize();
			vVelocity *= 100.0f;

			Bullet* pBullet = (Bullet*)pCur->pData;

			List_t* pNext = pCur->pNext;

			pBullet->GetRigidBody()->SetVelocity(&vVelocity);

			LL_Delete(&_pBulletHead, &_pBulletTail, pCur);

			pCur = pNext;

			_uMaxFireBullet--;
		}
	}
	else
	{
		_uMaxFireBullet = 5;
	}
}

void UBRS_74::FinalUpdate()
{
	_pTransform->Update();

	if (!_pOwner)
		_pCollider->Update();

	Matrix mFinalWorldTransform = _pOwner ? _pTransform->GetWorldTransform() * _pOwner->GetTransform()->GetWorldTransform() : _pTransform->GetWorldTransform();

	_pTransform->SetWorldTransform(mFinalWorldTransform);

	_pModel->UpdateWorldRow(&_pTransform->GetWorldTransform());
}

void UBRS_74::Render()
{
	_pModel->Render();

	if (!_pOwner)
		_pCollider->Render();
}

void UBRS_74::RenderShadow()
{
	_pModel->RenderShadow();
}

void UBRS_74::OnCollisionEnter(Collider* pOther)
{
	Actor* pOtherOwner = pOther->GetOwner();
	if (!wcscmp(pOtherOwner->Name, L"Swat"))
	{
		pOtherOwner->BindWeapon = AK_TRUE;
	}
}

void UBRS_74::OnCollision(Collider* pOther)
{
	Actor* pOtherOwner = pOther->GetOwner();
	if (!wcscmp(pOtherOwner->Name, L"Swat"))
	{
		pOtherOwner->BindWeapon = AK_TRUE;
	}
}

void UBRS_74::OnCollisionExit(Collider* pOther)
{
}

UBRS_74* UBRS_74::Clone()
{
	Spawn::Clone();
	return new UBRS_74();
}


