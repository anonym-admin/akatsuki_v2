#include "pch.h"
#include "Soldier.h"
#include "SkinnedModel.h"
#include "Animation.h"
#include "Transform.h"
#include "Controller.h"
#include "Collider.h"
#include "Gravity.h"
#include "RigidBody.h"
#include "Camera.h"
#include "Weapon.h"
#include "Sprite.h"
#include "BRS_74.h"

Soldier::Soldier()
{
	if (!Initialize())
	{
		__debugbreak();
	}
}

Soldier::Soldier(const wchar_t* wcFile)
{
	if (!Soldier::Initialize(wcFile))
	{
		__debugbreak();
	}
}

Soldier::~Soldier()
{
	if (_pSprite)
	{
		delete _pSprite;
		_pSprite = nullptr;
	}

	CleanUp();
}

AkBool Soldier::Initialize()
{
	Vector2 vMaxFrame = Vector2(4, 5);
	_pSprite = new Sprite(L"../../assets/particle/MuzzleFlash_4x5.dds", &vMaxFrame);

	// Create Model.
	AssetMeshDataContainer_t* pMeshDataContainer = GAssetManager->GetMeshData(L"soldier.mesh");
	Vector3 vAlbedo = Vector3(1.0f);
	Vector3 vEmissive = Vector3(0.0f);
	_pModel = CreateModel(pMeshDataContainer, &vAlbedo, 0.0f, 1.0f, &vEmissive, AK_TRUE);

	// Bind Animation.
	AssetAnimationContainer_t* pAnimContainer = GAssetManager->GetAnimation(L"soldier");
	BindAnimation(pAnimContainer->pAnim);
	memcpy(ANIM_CLIP, pAnimContainer->wcClipName, sizeof(wchar_t*) * COUNT);
	// _pAnimation->SetEndCallBack(ANIM_CLIP[PUNCHING_01], this, ::SetIdle);
	// _pAnimation->SetEndCallBack(ANIM_CLIP[PUNCHING_02], this, ::SetIdle);
	// _pAnimation->SetEndCallBack(ANIM_CLIP[RIFLE_FIRE], this, ::SetNextFire);
	// _pAnimation->SetEndCallBack(ANIM_CLIP[RUN_JUMP], this, ::SetIdle);
	// _pAnimation->SetEndCallBack(ANIM_CLIP[IDLE_JUMP], this, ::SetIdle);
	SetAnimation(IDLE);

	// Delete MeshData Resource.
	// GAssetManager->DeleteMeshData(ASSET_MESH_DATA_TYPE::SWATGUY);

	// Create Controller.
	_pController = CreateController();

	// Create Trnasform.
	_pTransform = CreateTransform();
	_pTransform->SetFront(0.0f, 0.0f, -1.0f);
	_pTransform->SetRight(-1.0f, 0.0f, 0.0f);

	// Create Collider.
	_pCollider = CreateCapsuleCollider(0.25f, 0.5f);

	// Create Camera.
	_pCamera = CreateCamera(2.0f, 0.5f);
	_pCamera->SetOwner(this);

	// Create Gravity
	_pGravity = CreateGravity();

	// Create Rigidbody
	_pRigidBody = CreateRigidBody();
	_pRigidBody->SetFrictionCoef(2.5f);
	_pRigidBody->SetMaxVeleocity(_fWalkSpeed);

	return AK_TRUE;
}

