#include "pch.h"
#include "Swat.h"
#include "SkinnedModel.h"
#include "Animation.h"
#include "Transform.h"
#include "Controller.h"
#include "Collider.h"
#include "Gravity.h"
#include "RigidBody.h"
#include "Camera.h"
#include "Weapon.h"

Swat::Swat()
{
	if (!Initialize())
	{
		__debugbreak();
	}
}

Swat::~Swat()
{
	CleanUp();
}

AkBool Swat::Initialize()
{
	// Create Model.
	AssetMeshDataContainer_t* pMeshDataContainer = GAssetManager->GetMeshDataContainer(ASSET_MESH_DATA_TYPE::SWATGUY);
	Vector3 vAlbedo = Vector3(1.0f);
	Vector3 vEmissive = Vector3(0.0f);
	_pModel = CreateModel(pMeshDataContainer, &vAlbedo, 0.0f, 1.0f, &vEmissive, AK_TRUE);

	// Bind Animation.
	AssetAnimationContainer_t* pAnimContainer = GAssetManager->GetAnimationContainer(ASSET_ANIM_TYPE::SWATGUY);
	BindAnimation(pAnimContainer->pAnim);
	memcpy(ANIM_CLIP, pAnimContainer->wcClipName, sizeof(wchar_t*) * COUNT);
	_pAnimation->SetEndCallBack(ANIM_CLIP[PUNCHING_01], this, ::SetIdle);
	_pAnimation->SetEndCallBack(ANIM_CLIP[PUNCHING_02], this, ::SetIdle);
	_pAnimation->SetEndCallBack(ANIM_CLIP[RUN_JUMP], this, ::SetIdle);
	_pAnimation->SetEndCallBack(ANIM_CLIP[IDLE_JUMP], this, ::SetIdle);
	SetAnimation(IDLE);

	// Delete MeshData Resource.
	GAssetManager->DeleteMeshData(ASSET_MESH_DATA_TYPE::SWATGUY);

	// Create Controller.
	_pController = CreateController();

	// Create Trnasform.
	_pTransform = CreateTransform();
	_pTransform->SetFront(0.0f, 0.0f, -1.0f); // 모델이 생성될때 나를 바라보는 방향으로 만들어지기 때문에 Front 의 방향을 재설정 필요!!
	_pTransform->SetRight(-1.0f, 0.0f, 0.0f); // 위 이유와 같음

	// Create Collider.
	_pCollider = CreateCapsuleCollider(0.25f, 0.5f);

	// Create Camera.
	Vector3 vCamPos = Vector3(0.0f, 0.5f, -2.0f);
	_pCamera = CreateCamera(&vCamPos);
	_pCamera->Mode = CAMERA_MODE::FOLLOW;
	_pCamera->SetOwner(this);

	// Create Gravity
	_pGravity = CreateGravity();

	// Create Rigidbody
	_pRigidBody = CreateRigidBody();
	_pRigidBody->SetFrictionCoef(2.5f);
	_pRigidBody->SetMaxVeleocity(_fWalkSpeed);

	return AK_TRUE;
}

void Swat::Update()
{
	UpdateMove();
	UpdateWeapon();
	UpdateFire();

	_pController->Update();
}

void Swat::FinalUpdate()
{
	// _pGravity->Update();

	_pRigidBody->Update();

	_pTransform->Update();

	_pCollider->Update();

	_pCollider->Update();

	_pCamera->Update();

	_pAnimation->Update();

	_pModel->UpdateWorldRow(_pTransform->GetWorldTransformAddr());

	FinalUpdateWeapon();
}

void Swat::Render()
{
	// Render model.
	_pModel->Render();

	// Render collider.
	_pCollider->Render();
}

void Swat::RenderShadow()
{
	_pModel->RenderShadow();
}

void Swat::OnCollisionEnter(Collider* pOther)
{
	printf("Swat Collision\n");


	/*Actor* pOwner = pOther->GetOwner();
	const wchar_t* wcName = pOwner->Name;

	if (!wcscmp(L"Map", wcName))
	{
		AkTriangle_t* pTri = pOther->GetTriangle();
		AkF32 fNdotY = pTri->vNormal.Dot(Vector3(0.0f, 1.0f, 0.0f));
		if (0.0f <= fNdotY - 1.0f && fNdotY - 1.0f <= 1e-5f)
		{
			GroundCollision = AK_TRUE;
		}

		AkSphere_t* pSphere = GetCollider()->GetBoundingSphere();
		Vector3 vTriToSphere = pSphere->vCenter - pTri->vP[0];
		AkF32 fDistance = vTriToSphere.Dot(pTri->vNormal);
		Vector3 vProjectedCenter = pSphere->vCenter - pTri->vNormal * (fDistance - pSphere->fRadius);

		_pTransform->SetPosition(&vProjectedCenter);
	}
	if (!wcscmp(L"Dancer", wcName))
	{
	}
	if (!wcscmp(L"BRS_74", wcName))
	{
		Weapon* pWeapon = (Weapon*)pOwner;
		pWeapon->AttachOwner(this);
	}
	if (!wcscmp(L"Box", wcName))
	{

	}*/
}

