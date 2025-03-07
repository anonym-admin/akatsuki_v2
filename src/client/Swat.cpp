#include "pch.h"
#include "Swat.h"
#include "AssetManager.h"
#include "SkinnedModel.h"
#include "Animation.h"
#include "Animator.h"
#include "Transform.h"
#include "Controller.h"
#include "Collider.h"
#include "Gravity.h"
#include "RigidBody.h"
#include "Camera.h"
#include "GameInput.h"
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
	GAssetManager->DeleteMeshData(ASSET_MESH_DATA_TYPE::ASSET_MESH_DATA_TYPE_SWATGUY);

	// Bind Animation.
	((SkinnedModel*)_pModel)->BindAnimation(GAnimator->GetAnimation(GAME_ANIMATION_TYPE::GAME_ANIM_TYPE_PLAYER));

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
	_pRigidBody->SetFrictionCoef(0.0f);
	_pRigidBody->SetMaxVeleocity(10.0f);

	return AK_TRUE;
}

void Swat::Update()
{
	Idle();

	_pController->Update();

	UpdateMove();
	UpdateWeapon();
	UpdateFire();
	UpdateAnimation();

}

void Swat::FinalUpdate()
{
	_pGravity->Update();

	_pRigidBody->Update();

	_pTransform->Update();

	_pCollider->Update();

	_pCamera->Update();

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

void Swat::Idle()
{
	Fire = AK_FALSE;
	SpeedUp = AK_FALSE;

	if (!GroundCollision)
	{
		// 지면과 충돌하지 않았을때 중력 가속도 적용
		_pRigidBody->SetVelocity(0.0f, _pRigidBody->GetVelocity().y, 0.0f);
	}
	else
	{
		// 지면과 충돌 시 위치 보정 중력 가속도는 적용되지 않음
		Vector3 vPos = _pTransform->GetPosition();

		_pRigidBody->SetVelocity(0.0f, 0.0f, 0.0f);
		_pTransform->SetPosition(&vPos);
	}

	if (!BindWeapon)
	{
		AnimState = ANIM_STATE::PLAYER_ANIM_STATE_IDLE;
	}
	else
	{
		AnimState = ANIM_STATE::PLAYER_ANIM_STATE_RIFLE_IDLE;
	}
}

void Swat::UpdateMove()
{
	if (Jumping)
		return;

	Vector3 vFrontDir = _pCamera->GetDirection();
	vFrontDir.y = 0.0f;
	vFrontDir.Normalize();

	Vector3 vUpDir = Vector3(0.0f, 1.0f, 0.0f);
	Vector3 vRightDir = vUpDir.Cross(vFrontDir);

	AkF32 fSpeed = 1.0f;

	Vector3 vNextDir = Vector3(0.0f);
	Vector3 vYawPitchRoll = _pTransform->GetRotation();


	if (MOVE_STATE::PLAYER_MOVE_STATE_FRONT_WALK == MoveState)
	{
		vNextDir += vFrontDir;
	}
	else if (MOVE_STATE::PLAYER_MOVE_STATE_BACK_WALK == MoveState)
	{
		vNextDir -= vFrontDir;

		vYawPitchRoll.x += DirectX::XM_PI;
	}
	else if (MOVE_STATE::PLAYER_MOVE_STATE_RIGHT_WALK == MoveState)
	{
		vNextDir += vRightDir;

		vYawPitchRoll.x += DirectX::XM_PIDIV2;
	}
	else if (MOVE_STATE::PLAYER_MOVE_STATE_LEFT_WALK == MoveState)
	{
		vNextDir -= vRightDir;

		vYawPitchRoll.x -= DirectX::XM_PIDIV2;
	}
	else if (MOVE_STATE::PLAYER_MOVE_STATE_DIAGONAL_RIGHT_FRONT_WALK == MoveState)
	{
		vNextDir += vFrontDir;
		vNextDir += vRightDir;

		vYawPitchRoll.x += DirectX::XM_PIDIV4;
	}
	else if (MOVE_STATE::PLAYER_MOVE_STATE_DIAGONAL_LEFT_FRONT_WALK == MoveState)
	{
		vNextDir += vFrontDir;
		vNextDir -= vRightDir;

		vYawPitchRoll.x -= DirectX::XM_PIDIV4;
	}
	else if (MOVE_STATE::PLAYER_MOVE_STATE_DIAGONAL_RIHGT_BACK_WALK == MoveState)
	{
		vNextDir -= vFrontDir;
		vNextDir += vRightDir;

		vYawPitchRoll.x += (DirectX::XM_PI - DirectX::XM_PIDIV4);
	}
	else if (MOVE_STATE::PLAYER_MOVE_STATE_DIAGONAL_LEFT_BACK_WALK == MoveState)
	{
		vNextDir -= vFrontDir;
		vNextDir -= vRightDir;

		vYawPitchRoll.x += (DirectX::XM_PI + DirectX::XM_PIDIV4);
	}
	else if (MOVE_STATE::PLAYER_MOVE_STATE_FRONT_RUN == MoveState)
	{
		vNextDir += vFrontDir;
	}
	else if (MOVE_STATE::PLAYER_MOVE_STATE_BACK_RUN == MoveState)
	{
		vNextDir -= vFrontDir;

		vYawPitchRoll.x += DirectX::XM_PI;
	}
	else if (MOVE_STATE::PLAYER_MOVE_STATE_RIGHT_RUN == MoveState)
	{
		vNextDir += vRightDir;

		vYawPitchRoll.x += DirectX::XM_PIDIV2;
	}
	else if (MOVE_STATE::PLAYER_MOVE_STATE_LEFT_RUN == MoveState)
	{
		vNextDir -= vRightDir;

		vYawPitchRoll.x -= DirectX::XM_PIDIV2;
	}
	else if (MOVE_STATE::PLAYER_MOVE_STATE_DIAGONAL_RIGHT_FRONT_RUN == MoveState)
	{
		vNextDir += vFrontDir;
		vNextDir += vRightDir;

		vYawPitchRoll.x += DirectX::XM_PIDIV4;
	}
	else if (MOVE_STATE::PLAYER_MOVE_STATE_DIAGONAL_LEFT_FRONT_RUN == MoveState)
	{
		vNextDir += vFrontDir;
		vNextDir -= vRightDir;

		vYawPitchRoll.x += -DirectX::XM_PIDIV4;
	}
	else if (MOVE_STATE::PLAYER_MOVE_STATE_DIAGONAL_RIGHT_BACK_RUN == MoveState)
	{
		vNextDir -= vFrontDir;
		vNextDir += vRightDir;

		vYawPitchRoll.x += (DirectX::XM_PI - DirectX::XM_PIDIV4);
	}
	else if (MOVE_STATE::PLAYER_MOVE_STATE_DIAGONAL_LEFT_BACK_RUN == MoveState)
	{
		vNextDir -= vFrontDir;
		vNextDir -= vRightDir;

		vYawPitchRoll.x += (DirectX::XM_PI + DirectX::XM_PIDIV4);
	}

	vNextDir.Normalize();

	// Run 적용
	if (SpeedUp)
		fSpeed *= 3.5f;

	Vector3 vNextVelocity = vNextDir * fSpeed;

	// 중력적용
	if (!GroundCollision)
	{
		vNextVelocity = Vector3(vNextVelocity.x, _pRigidBody->GetVelocity().y, vNextVelocity.z);
	}

	_pTransform->SetRotation(&vYawPitchRoll);

	_pRigidBody->SetVelocity(vNextVelocity);
}

void Swat::UpdateWeapon()
{
	if (!BindWeapon)
		return;

	if (MOVE_STATE::PLAYER_MOVE_STATE_NONE == MoveState)
	{
		_pWeapon->GetTransform()->SetRotation(DirectX::XMConvertToRadians(7.079f), DirectX::XMConvertToRadians(14.038f), DirectX::XMConvertToRadians(149.514f));
		_pWeapon->GetTransform()->SetPosition(0.433f, 0.283f, 0.047f);
		_pWeapon->GetTransform()->SetScale(0.56f, 0.56f, 0.56f);

		LeftHand = AK_TRUE;
		RightHand = AK_FALSE;
	}
	if (MOVE_STATE::PLAYER_MOVE_STATE_FRONT_WALK <= MoveState && MoveState <= MOVE_STATE::PLAYER_MOVE_STATE_DIAGONAL_LEFT_BACK_WALK)
	{
		_pWeapon->GetTransform()->SetRotation(DirectX::XMConvertToRadians(0.807f), DirectX::XMConvertToRadians(-2.340f), DirectX::XMConvertToRadians(142.585f));
		_pWeapon->GetTransform()->SetPosition(0.46f, 0.289f, 0.068f);
		_pWeapon->GetTransform()->SetScale(0.56f, 0.56f, 0.56f);

		LeftHand = AK_TRUE;
		RightHand = AK_FALSE;
	}
	if (MOVE_STATE::PLAYER_MOVE_STATE_FRONT_RUN <= MoveState && MoveState <= MOVE_STATE::PLAYER_MOVE_STATE_DIAGONAL_LEFT_BACK_RUN)
	{
		_pWeapon->GetTransform()->SetRotation(DirectX::XMConvertToRadians(-10.821f), DirectX::XMConvertToRadians(17.809f), DirectX::XMConvertToRadians(155.865f));
		_pWeapon->GetTransform()->SetPosition(0.437f, 0.293f, 0.053f);
		_pWeapon->GetTransform()->SetScale(0.56f, 0.56f, 0.56f);

		LeftHand = AK_TRUE;
		RightHand = AK_FALSE;
	}
}

void Swat::UpdateFire()
{
	if (!Fire)
		return;

	if (MOVE_STATE::PLAYER_MOVE_STATE_NONE == MoveState)
	{
		_pWeapon->GetTransform()->SetRotation(DirectX::XMConvertToRadians(6.16f), DirectX::XMConvertToRadians(-2.817f), DirectX::XMConvertToRadians(178.809f));
		_pWeapon->GetTransform()->SetPosition(0.426f, 0.297f, 0.058f);

		AnimState = ANIM_STATE::PLAYER_ANIM_STATE_RIFLE_IDLE_FIRE;
	}
	else if (MOVE_STATE::PLAYER_MOVE_STATE_FRONT_WALK <= MoveState && MoveState <= MOVE_STATE::PLAYER_MOVE_STATE_DIAGONAL_LEFT_BACK_WALK)
	{
		_pWeapon->GetTransform()->SetRotation(DirectX::XMConvertToRadians(16.092f), DirectX::XMConvertToRadians(-21.279f), DirectX::XMConvertToRadians(-166.074f));
		_pWeapon->GetTransform()->SetPosition(0.410f, 0.301f, 0.014f);

		AnimState = ANIM_STATE::PLAYER_ANIM_STATE_RIFLE_WALK_FIRE;
	}
	else if (MOVE_STATE::PLAYER_MOVE_STATE_FRONT_RUN <= MoveState && MoveState <= MOVE_STATE::PLAYER_MOVE_STATE_DIAGONAL_LEFT_BACK_RUN)
	{
		_pWeapon->GetTransform()->SetRotation(DirectX::XMConvertToRadians(-23.338f), DirectX::XMConvertToRadians(-20.093f), DirectX::XMConvertToRadians(-167.379f));
		_pWeapon->GetTransform()->SetPosition(0.410f, 0.309f, 0.014f);

		AnimState = ANIM_STATE::PLAYER_ANIM_STATE_RIFLE_RUN_FIRE;
	}
}

void Swat::UpdateAnimation()
{
	if (Jumping)
	{
		// 아직 점프중
		AnimState = ANIM_STATE::PLAYER_ANIM_STATE_JUMP;
	}

	AkBool bEndAnim = ((SkinnedModel*)_pModel)->PlayAnimation(GAME_ANIM_PLAYER_ANIM_FILE_NAME[(AkU32)AnimState], AK_FALSE);

	if (ANIM_STATE::PLAYER_ANIM_STATE_JUMP == AnimState)
	{
		if (bEndAnim)
		{
			Jumping = AK_FALSE;
		}
	}
}

void Swat::FinalUpdateWeapon()
{
	if (!BindWeapon)
	{
		return;
	}

	Matrix* pFinalTransform = ((SkinnedModel*)_pModel)->GetAnimation()->GetFinalTransforms();

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

	_pWeapon->GetTransform()->SetParent(&_mHandAnimTransform);
}