AkBool Soldier::Initialize(const wchar_t* wcFile)
{
	if (!Actor::Initialize(wcFile))
	{
		__debugbreak();
		return AK_FALSE;
	}

	// Create Controller.
	_pController = CreateController();

	// Bind Animation.
	AssetAnimationContainer_t* pAnimContainer = GAssetManager->GetAnimation(L"soldier");
	BindAnimation(pAnimContainer->pAnim);
	memcpy(ANIM_CLIP, pAnimContainer->wcClipName, sizeof(wchar_t*) * COUNT);
	SetAnimation(IDLE);

	// Create Camera.
	_pCamera = CreateCamera(2.0f, 0.5f);
	_pCamera->SetOwner(this);

	// Create Gravity
	_pGravity = CreateGravity();

	// Create Rigidbody
	_pRigidBody = CreateRigidBody();
	_pRigidBody->SetFrictionCoef(2.5f);
	_pRigidBody->SetMaxVeleocity(_fWalkSpeed);

	return AK_TRUE;
}

void Soldier::Update()
{
	// _pSprite->Update();

	if(KEY_DOWN(KEY_INPUT_P))
	{
		Vector3 vPos = Vector3(3.0f, 1.5f, 1025.0f);
		_pSprite->Play(&vPos);
	}

	UpdateMove();
	UpdateWeapon();
	UpdateFire();

	_pController->Update();
}

void Soldier::FinalUpdate()
{
	// _pGravity->Update();

	_pRigidBody->Update();

	_pTransform->Update();

	_pCollider->Update();

	_pCamera->Update();

	_pAnimation->Update();

	_pModel->UpdateWorldRow(_pTransform->GetWorldTransformAddr());

	FinalUpdateWeapon();
}

void Soldier::RenderDepthMap()
{
	_pModel->RenderDepthMap();
}

void Soldier::Render()
{
	// _pSprite->Render();

	// Render model.
	_pModel->Render();

	// Render collider.
	_pCollider->Render();
}

void Soldier::RenderShadowMaps()
{
	_pModel->RenderShadowMaps();
}

void Soldier::OnCollisionEnter(Collider* pOther)
{
	Actor* pOtherOwner = pOther->GetOwner();
	if (!wcscmp(pOtherOwner->Name, L"BRS_74"))
	{
		((Weapon*)pOtherOwner)->AttachOwner(this);
		SetWeapon((Weapon*)pOtherOwner);
	}
}

void Soldier::OnCollision(Collider* pOther)
{
	Actor* pOtherOwner = pOther->GetOwner();
	if (!wcscmp(pOtherOwner->Name, L"BRS_74"))
	{
		((Weapon*)pOtherOwner)->AttachOwner(this);
	}
}

void Soldier::OnCollisionExit(Collider* pOther)
{

}

void Soldier::ActionReaction(Collider* pOther)
{
	// 대상 충돌체가 육면체일 경우 대각선과 가로 세로의 반지름이 달라 진동하는 현상이 발생한다.
	Vector3 vOtherPos = pOther->GetTransform()->GetGlobalPosition();
	Vector3 vMyPos = _pCollider->GetTransform()->GetGlobalPosition();
	Vector3 vDir = vOtherPos - vMyPos;

	vDir.y = 0.0f;

	AkF32 fRa = pOther->Radius();
	AkF32 fRb = _pCollider->Radius();

	AkF32 fDist = vDir.Length();

	AkF32 fOverlapped = (fRa + fRb) - fDist;

	Vector3 vPos = _pTransform->GetPosition();
	vPos -= (vDir * fOverlapped) * 0.5f; // 진동현상을 줄이기 위해 0.5f 스케일 적용...
	_pTransform->SetPosition(&vPos);
}

void Soldier::CleanUp()
{
	UnBindAnimation();
}

void Soldier::SetIdle()
{
	Jumping = AK_FALSE;
	Attack = AK_FALSE;
	Fire = AK_FALSE;

	if (BindWeapon)
	{
		// SetAnimation(RIFLE_IDLE);
		((BRS_74*)_pWeapon)->Release();
	}
	else
	{
		SetAnimation(IDLE);
	}
}

void Soldier::SetNextPunching()
{
	//SetAnimation(PUNCHING_02);
}

void Soldier::SetNextFire()
{
	AkF32 fTime = 0.0f;
	while (fTime <= 1000.0f)
	{
		fTime += DT;
		//SetAnimation(RIFLE_FIRE);
	}
	
	SetIdle();
}