void Swat::OnCollision(Collider* pOther)
{
	//Actor* pOwner = pOther->GetOwner();
	//const wchar_t* wcName = pOwner->Name;

	//if (!wcscmp(L"Map", wcName))
	//{
	//	AkTriangle_t* pTri = pOther->GetTriangle();
	//	AkF32 fNdotY = pTri->vNormal.Dot(Vector3(0.0f, 1.0f, 0.0f));
	//	if (0.0f <= fNdotY - 1.0f && fNdotY - 1.0f <= 1e-5f)
	//	{
	//		GroundCollision = AK_TRUE;
	//	}

	//	AkSphere_t* pSphere = GetCollider()->GetBoundingSphere();
	//	Vector3 vTriToSphere = pSphere->vCenter - pTri->vP[0];
	//	AkF32 fDistance = vTriToSphere.Dot(pTri->vNormal);
	//	Vector3 vProjectedCenter = pSphere->vCenter - pTri->vNormal * (fDistance - pSphere->fRadius);

	//	_pTransform->SetPosition(&vProjectedCenter);
	//}
	//if (!wcscmp(L"Dancer", wcName))
	//{
	//}
	//if (!wcscmp(L"Ground", wcName))
	//{
	//}
	//if (!wcscmp(L"Box", wcName))
	//{
	//}
}

void Swat::OnCollisionExit(Collider* pOther)
{
	//Actor* pOwner = pOther->GetOwner();
	//const wchar_t* wcName = pOwner->Name;

	//if (!wcscmp(L"Map", wcName))
	//{
	//	GroundCollision = AK_FALSE;
	//}
}

void Swat::CleanUp()
{
	UnBindAnimation();
}

void Swat::SetIdle()
{
	Attack = AK_FALSE;
	Jumping = AK_FALSE;

	SetAnimation(IDLE);
}

void Swat::SetNextPunching()
{

	SetAnimation(PUNCHING_02);

}

void Swat::UpdateMove()
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
			SetAnimation(F_WALK);
		}
		// 30 <= [] <= 60
		else if (0.5f <= fCosValue0 && fCosValue0 <= 0.866025f)
		{
			AkF32 fCosValue1 = vDir.Dot(_pTransform->Right());
			if (fCosValue1 >= 0.0f)
				SetAnimation(FR_WALK);
			else
				SetAnimation(FL_WALK);
		}
		// -60 < [] < 60
		else if (-0.5f < fCosValue0 && fCosValue0 < 0.5f)
		{
			AkF32 fCosValue1 = vDir.Dot(_pTransform->Right());
			if (fCosValue1 >= 0.0f)
				SetAnimation(R_WALK);
			else
				SetAnimation(L_WALK);
		}
		// -60 <= [] <= -30
		else if (-0.866025f <= fCosValue0 && fCosValue0 <= -0.5f)
		{
			AkF32 fCosValue1 = vDir.Dot(_pTransform->Right());
			if (fCosValue1 >= 0.0f)
				SetAnimation(BR_WALK);
			else
				SetAnimation(BL_WALK);
		}
		// [] < -60
		else
		{
			SetAnimation(B_WALK);
		}

		// Return Walk Speed.
		_pRigidBody->SetMaxVeleocity(_fWalkSpeed);
	}
	else if (vVelocity.Length() > 3.0f)
	{
		// Run.
		SetAnimation(F_RUN);
	}
	else if (0.0f >= vVelocity.Length())
	{
		// Idle.
		if (F_WALK <= AnimState && AnimState <= B_WALK)
			SetAnimation(IDLE);

		if (F_RUN == AnimState)
			SetAnimation(IDLE);

		// Return Walk Speed.
		_pRigidBody->SetMaxVeleocity(_fWalkSpeed);
	}
}

