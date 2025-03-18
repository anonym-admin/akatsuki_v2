#include "pch.h"
#include "BRS_74.h"
#include "Application.h"
#include "Bullet.h"
#include "Sprite.h"

/*
=============
BRS_74
=============
*/

BRS_74::BRS_74()
{
	if (!Initialize())
	{
		__debugbreak();
	}
}

BRS_74::~BRS_74()
{
	CleanUp();
}

AkBool BRS_74::Initialize()
{
	// Create Model.
	AssetMeshDataContainer_t* pMeshDataContainer = GAssetManager->GetMeshDataContainer(ASSET_MESH_DATA_TYPE::BRS_74);
	Vector3 vAlbedo = Vector3(1.0f);
	Vector3 vEmissive = Vector3(0.0f);
	_pModel = CreateModel(pMeshDataContainer, &vAlbedo, 0.0f, 1.0f, &vEmissive, AK_FALSE);

	// Create Collider.
	Vector3 vMin = Vector3(0.0f);
	Vector3 vMax = Vector3(0.0f);
	CalcColliderMinMax(pMeshDataContainer->pMeshData, pMeshDataContainer->uMeshDataNum, &vMin, &vMax);
	_pCollider = CreateBoxCollider(&vMin, &vMax);

	GAssetManager->DeleteMeshData(ASSET_MESH_DATA_TYPE::BRS_74);

	// Create Muzzle Effect.
	Vector2 vMaxFrame = Vector2(4.0f, 5.0f);
	_pMuzzleEffect = new Sprite(L"../../assets/particle/MuzzleFlash_4x5.dds", &vMaxFrame);

	// Create Fire Sound.
	_pFireSound = GSoundManager->LoadSound("../../assets/audio/AKS74U_Fire0.wav");

	return AK_TRUE;
}

void BRS_74::Update()
{
	if (_bFire)
	{
		_pMuzzleEffect->Update();
	}
}

void BRS_74::FinalUpdate()
{
	_pTransform->Update();

	if (!_pOwner)
	{
		_pCollider->Update();
	}

	Matrix mFinalWorldTransform = _pOwner ? _pTransform->GetWorldTransform() * _pOwner->GetTransform()->GetWorldTransform() : _pTransform->GetWorldTransform();

	_pTransform->SetWorldTransform(mFinalWorldTransform);

	_pModel->UpdateWorldRow(&_pTransform->GetWorldTransform());
}

void BRS_74::Render()
{
	_pModel->Render();

	if (!_pOwner)
	{
		_pCollider->Render();
	}

	if (_bFire)
	{
		_pMuzzleEffect->Render();
	}
}

void BRS_74::RenderShadow()
{
	_pModel->RenderShadow();
}

void BRS_74::OnCollisionEnter(Collider* pOther)
{
	Actor* pOtherOwner = pOther->GetOwner();
	if (!wcscmp(pOtherOwner->Name, L"Swat"))
	{
		pOtherOwner->BindWeapon = AK_TRUE;
	}
}

void BRS_74::OnCollision(Collider* pOther)
{
	Actor* pOtherOwner = pOther->GetOwner();
	if (!wcscmp(pOtherOwner->Name, L"Swat"))
	{
		pOtherOwner->BindWeapon = AK_TRUE;
	}
}

void BRS_74::OnCollisionExit(Collider* pOther)
{
}

BRS_74* BRS_74::Clone()
{
	Spawn::Clone();
	return new BRS_74();
}

void BRS_74::Fire()
{
	_bFire = AK_TRUE;

	Vector3 vPos = _pTransform->GetPosition();
	Vector3 vOffset = -_pTransform->Front() * 0.5f;

	vPos += vOffset;

	_pMuzzleEffect->Play(&vPos);

	_pFireSound->PlayOnce();
}

void BRS_74::Release()
{
	_bFire = AK_FALSE;
}

void BRS_74::CleanUp()
{
	if (_pMuzzleEffect)
	{
		delete _pMuzzleEffect;
		_pMuzzleEffect = nullptr;
	}
}