void Soldier::UpdateMove()
{
	if (Jumping || Attack)
		return;

	Vector3 vVelocity = _pRigidBody->GetVelocity();

	if (0.2f < vVelocity.Length() && vVelocity.Length() <= 2.8f)
	{
		// Walk.
		Vector3 vDir = vVelocity;
		vDir.Normalize();

		AkF32 fCosValue0 = vDir.Dot(_pTransform->Front());

		// [] > 60
		if (0.866025f < fCosValue0)
		{
			 //BindWeapon ? SetAnimation(RIFLE_F_WALK) : SetAnimation(F_WALK);
		}
		// 30 <= [] <= 60
		else if (0.5f <= fCosValue0 && fCosValue0 <= 0.866025f)
		{
			//AkF32 fCosValue1 = vDir.Dot(_pTransform->Right());
			//if (fCosValue1 >= 0.0f)
			//	SetAnimation(FR_WALK);
			//else
			//	SetAnimation(FL_WALK);
		}
		// -60 < [] < 60
		else if (-0.5f < fCosValue0 && fCosValue0 < 0.5f)
		{
			//AkF32 fCosValue1 = vDir.Dot(_pTransform->Right());
			//if (fCosValue1 >= 0.0f)
			//	SetAnimation(R_WALK);
			//else
			//	SetAnimation(L_WALK);
		}
		// -60 <= [] <= -30
		else if (-0.866025f <= fCosValue0 && fCosValue0 <= -0.5f)
		{
			//AkF32 fCosValue1 = vDir.Dot(_pTransform->Right());
			//if (fCosValue1 >= 0.0f)
			//	SetAnimation(BR_WALK);
			//else
			//	SetAnimation(BL_WALK);
		}
		// [] < -60
		else
		{
		}

		SetAnimation(WALK);

		// Return Walk Speed.
		_pRigidBody->SetMaxVeleocity(_fWalkSpeed);
	}
	else if (vVelocity.Length() > 3.0f)
	{
		//// Run.
		//BindWeapon ? SetAnimation(RIFLE_RUN) : SetAnimation(F_RUN);
	}
	else if (0.0f >= vVelocity.Length())
	{
		// Idle.
		//if (F_WALK <= AnimState && AnimState <= RIFLE_F_WALK)
		//	BindWeapon ? SetAnimation(RIFLE_IDLE) : SetAnimation(IDLE);

		//if (F_RUN == AnimState || RIFLE_RUN == AnimState)
		//	BindWeapon ? SetAnimation(RIFLE_IDLE) : SetAnimation(IDLE);

		SetAnimation(ANIM_STATE::IDLE);

		// Return Walk Speed.
		_pRigidBody->SetMaxVeleocity(_fWalkSpeed);
	}
}

void Soldier::UpdateWeapon()
{
	if (!BindWeapon)
		return;

	Vector3 vVelocity = _pRigidBody->GetVelocity();

	if (0.2f < vVelocity.Length() && vVelocity.Length() <= 2.8f)
	{
		_pWeapon->GetTransform()->SetRotation(DirectX::XMConvertToRadians(0.807f), DirectX::XMConvertToRadians(-2.340f), DirectX::XMConvertToRadians(142.585f));
		_pWeapon->GetTransform()->SetPosition(0.46f, 0.289f, 0.068f);
		_pWeapon->GetTransform()->SetScale(0.56f, 0.56f, 0.56f);
	}
	else if (vVelocity.Length() > 3.0f)
	{
		_pWeapon->GetTransform()->SetRotation(DirectX::XMConvertToRadians(-10.821f), DirectX::XMConvertToRadians(17.809f), DirectX::XMConvertToRadians(155.865f));
		_pWeapon->GetTransform()->SetPosition(0.437f, 0.293f, 0.053f);
		_pWeapon->GetTransform()->SetScale(0.56f, 0.56f, 0.56f);
	}
	else if (0.0f >= vVelocity.Length())
	{
		_pWeapon->GetTransform()->SetRotation(DirectX::XMConvertToRadians(7.079f), DirectX::XMConvertToRadians(14.038f), DirectX::XMConvertToRadians(149.514f));
		_pWeapon->GetTransform()->SetPosition(0.433f, 0.283f, 0.047f);
		_pWeapon->GetTransform()->SetScale(0.56f, 0.56f, 0.56f);
	}

	LeftHand = AK_TRUE;
	RightHand = AK_FALSE;
}