void Swat::UpdateWeapon()
{
	//if (!BindWeapon)
	//	return;

	//if (MOVE_STATE::PLAYER_MOVE_STATE_NONE == MoveState)
	//{
	//	_pWeapon->GetTransform()->SetRotation(DirectX::XMConvertToRadians(7.079f), DirectX::XMConvertToRadians(14.038f), DirectX::XMConvertToRadians(149.514f));
	//	_pWeapon->GetTransform()->SetPosition(0.433f, 0.283f, 0.047f);
	//	_pWeapon->GetTransform()->SetScale(0.56f, 0.56f, 0.56f);

	//	LeftHand = AK_TRUE;
	//	RightHand = AK_FALSE;
	//}
	//if (MOVE_STATE::PLAYER_MOVE_STATE_FRONT_WALK <= MoveState && MoveState <= MOVE_STATE::PLAYER_MOVE_STATE_DIAGONAL_LEFT_BACK_WALK)
	//{
	//	_pWeapon->GetTransform()->SetRotation(DirectX::XMConvertToRadians(0.807f), DirectX::XMConvertToRadians(-2.340f), DirectX::XMConvertToRadians(142.585f));
	//	_pWeapon->GetTransform()->SetPosition(0.46f, 0.289f, 0.068f);
	//	_pWeapon->GetTransform()->SetScale(0.56f, 0.56f, 0.56f);

	//	LeftHand = AK_TRUE;
	//	RightHand = AK_FALSE;
	//}
	//if (MOVE_STATE::PLAYER_MOVE_STATE_FRONT_RUN <= MoveState && MoveState <= MOVE_STATE::PLAYER_MOVE_STATE_DIAGONAL_LEFT_BACK_RUN)
	//{
	//	_pWeapon->GetTransform()->SetRotation(DirectX::XMConvertToRadians(-10.821f), DirectX::XMConvertToRadians(17.809f), DirectX::XMConvertToRadians(155.865f));
	//	_pWeapon->GetTransform()->SetPosition(0.437f, 0.293f, 0.053f);
	//	_pWeapon->GetTransform()->SetScale(0.56f, 0.56f, 0.56f);

	//	LeftHand = AK_TRUE;
	//	RightHand = AK_FALSE;
	//}
}

void Swat::UpdateFire()
{
	//if (!Fire)
	//	return;

	//if (MOVE_STATE::PLAYER_MOVE_STATE_NONE == MoveState)
	//{
	//	_pWeapon->GetTransform()->SetRotation(DirectX::XMConvertToRadians(6.16f), DirectX::XMConvertToRadians(-2.817f), DirectX::XMConvertToRadians(178.809f));
	//	_pWeapon->GetTransform()->SetPosition(0.426f, 0.297f, 0.058f);

	//	SetAnimation(ANIM_STATE::PLAYER_ANIM_STATE_RIFLE_IDLE_FIRE);
	//}
	//else if (MOVE_STATE::PLAYER_MOVE_STATE_FRONT_WALK <= MoveState && MoveState <= MOVE_STATE::PLAYER_MOVE_STATE_DIAGONAL_LEFT_BACK_WALK)
	//{
	//	_pWeapon->GetTransform()->SetRotation(DirectX::XMConvertToRadians(16.092f), DirectX::XMConvertToRadians(-21.279f), DirectX::XMConvertToRadians(-166.074f));
	//	_pWeapon->GetTransform()->SetPosition(0.410f, 0.301f, 0.014f);

	//	SetAnimation(ANIM_STATE::PLAYER_ANIM_STATE_RIFLE_WALK_FIRE);
	//}
	//else if (MOVE_STATE::PLAYER_MOVE_STATE_FRONT_RUN <= MoveState && MoveState <= MOVE_STATE::PLAYER_MOVE_STATE_DIAGONAL_LEFT_BACK_RUN)
	//{
	//	_pWeapon->GetTransform()->SetRotation(DirectX::XMConvertToRadians(-23.338f), DirectX::XMConvertToRadians(-20.093f), DirectX::XMConvertToRadians(-167.379f));
	//	_pWeapon->GetTransform()->SetPosition(0.410f, 0.309f, 0.014f);

	//	SetAnimation(ANIM_STATE::PLAYER_ANIM_STATE_RIFLE_RUN_FIRE);
	//}
}

void Swat::FinalUpdateWeapon()
{
	//if (!BindWeapon)
	//{
	//	return;
	//}

	//Matrix* pFinalTransform = ((SkinnedModel*)_pModel)->GetAnimation()->GetFinalTransforms();

	//// Gun model default matrix.
	//if (RightHand)
	//{
	//	Matrix mRightHandAnimTransfrom = pFinalTransform[34].Transpose();
	//	_mHandAnimTransform = mRightHandAnimTransfrom;
	//}
	//if (LeftHand)
	//{
	//	Matrix mLeftHandAnimTransform = pFinalTransform[10].Transpose();
	//	_mHandAnimTransform = mLeftHandAnimTransform;
	//}

	//_pWeapon->GetTransform()->SetParent(&_mHandAnimTransform);
}

void Swat::SetAnimation(ANIM_STATE eState, AkF32 fSpeed)
{
	if (eState != AnimState)
	{
		AnimState = eState;
		_pAnimation->PlayClip(ANIM_CLIP[eState], ANIM_CLIP_STATE::LOOP, fSpeed, 0.2f);
	}
}

void SetIdle(Actor* pSwat)
{
	((Swat*)pSwat)->SetIdle();
}

void SetNextPunching(Actor* pSwat)
{
	((Swat*)pSwat)->SetNextPunching();
}
