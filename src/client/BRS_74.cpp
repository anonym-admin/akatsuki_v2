#include "pch.h"
#include "BRS_74.h"
#include "Application.h"
#include "Bullet.h"
#include "Sprite.h"
#include "Casing.h"

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

BRS_74::BRS_74(const wchar_t* wcScript)
	: Weapon()
{
	if (!Initialize(wcScript))
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
	AssetMeshDataContainer_t* pMeshDataContainer = GAssetManager->GetMeshData(L"brs-74.mesh");
	Vector3 vAlbedo = Vector3(1.0f);
	Vector3 vEmissive = Vector3(0.0f);
	_pModel = CreateModel(pMeshDataContainer, &vAlbedo, 0.0f, 1.0f, &vEmissive, AK_FALSE);

	// Create Collider.
	Vector3 vMin = Vector3(0.0f);
	Vector3 vMax = Vector3(0.0f);
	CalcColliderMinMax(pMeshDataContainer->pMeshData, pMeshDataContainer->uMeshDataNum, &vMin, &vMax);
	_pCollider = CreateBoxCollider(&vMin, &vMax);

	// Create Muzzle Effect.
	Vector2 vMaxFrame = Vector2(4.0f, 5.0f);
	_pMuzzleEffect = new Sprite(L"../../assets/particle/MuzzleFlash_4x5.dds", &vMaxFrame);

	// Create Fire Sound.
	_pFireSound = GSoundManager->LoadSound("../../assets/audio/AKS74U_Fire0.wav");

	return AK_TRUE;
}

AkBool BRS_74::Initialize(const wchar_t* wcScript)
{
	if (!Actor::Initialize(wcScript))
	{
		__debugbreak();
		return AK_FALSE;
	}

	// Create Muzzle Effect.
	Vector2 vMaxFrame = Vector2(4.0f, 5.0f);
	_pMuzzleEffect = new Sprite(L"../../assets/particle/MuzzleFlash_4x5.dds", &vMaxFrame);

	// Create Fire Sound.
	_pFireSound = GSoundManager->LoadSound("../../assets/audio/AKS74U_Fire0.wav");

	// Create Casing Bounce Sound.
	_pCasingBounceSound = GSoundManager->LoadSound("../../assets/audio/BulletCasingBounce.wav");

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

	_pCullingCollider->Update();

	_pModel->UpdateWorldRow(&_pTransform->GetWorldTransform());

	// Final Update Casing.
	for (AkU32 i = 0; i < _uCasingCount; i++)
	{
		_pCasings[i]->FinalUpdate();
	}
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

	// Render Update Casing.
	for (AkU32 i = 0; i < _uCasingCount; i++)
	{
		_pCasings[i]->Render();
	}
}

void BRS_74::RenderShadowMaps()
{
	_pModel->RenderShadowMaps();
}

void BRS_74::RenderDepthMap()
{
	_pModel->RenderDepthMap();
}

void BRS_74::OnCollisionEnter(Collider* pOther)
{
	Actor* pOtherOwner = pOther->GetOwner();
	if (!wcscmp(pOtherOwner->Name, L"soldier"))
	{
		pOtherOwner->BindWeapon = AK_TRUE;
	}
}

void BRS_74::OnCollision(Collider* pOther)
{
	Actor* pOtherOwner = pOther->GetOwner();
	if (!wcscmp(pOtherOwner->Name, L"soldier"))
	{
		pOtherOwner->BindWeapon = AK_TRUE;
	}
}

void BRS_74::OnCollisionExit(Collider* pOther)
{
}

void BRS_74::Fire()
{
	static AkF32 fAccTime = 0.0f;

	_bFire = AK_TRUE;

	Vector3 vPos = _pTransform->GetGlobalPosition();
	Vector3 vOffset = -_pTransform->Front() * 0.275f + Vector3(0.0f, 0.075f, 0.0f);

	vPos += vOffset;

	_pMuzzleEffect->Play(&vPos);

	fAccTime += DT;

	// 해당 시간이 지날때까지 사운드는 플레이 되지 않는다.
	if (fAccTime < 0.1f)
	{
		return;
	}

	// Casing 생성.
	if (_uCasingCount < MAX_CASING_COUNT)
	{
		_pCasings[_uCasingCount] = CreateCasing();
		_uCasingCount++;
	}

	fAccTime = 0.0f;

	_pFireSound->PlayOnce();
}

void BRS_74::Release()
{
	_bFire = AK_FALSE;
	_bFirst = AK_TRUE;

	_pCasingBounceSound->PlayOnce();
}

void BRS_74::CleanUp()
{
	if (_pMuzzleEffect)
	{
		delete _pMuzzleEffect;
		_pMuzzleEffect = nullptr;
	}

	DestroyAllCasing();
}

Casing* BRS_74::CreateCasing()
{
	Casing* pCasing = new Casing;
	Quaternion qQuat = _pTransform->GetGlobalRotation();
	Vector3 vYPR = Vector3(0.0f);
	QuternionToEuler(qQuat, vYPR);
	Vector3 vPos = _pTransform->GetGlobalPosition();
	Vector3 vRight = -_pTransform->Right();
	vRight *= Random(1.0f, 2.0f);
	vRight.y += 2.0f;
	pCasing->GetTransform()->SetRotation(&vYPR);
	pCasing->GetTransform()->SetPosition(&vPos);
	pCasing->GetRigidBody()->SetVelocity(&vRight);
	return pCasing;
}

void BRS_74::DestroyCasing(Casing* pCasing)
{
	if (pCasing)
		delete pCasing;
}

void BRS_74::DestroyAllCasing()
{
	for (AkU32 i = 0; i < _uCasingCount; i++)
	{
		delete _pCasings[i];
		_pCasings[i] = nullptr;
	}
	_uCasingCount = 0;
}

BRS_74* BRS_74::Clone()
{
	return new BRS_74();
}