void Soldier::UpdateFire()
{
	static AkBool PrevFire = AK_FALSE;

	if (!Fire)
	{
		if (PrevFire)
		{
			SetWeaponRelativePosition();
		}

		// IDLE 상태 전환으로 인한 Fire 애니메이션의 부자연스러운 현상 방지.
		PrevFire = Fire;

		return;
	}

	SetWeaponRelativePosition();

	PrevFire = Fire;
}

void Soldier::FinalUpdateWeapon()
{
	if (!BindWeapon)
	{
		return;
	}

	Matrix* pFinalTransform = ((SkinnedModel*)_pModel)->GetAnimation()->GetBoneTransforms(); // ID 검색 기능 추가.

	// Gun model default matrix.
	if (RightHand)
	{
		Matrix mRightHandAnimTransfrom = pFinalTransform[34].Transpose();
		_mHandAnimTransform = mRightHandAnimTransfrom;
	}
	if (LeftHand)
	{
		Matrix mLeftHandAnimTransform = pFinalTransform[10].Transpose();
		_mHandAnimTransform = mLeftHandAnimTransform;
	}

	_mHandAnimTransform *= _pTransform->GetWorldTransform();

	_pWeapon->GetTransform()->SetParent(&_mHandAnimTransform);
}

void Soldier::SetWeaponRelativePosition()
{
	Vector3 vVelocity = _pRigidBody->GetVelocity();

	if (0.2f < vVelocity.Length() && vVelocity.Length() <= 2.8f)
	{
		_pWeapon->GetTransform()->SetRotation(DirectX::XMConvertToRadians(16.092f), DirectX::XMConvertToRadians(-21.279f), DirectX::XMConvertToRadians(-166.074f));
		_pWeapon->GetTransform()->SetPosition(0.410f, 0.301f, 0.014f);
	}
	else if (vVelocity.Length() > 3.0f)
	{
		_pWeapon->GetTransform()->SetRotation(DirectX::XMConvertToRadians(-23.338f), DirectX::XMConvertToRadians(-20.093f), DirectX::XMConvertToRadians(-167.379f));
		_pWeapon->GetTransform()->SetPosition(0.410f, 0.309f, 0.014f);
	}
	else if (0.0f >= vVelocity.Length())
	{
		_pWeapon->GetTransform()->SetRotation(DirectX::XMConvertToRadians(6.16f), DirectX::XMConvertToRadians(-2.817f), DirectX::XMConvertToRadians(178.809f));
		_pWeapon->GetTransform()->SetPosition(0.426f, 0.297f, 0.058f);
	}
}

void Soldier::SetAnimation(ANIM_STATE eState, AkF32 fSpeed)
{
	if (eState != AnimState)
	{
		AnimState = eState;
		_pAnimation->PlayClip(ANIM_CLIP[eState], ANIM_CLIP_STATE::LOOP, fSpeed, 0.2f);
	}
}

void SetIdle(Actor* pSwat)
{
	((Soldier*)pSwat)->SetIdle();
}

void SetNextPunching(Actor* pSwat)
{
	((Soldier*)pSwat)->SetNextPunching();
}

void SetNextFire(Actor* pSwat)
{
	((Soldier*)pSwat)->SetNextFire();
}
