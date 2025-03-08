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
	AssetMeshDataContainer_t* pMeshDataContainer = GAssetManager->GetMeshDataContainer(ASSET_MESH_DATA_TYPE::ASSET_MESH_DATA_TYPE_SWATGUY);
	Vector3 vAlbedo = Vector3(1.0f);
	Vector3 vEmissive = Vector3(0.0f);
	_pModel = CreateModel(pMeshDataContainer, &vAlbedo, 0.0f, 1.0f, &vEmissive, AK_TRUE);

	// Create Anim.
	_pAnimation = CreateAnimation(pMeshDataContainer, ANIM_CLIP[IDLE], (AkU32)COUNT);
	_pAnimation->ReadClip(ANIM_FILE_PATH, ANIM_CLIP[IDLE]);
	_pAnimation->ReadClip(ANIM_FILE_PATH, ANIM_CLIP[F_WALK]);
	_pAnimation->ReadClip(ANIM_FILE_PATH, ANIM_CLIP[FL_WALK]);
	_pAnimation->ReadClip(ANIM_FILE_PATH, ANIM_CLIP[FR_WALK]);
	_pAnimation->ReadClip(ANIM_FILE_PATH, ANIM_CLIP[L_WALK]);
	_pAnimation->ReadClip(ANIM_FILE_PATH, ANIM_CLIP[R_WALK]);
	_pAnimation->ReadClip(ANIM_FILE_PATH, ANIM_CLIP[BL_WALK]);
	_pAnimation->ReadClip(ANIM_FILE_PATH, ANIM_CLIP[BR_WALK]);
	_pAnimation->ReadClip(ANIM_FILE_PATH, ANIM_CLIP[B_WALK]);
	_pAnimation->ReadClip(ANIM_FILE_PATH, ANIM_CLIP[F_RUN]);
	_pAnimation->ReadClip(ANIM_FILE_PATH, ANIM_CLIP[FL_RUN]);
	_pAnimation->ReadClip(ANIM_FILE_PATH, ANIM_CLIP[FR_RUN]);
	_pAnimation->ReadClip(ANIM_FILE_PATH, ANIM_CLIP[PUNCHING_01]);
	SetAnimation(IDLE);

	// Delete MeshData Resource.
	GAssetManager->DeleteMeshData(ASSET_MESH_DATA_TYPE::ASSET_MESH_DATA_TYPE_SWATGUY);

	// Bind Animation.
	((SkinnedModel*)_pModel)->BindAnimation(_pAnimation);

	// Create Controller.
	_pController = CreateController();

	// Create Trnasform.
	_pTransform = CreateTransform();

	// Create Collider.
	Vector3 vCenter = Vector3(0.0f);
	AkF32 fRadius = 0.5f;
	_pCollider = CreateCollider();
	_pCollider->CreateBoundingSphere(fRadius, &vCenter);

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
	_pRigidBody->SetMaxVeleocity(3.5f);

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

	_pCamera->Update();

	_pAnimation->Update();

	_pModel->UpdateWorldRow(_pTransform->GetWorldTransformAddr());

	FinalUpdateWeapon();
}

void Swat::Render()
{
	_pModel->Render();
}

void Swat::RenderShadow()
{
	_pModel->RenderShadow();
}

void Swat::OnCollisionEnter(Collider* pOther)
{
	Actor* pOwner = pOther->GetOwner();
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

	}
}

void Swat::OnCollision(Collider* pOther)
{
	Actor* pOwner = pOther->GetOwner();
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
	if (!wcscmp(L"Ground", wcName))
	{
	}
	if (!wcscmp(L"Box", wcName))
	{
	}
}

void Swat::OnCollisionExit(Collider* pOther)
{
	Actor* pOwner = pOther->GetOwner();
	const wchar_t* wcName = pOwner->Name;

	if (!wcscmp(L"Map", wcName))
	{
		GroundCollision = AK_FALSE;
	}
}

void Swat::CleanUp()
{
}

void Swat::SetIdle()
{
	SetAnimation(IDLE);
}

void Swat::UpdateMove()
{
	Vector3 vVelocity = _pRigidBody->GetVelocity();

	// printf("%lf\n", vVelocity.Length());

	if (0.2f < vVelocity.Length() && vVelocity.Length() <= 2.8f)
	{
		// Walk.
		Vector3 vDir = vVelocity;
		vDir.Normalize();

		AkF32 fCosValue0 = vDir.Dot(_pTransform->Front());

		if (0.866025f < fCosValue0)
		{
			SetAnimation(F_WALK);
		}
		else if (0.5f <= fCosValue0 && fCosValue0 <= 0.866025f)
		{
			AkF32 fCosValue1 = vDir.Dot(_pTransform->Right());
			if (fCosValue1 >= 0.0f)
				SetAnimation(FR_WALK);
			else
				SetAnimation(FL_WALK);
		}
		else if (-0.5f < fCosValue0 && fCosValue0 < 0.5f)
		{
			AkF32 fCosValue1 = vDir.Dot(_pTransform->Right());
			if (fCosValue1 >= 0.0f)
				SetAnimation(R_WALK);
			else
				SetAnimation(L_WALK);
		}
		else if (-0.866025f <= fCosValue0 && fCosValue0 <= -0.5f)
		{
			AkF32 fCosValue1 = vDir.Dot(_pTransform->Right());
			if (fCosValue1 >= 0.0f)
				SetAnimation(BR_WALK);
			else
				SetAnimation(BL_WALK);
		}
		else
		{
			SetAnimation(B_WALK);
		}
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

void SetIdle(Swat* pSwat)
{
	pSwat->SetIdle();
}
