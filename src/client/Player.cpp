#include "pch.h"
#include "Player.h"
#include "PlayerModel.h"
#include "GameInput.h"
#include "Application.h"
#include "RigidBody.h"
#include "Gravity.h"
#include "Camera.h"
#include "Timer.h"
#include "BRS_74.h"
#include "Collider.h"
#include "Animation.h"
#include "ModelManager.h"

#include "Sound.h"

/*
==========
Player
==========
*/

UPlayer::UPlayer()
{
}

UPlayer::~UPlayer()
{
	CleanUp();
}

AkBool UPlayer::Initialize(UApplication* pApp)
{
	if (!UActor::Initialize(pApp))
	{
		__debugbreak();
		return AK_FALSE;
	}

	// Bind Model.
	UModelManager* pModelManager = pApp->GetModelManager();
	UModel* pHeroModel = pModelManager->GetModel(MODEL_TYPE::BLENDER_MODEL_HERO);
	BindModel(pHeroModel);

	// Create Collider.
	UCollider* pCollider = CreateCollider();
	Vector3 vCenter = Vector3(0.0f);
	AkF32 fRadius = 0.5f;
	pCollider->CreateBoundingSphere(fRadius, &vCenter);

	// Create Gravity.
	UGravity* pGravidy = CreateGravity();

	// Create Rigid Body.
	URigidBody* pRigidBody = CreateRigidBody();
	pRigidBody->SetFrictionCoef(0.0f);
	pRigidBody->SetMaxVeleocity(10.0f);

	// Create Camera.
	UInGameCamera* pCamera = CreateCamera();

	return AK_TRUE;
}

void UPlayer::Update(const AkF32 fDeltaTime)
{
	UApplication* pApp = GetApp();
	UPlayerModel* pModel = (UPlayerModel*)GetModel(GetModelContextIndex());
	UGameInput* pGameInput = pApp->GetGameInput();
	URigidBody* pRigidBody = GetRigidBody();
	UCollider* pCollider = GetCollider();
	UInGameCamera* pCamera = GetCamera();

	AkBool bEnableEditor = pApp->EnableEditor();

	if (!_bJumping)
	{
		if (!_bBindWeapon)
		{
			_eAnimState = PLAYER_ANIM_STATE::PLAYER_ANIM_STATE_IDLE;
		}
		else
		{
			_eAnimState = PLAYER_ANIM_STATE::PLAYER_ANIM_STATE_RIFLE_IDLE;
		}
	}

	AkBool bSpeedUp = AK_FALSE;
	_eMoveState = PLAYER_MOVE_STATE::PLAYER_MOVE_STATE_NONE;

	/*
	=======================
	Update player state.
	=======================
	*/

	_bFire = AK_FALSE;






	// 1. walk
	if (pGameInput->KeyHoldDown(KEY_INPUT_W))
	{
		_eMoveState = PLAYER_MOVE_STATE::PLAYER_MOVE_STATE_FRONT_WALK;

		if (_bBindWeapon)
		{
			_eAnimState = PLAYER_ANIM_STATE::PLAYER_ANIM_STATE_RIFLE_WALK;
		}
		else
		{
			_eAnimState = PLAYER_ANIM_STATE::PLAYER_ANIM_STATE_WALK;
		}
	}
	if (pGameInput->KeyHoldDown(KEY_INPUT_S))
	{
		_eMoveState = PLAYER_MOVE_STATE::PLAYER_MOVE_STATE_BACK_WALK;

		if (_bBindWeapon)
		{
			_eAnimState = PLAYER_ANIM_STATE::PLAYER_ANIM_STATE_RIFLE_WALK;
		}
		else
		{
			_eAnimState = PLAYER_ANIM_STATE::PLAYER_ANIM_STATE_WALK;
		}
	}
	if (pGameInput->KeyHoldDown(KEY_INPUT_A))
	{
		_eMoveState = PLAYER_MOVE_STATE::PLAYER_MOVE_STATE_LEFT_WALK;

		if (_bBindWeapon)
		{
			_eAnimState = PLAYER_ANIM_STATE::PLAYER_ANIM_STATE_RIFLE_WALK;
		}
		else
		{
			_eAnimState = PLAYER_ANIM_STATE::PLAYER_ANIM_STATE_WALK;
		}
	}
	if (pGameInput->KeyHoldDown(KEY_INPUT_D))
	{
		_eMoveState = PLAYER_MOVE_STATE::PLAYER_MOVE_STATE_RIGHT_WALK;

		if (_bBindWeapon)
		{
			_eAnimState = PLAYER_ANIM_STATE::PLAYER_ANIM_STATE_RIFLE_WALK;
		}
		else
		{
			_eAnimState = PLAYER_ANIM_STATE::PLAYER_ANIM_STATE_WALK;
		}
	}
	if (pGameInput->KeyHoldDown(KEY_INPUT_W) && pGameInput->KeyHoldDown(KEY_INPUT_A))
	{
		_eMoveState = PLAYER_MOVE_STATE::PLAYER_MOVE_STATE_DIAGONAL_LEFT_FRONT_WALK;

		if (_bBindWeapon)
		{
			_eAnimState = PLAYER_ANIM_STATE::PLAYER_ANIM_STATE_RIFLE_WALK;
		}
		else
		{
			_eAnimState = PLAYER_ANIM_STATE::PLAYER_ANIM_STATE_WALK;
		}
	}
	if (pGameInput->KeyHoldDown(KEY_INPUT_W) && pGameInput->KeyHoldDown(KEY_INPUT_D))
	{
		_eMoveState = PLAYER_MOVE_STATE::PLAYER_MOVE_STATE_DIAGONAL_RIGHT_FRONT_WALK;

		if (_bBindWeapon)
		{
			_eAnimState = PLAYER_ANIM_STATE::PLAYER_ANIM_STATE_RIFLE_WALK;
		}
		else
		{
			_eAnimState = PLAYER_ANIM_STATE::PLAYER_ANIM_STATE_WALK;
		};
	}
	if (pGameInput->KeyHoldDown(KEY_INPUT_S) && pGameInput->KeyHoldDown(KEY_INPUT_A))
	{
		_eMoveState = PLAYER_MOVE_STATE::PLAYER_MOVE_STATE_DIAGONAL_LEFT_BACK_WALK;

		if (_bBindWeapon)
		{
			_eAnimState = PLAYER_ANIM_STATE::PLAYER_ANIM_STATE_RIFLE_WALK;
		}
		else
		{
			_eAnimState = PLAYER_ANIM_STATE::PLAYER_ANIM_STATE_WALK;
		}
	}
	if (pGameInput->KeyHoldDown(KEY_INPUT_S) && pGameInput->KeyHoldDown(KEY_INPUT_D))
	{
		_eMoveState = PLAYER_MOVE_STATE::PLAYER_MOVE_STATE_DIAGONAL_RIHGT_BACK_WALK;

		if (_bBindWeapon)
		{
			_eAnimState = PLAYER_ANIM_STATE::PLAYER_ANIM_STATE_RIFLE_WALK;
		}
		else
		{
			_eAnimState = PLAYER_ANIM_STATE::PLAYER_ANIM_STATE_WALK;
		}
	}

	// 2. run
	if (pGameInput->KeyHoldDown(KEY_INPUT_W) && pGameInput->KeyHoldDown(KEY_INPUT_LSHIFT))
	{
		bSpeedUp = AK_TRUE;

		_eMoveState = PLAYER_MOVE_STATE::PLAYER_MOVE_STATE_FRONT_RUN;

		if (_bBindWeapon)
		{
			_eAnimState = PLAYER_ANIM_STATE::PLAYER_ANIM_STATE_RIFLE_RUN;
		}
		else
		{
			_eAnimState = PLAYER_ANIM_STATE::PLAYER_ANIM_STATE_RUN;
		}
	}
	if (pGameInput->KeyHoldDown(KEY_INPUT_S) && pGameInput->KeyHoldDown(KEY_INPUT_LSHIFT))
	{
		bSpeedUp = AK_TRUE;

		_eMoveState = PLAYER_MOVE_STATE::PLAYER_MOVE_STATE_BACK_RUN;

		if (_bBindWeapon)
		{
			_eAnimState = PLAYER_ANIM_STATE::PLAYER_ANIM_STATE_RIFLE_RUN;
		}
		else
		{
			_eAnimState = PLAYER_ANIM_STATE::PLAYER_ANIM_STATE_RUN;
		}
	}
	if (pGameInput->KeyHoldDown(KEY_INPUT_A) && pGameInput->KeyHoldDown(KEY_INPUT_LSHIFT))
	{
		bSpeedUp = AK_TRUE;

		_eMoveState = PLAYER_MOVE_STATE::PLAYER_MOVE_STATE_LEFT_RUN;

		if (_bBindWeapon)
		{
			_eAnimState = PLAYER_ANIM_STATE::PLAYER_ANIM_STATE_RIFLE_RUN;
		}
		else
		{
			_eAnimState = PLAYER_ANIM_STATE::PLAYER_ANIM_STATE_RUN;
		}
	}
	if (pGameInput->KeyHoldDown(KEY_INPUT_D) && pGameInput->KeyHoldDown(KEY_INPUT_LSHIFT))
	{
		bSpeedUp = AK_TRUE;

		_eMoveState = PLAYER_MOVE_STATE::PLAYER_MOVE_STATE_RIGHT_RUN;

		if (_bBindWeapon)
		{
			_eAnimState = PLAYER_ANIM_STATE::PLAYER_ANIM_STATE_RIFLE_RUN;
		}
		else
		{
			_eAnimState = PLAYER_ANIM_STATE::PLAYER_ANIM_STATE_RUN;
		}
	}
	if (pGameInput->KeyHoldDown(KEY_INPUT_W) && pGameInput->KeyHoldDown(KEY_INPUT_D) && pGameInput->KeyHoldDown(KEY_INPUT_LSHIFT))
	{
		bSpeedUp = AK_TRUE;

		_eMoveState = PLAYER_MOVE_STATE::PLAYER_MOVE_STATE_DIAGONAL_RIGHT_FRONT_RUN;

		if (_bBindWeapon)
		{
			_eAnimState = PLAYER_ANIM_STATE::PLAYER_ANIM_STATE_RIFLE_RUN;
		}
		else
		{
			_eAnimState = PLAYER_ANIM_STATE::PLAYER_ANIM_STATE_RUN;
		}
	}
	if (pGameInput->KeyHoldDown(KEY_INPUT_W) && pGameInput->KeyHoldDown(KEY_INPUT_A) && pGameInput->KeyHoldDown(KEY_INPUT_LSHIFT))
	{
		bSpeedUp = AK_TRUE;

		_eMoveState = PLAYER_MOVE_STATE::PLAYER_MOVE_STATE_DIAGONAL_LEFT_FRONT_RUN;

		if (_bBindWeapon)
		{
			_eAnimState = PLAYER_ANIM_STATE::PLAYER_ANIM_STATE_RIFLE_RUN;
		}
		else
		{
			_eAnimState = PLAYER_ANIM_STATE::PLAYER_ANIM_STATE_RUN;
		}
	}
	if (pGameInput->KeyHoldDown(KEY_INPUT_S) && pGameInput->KeyHoldDown(KEY_INPUT_D) && pGameInput->KeyHoldDown(KEY_INPUT_LSHIFT))
	{
		bSpeedUp = AK_TRUE;

		_eMoveState = PLAYER_MOVE_STATE::PLAYER_MOVE_STATE_DIAGONAL_RIGHT_BACK_RUN;

		if (_bBindWeapon)
		{
			_eAnimState = PLAYER_ANIM_STATE::PLAYER_ANIM_STATE_RIFLE_RUN;
		}
		else
		{
			_eAnimState = PLAYER_ANIM_STATE::PLAYER_ANIM_STATE_RUN;
		}
	}
	if (pGameInput->KeyHoldDown(KEY_INPUT_S) && pGameInput->KeyHoldDown(KEY_INPUT_A) && pGameInput->KeyHoldDown(KEY_INPUT_LSHIFT))
	{
		bSpeedUp = AK_TRUE;

		_eMoveState = PLAYER_MOVE_STATE::PLAYER_MOVE_STATE_DIAGONAL_LEFT_BACK_RUN;

		if (_bBindWeapon)
		{
			_eAnimState = PLAYER_ANIM_STATE::PLAYER_ANIM_STATE_RIFLE_RUN;
		}
		else
		{
			_eAnimState = PLAYER_ANIM_STATE::PLAYER_ANIM_STATE_RUN;
		}
	}






	// Fire
	if (_bBindWeapon)
	{
		if (pGameInput->LeftBtnDown())
		{
			// Test
			// GetApp()->GetTestSound()->PlayOnce();
		}
		if (pGameInput->LeftBtnHold())
		{
			if (bSpeedUp)
				_eAnimState = PLAYER_ANIM_STATE::PLAYER_ANIM_STATE_RIFLE_RUN_FIRE;
			else
				_eAnimState = PLAYER_ANIM_STATE::PLAYER_ANIM_STATE_RIFLE_WALK_FIRE;

			_bFire = AK_TRUE;

			// Test
			GetApp()->GetTestSound()->Play(AK_FALSE);
		}
	}






	// 아직 점프중이라면 점프를 유지한다.
	if (_bJumping)
	{
		_eAnimState = PLAYER_ANIM_STATE::PLAYER_ANIM_STATE_JUMP;
	}

	/*
	=======================
	Update position.
	=======================
	*/

	// Update move 
	if (!_bGroundCollision)
	{
		// 지면과 충돌하지 않았을때 중력 가속도 적용
		pRigidBody->SetVelocity(0.0f, pRigidBody->GetVelocity().y, 0.0f);
	}
	else
	{
		// 지면과 충돌 시 위치 보정 중력 가속도는 적용되지 않음
		Vector3 vPos = GetPosition();

		pRigidBody->SetVelocity(0.0f, 0.0f, 0.0f);
		SetPosition(vPos.x, vPos.y, vPos.z);
	}

	Vector3 vFrontDir = pCamera->GetCameraDirection();
	vFrontDir.y = 0.0f;
	vFrontDir.Normalize();

	Vector3 vUpDir = Vector3(0.0f, 1.0f, 0.0f);
	Vector3 vRightDir = vUpDir.Cross(vFrontDir);

	AkF32 fSpeed = 1.0f;

	Vector3 vNextDir = Vector3(0.0f);
	AkF32 vCurRotY = GetRotationY();

	if (PLAYER_MOVE_STATE::PLAYER_MOVE_STATE_NONE == _eMoveState)
	{
		if (_bBindWeapon && !bEnableEditor)
		{
			// 해당 플레이어 중심으로 위치 변경
			// 스크립트 파일로 저장 필요.
			_pWeapon->SetRelativeRotationX(-14.038f);
			_pWeapon->SetRelativeRotationY(7.079f);
			_pWeapon->SetRelativeRotationZ(149.514f);
			_pWeapon->SetRelativePosition(0.433f, 0.283f, 0.047f);
			_pWeapon->SetScale(0.56f, 0.56f, 0.56f);
			_pWeapon->SetOwnerRotationY(GetRotationY());

			_bLeftHand = AK_TRUE;
			_bRightHand = AK_FALSE;

			if (_bFire)
			{
				_pWeapon->SetRelativeRotationX(2.817f);
				_pWeapon->SetRelativeRotationY(6.16f);
				_pWeapon->SetRelativeRotationZ(178.809f);
				_pWeapon->SetRelativePosition(0.426f, 0.297f, 0.058f);

				_eAnimState = PLAYER_ANIM_STATE::PLAYER_ANIM_STATE_RIFLE_IDLE_FIRE;
			}
		}
	}
	else if (PLAYER_MOVE_STATE::PLAYER_MOVE_STATE_FRONT_WALK == _eMoveState)
	{
		vNextDir += vFrontDir;

		if (_bBindWeapon && !bEnableEditor)
		{
			_pWeapon->SetRelativeRotationX(2.340f);
			_pWeapon->SetRelativeRotationY(-0.807f);
			_pWeapon->SetRelativeRotationZ(142.585f);
			_pWeapon->SetRelativePosition(0.46f, 0.289f, 0.068f);
			_pWeapon->SetScale(0.56f, 0.56f, 0.56f);
			_pWeapon->SetOwnerRotationY(GetRotationY());

			_bLeftHand = AK_TRUE;
			_bRightHand = AK_FALSE;

			if (_bFire)
			{
				_pWeapon->SetRelativeRotationX(21.279f);
				_pWeapon->SetRelativeRotationY(-16.092f);
				_pWeapon->SetRelativeRotationZ(-166.074f);
				_pWeapon->SetRelativePosition(0.410f, 0.301f, 0.014f);

				_eAnimState = PLAYER_ANIM_STATE::PLAYER_ANIM_STATE_RIFLE_WALK_FIRE;
			}
		}
	}
	else if (PLAYER_MOVE_STATE::PLAYER_MOVE_STATE_BACK_WALK == _eMoveState)
	{
		vNextDir -= vFrontDir;

		SetRotationY(vCurRotY + DirectX::XM_PI);

		// Weapon
		if (_bBindWeapon && !bEnableEditor)
		{
			_pWeapon->SetRelativeRotationX(2.340f);
			_pWeapon->SetRelativeRotationY(-0.807f);
			_pWeapon->SetRelativeRotationZ(142.585f);
			_pWeapon->SetRelativePosition(0.46f, 0.289f, 0.068f);
			_pWeapon->SetScale(0.56f, 0.56f, 0.56f);
			_pWeapon->SetOwnerRotationY(GetRotationY());

			_bLeftHand = AK_TRUE;
			_bRightHand = AK_FALSE;

			if (_bFire)
			{
				_pWeapon->SetRelativeRotationX(21.279f);
				_pWeapon->SetRelativeRotationY(-16.092f);
				_pWeapon->SetRelativeRotationZ(-166.074f);
				_pWeapon->SetRelativePosition(0.410f, 0.301f, 0.014f);

				_eAnimState = PLAYER_ANIM_STATE::PLAYER_ANIM_STATE_RIFLE_WALK_FIRE;
			}
		}
	}
	else if (PLAYER_MOVE_STATE::PLAYER_MOVE_STATE_RIGHT_WALK == _eMoveState)
	{
		vNextDir += vRightDir;

		SetRotationY(vCurRotY + DirectX::XM_PIDIV2);

		// Weapon
		if (_bBindWeapon && !bEnableEditor)
		{
			_pWeapon->SetRelativeRotationX(2.340f);
			_pWeapon->SetRelativeRotationY(-0.807f);
			_pWeapon->SetRelativeRotationZ(142.585f);
			_pWeapon->SetRelativePosition(0.46f, 0.289f, 0.068f);
			_pWeapon->SetScale(0.56f, 0.56f, 0.56f);
			_pWeapon->SetOwnerRotationY(GetRotationY());

			_bLeftHand = AK_TRUE;
			_bRightHand = AK_FALSE;

			if (_bFire)
			{
				_pWeapon->SetRelativeRotationX(21.279f);
				_pWeapon->SetRelativeRotationY(-16.092f);
				_pWeapon->SetRelativeRotationZ(-166.074f);
				_pWeapon->SetRelativePosition(0.410f, 0.301f, 0.014f);

				_eAnimState = PLAYER_ANIM_STATE::PLAYER_ANIM_STATE_RIFLE_WALK_FIRE;
			}
		}
	}
	else if (PLAYER_MOVE_STATE::PLAYER_MOVE_STATE_LEFT_WALK == _eMoveState)
	{
		vNextDir -= vRightDir;

		SetRotationY(vCurRotY - DirectX::XM_PIDIV2);

		// Weapon
		if (_bBindWeapon && !bEnableEditor)
		{
			_pWeapon->SetRelativeRotationX(2.340f);
			_pWeapon->SetRelativeRotationY(-0.807f);
			_pWeapon->SetRelativeRotationZ(142.585f);
			_pWeapon->SetRelativePosition(0.46f, 0.289f, 0.068f);
			_pWeapon->SetScale(0.56f, 0.56f, 0.56f);
			_pWeapon->SetOwnerRotationY(GetRotationY());

			_bLeftHand = AK_TRUE;
			_bRightHand = AK_FALSE;

			if (_bFire)
			{
				_pWeapon->SetRelativeRotationX(21.279f);
				_pWeapon->SetRelativeRotationY(-16.092f);
				_pWeapon->SetRelativeRotationZ(-166.074f);
				_pWeapon->SetRelativePosition(0.410f, 0.301f, 0.014f);

				_eAnimState = PLAYER_ANIM_STATE::PLAYER_ANIM_STATE_RIFLE_WALK_FIRE;
			}
		}
	}
	else if (PLAYER_MOVE_STATE::PLAYER_MOVE_STATE_DIAGONAL_RIGHT_FRONT_WALK == _eMoveState)
	{
		vNextDir += vFrontDir;
		vNextDir += vRightDir;

		SetRotationY(vCurRotY + DirectX::XM_PIDIV4);

		// Weapon
		if (_bBindWeapon && !bEnableEditor)
		{
			_pWeapon->SetRelativeRotationX(2.340f);
			_pWeapon->SetRelativeRotationY(-0.807f);
			_pWeapon->SetRelativeRotationZ(142.585f);
			_pWeapon->SetRelativePosition(0.46f, 0.289f, 0.068f);
			_pWeapon->SetScale(0.56f, 0.56f, 0.56f);
			_pWeapon->SetOwnerRotationY(GetRotationY());

			_bLeftHand = AK_TRUE;
			_bRightHand = AK_FALSE;

			if (_bFire)
			{
				_pWeapon->SetRelativeRotationX(21.279f);
				_pWeapon->SetRelativeRotationY(-16.092f);
				_pWeapon->SetRelativeRotationZ(-166.074f);
				_pWeapon->SetRelativePosition(0.410f, 0.301f, 0.014f);

				_eAnimState = PLAYER_ANIM_STATE::PLAYER_ANIM_STATE_RIFLE_WALK_FIRE;
			}
		}
	}
	else if (PLAYER_MOVE_STATE::PLAYER_MOVE_STATE_DIAGONAL_LEFT_FRONT_WALK == _eMoveState)
	{
		vNextDir += vFrontDir;
		vNextDir -= vRightDir;

		SetRotationY(vCurRotY - DirectX::XM_PIDIV4);

		// Weapon
		if (_bBindWeapon && !bEnableEditor)
		{
			_pWeapon->SetRelativeRotationX(2.340f);
			_pWeapon->SetRelativeRotationY(-0.807f);
			_pWeapon->SetRelativeRotationZ(142.585f);
			_pWeapon->SetRelativePosition(0.46f, 0.289f, 0.068f);
			_pWeapon->SetScale(0.56f, 0.56f, 0.56f);
			_pWeapon->SetOwnerRotationY(GetRotationY());

			_bLeftHand = AK_TRUE;
			_bRightHand = AK_FALSE;

			if (_bFire)
			{
				_pWeapon->SetRelativeRotationX(21.279f);
				_pWeapon->SetRelativeRotationY(-16.092f);
				_pWeapon->SetRelativeRotationZ(-166.074f);
				_pWeapon->SetRelativePosition(0.410f, 0.301f, 0.014f);

				_eAnimState = PLAYER_ANIM_STATE::PLAYER_ANIM_STATE_RIFLE_WALK_FIRE;
			}
		}
	}
	else if (PLAYER_MOVE_STATE::PLAYER_MOVE_STATE_DIAGONAL_RIHGT_BACK_WALK == _eMoveState)
	{
		vNextDir -= vFrontDir;
		vNextDir += vRightDir;

		SetRotationY(vCurRotY + DirectX::XM_PI - DirectX::XM_PIDIV4);

		// Weapon
		if (_bBindWeapon && !bEnableEditor)
		{
			_pWeapon->SetRelativeRotationX(2.340f);
			_pWeapon->SetRelativeRotationY(-0.807f);
			_pWeapon->SetRelativeRotationZ(142.585f);
			_pWeapon->SetRelativePosition(0.46f, 0.289f, 0.068f);
			_pWeapon->SetScale(0.56f, 0.56f, 0.56f);
			_pWeapon->SetOwnerRotationY(GetRotationY());

			_bLeftHand = AK_TRUE;
			_bRightHand = AK_FALSE;

			if (_bFire)
			{
				_pWeapon->SetRelativeRotationX(21.279f);
				_pWeapon->SetRelativeRotationY(-16.092f);
				_pWeapon->SetRelativeRotationZ(-166.074f);
				_pWeapon->SetRelativePosition(0.410f, 0.301f, 0.014f);

				_eAnimState = PLAYER_ANIM_STATE::PLAYER_ANIM_STATE_RIFLE_WALK_FIRE;
			}
		}
	}
	else if (PLAYER_MOVE_STATE::PLAYER_MOVE_STATE_DIAGONAL_LEFT_BACK_WALK == _eMoveState)
	{
		vNextDir -= vFrontDir;
		vNextDir -= vRightDir;

		SetRotationY(vCurRotY + DirectX::XM_PI + DirectX::XM_PIDIV4);

		// Weapon
		if (_bBindWeapon && !bEnableEditor)
		{
			_pWeapon->SetRelativeRotationX(2.340f);
			_pWeapon->SetRelativeRotationY(-0.807f);
			_pWeapon->SetRelativeRotationZ(142.585f);
			_pWeapon->SetRelativePosition(0.46f, 0.289f, 0.068f);
			_pWeapon->SetScale(0.56f, 0.56f, 0.56f);
			_pWeapon->SetOwnerRotationY(GetRotationY());

			_bLeftHand = AK_TRUE;
			_bRightHand = AK_FALSE;

			if (_bFire)
			{
				_pWeapon->SetRelativeRotationX(21.279f);
				_pWeapon->SetRelativeRotationY(-16.092f);
				_pWeapon->SetRelativeRotationZ(-166.074f);
				_pWeapon->SetRelativePosition(0.410f, 0.301f, 0.014f);

				_eAnimState = PLAYER_ANIM_STATE::PLAYER_ANIM_STATE_RIFLE_WALK_FIRE;
			}
		}
	}
	else if (PLAYER_MOVE_STATE::PLAYER_MOVE_STATE_FRONT_RUN == _eMoveState)
	{
		vNextDir += vFrontDir;

		if (_bBindWeapon && !bEnableEditor)
		{
			_pWeapon->SetRelativeRotationX(-17.809f);
			_pWeapon->SetRelativeRotationY(10.821f);
			_pWeapon->SetRelativeRotationZ(155.865f);
			_pWeapon->SetRelativePosition(0.437f, 0.293f, 0.053f);
			_pWeapon->SetScale(0.56f, 0.56f, 0.56f);
			_pWeapon->SetOwnerRotationY(GetRotationY()); // Camera Update 로 인해 추가.

			_bLeftHand = AK_TRUE;
			_bRightHand = AK_FALSE;

			if (_bFire)
			{
				_pWeapon->SetRelativeRotationX(20.093f);
				_pWeapon->SetRelativeRotationY(23.338f);
				_pWeapon->SetRelativeRotationZ(-167.379f);
				_pWeapon->SetRelativePosition(0.418f, 0.309f, 0.014f);

				_eAnimState = PLAYER_ANIM_STATE::PLAYER_ANIM_STATE_RIFLE_RUN_FIRE;
			}
		}
	}
	else if (PLAYER_MOVE_STATE::PLAYER_MOVE_STATE_BACK_RUN == _eMoveState)
	{
		vNextDir -= vFrontDir;

		SetRotationY(vCurRotY + DirectX::XM_PI);

		if (_bBindWeapon && !bEnableEditor)
		{
			_pWeapon->SetRelativeRotationX(-17.809f);
			_pWeapon->SetRelativeRotationY(10.821f);
			_pWeapon->SetRelativeRotationZ(155.865f);
			_pWeapon->SetRelativePosition(0.437f, 0.293f, 0.053f);
			_pWeapon->SetScale(0.56f, 0.56f, 0.56f);
			_pWeapon->SetOwnerRotationY(GetRotationY()); // Camera Update 로 인해 추가.

			_bLeftHand = AK_TRUE;
			_bRightHand = AK_FALSE;

			if (_bFire)
			{
				_pWeapon->SetRelativeRotationX(20.093f);
				_pWeapon->SetRelativeRotationY(23.338f);
				_pWeapon->SetRelativeRotationZ(-167.379f);
				_pWeapon->SetRelativePosition(0.418f, 0.309f, 0.014f);

				_eAnimState = PLAYER_ANIM_STATE::PLAYER_ANIM_STATE_RIFLE_RUN_FIRE;
			}
		}
	}
	else if (PLAYER_MOVE_STATE::PLAYER_MOVE_STATE_RIGHT_RUN == _eMoveState)
	{
		vNextDir += vRightDir;

		SetRotationY(vCurRotY + DirectX::XM_PIDIV2);

		if (_bBindWeapon && !bEnableEditor)
		{
			_pWeapon->SetRelativeRotationX(-17.809f);
			_pWeapon->SetRelativeRotationY(10.821f);
			_pWeapon->SetRelativeRotationZ(155.865f);
			_pWeapon->SetRelativePosition(0.437f, 0.293f, 0.053f);
			_pWeapon->SetScale(0.56f, 0.56f, 0.56f);
			_pWeapon->SetOwnerRotationY(GetRotationY()); // Camera Update 로 인해 추가.

			_bLeftHand = AK_TRUE;
			_bRightHand = AK_FALSE;

			if (_bFire)
			{
				_pWeapon->SetRelativeRotationX(20.093f);
				_pWeapon->SetRelativeRotationY(23.338f);
				_pWeapon->SetRelativeRotationZ(-167.379f);
				_pWeapon->SetRelativePosition(0.418f, 0.309f, 0.014f);

				_eAnimState = PLAYER_ANIM_STATE::PLAYER_ANIM_STATE_RIFLE_RUN_FIRE;
			}
		}
	}
	else if (PLAYER_MOVE_STATE::PLAYER_MOVE_STATE_LEFT_RUN == _eMoveState)
	{
		vNextDir -= vRightDir;

		SetRotationY(vCurRotY - DirectX::XM_PIDIV2);

		if (_bBindWeapon && !bEnableEditor)
		{
			_pWeapon->SetRelativeRotationX(-17.809f);
			_pWeapon->SetRelativeRotationY(10.821f);
			_pWeapon->SetRelativeRotationZ(155.865f);
			_pWeapon->SetRelativePosition(0.437f, 0.293f, 0.053f);
			_pWeapon->SetScale(0.56f, 0.56f, 0.56f);
			_pWeapon->SetOwnerRotationY(GetRotationY()); // Camera Update 로 인해 추가.

			_bLeftHand = AK_TRUE;
			_bRightHand = AK_FALSE;

			if (_bFire)
			{
				_pWeapon->SetRelativeRotationX(20.093f);
				_pWeapon->SetRelativeRotationY(23.338f);
				_pWeapon->SetRelativeRotationZ(-167.379f);
				_pWeapon->SetRelativePosition(0.418f, 0.309f, 0.014f);

				_eAnimState = PLAYER_ANIM_STATE::PLAYER_ANIM_STATE_RIFLE_RUN_FIRE;
			}
		}
	}
	else if (PLAYER_MOVE_STATE::PLAYER_MOVE_STATE_DIAGONAL_RIGHT_FRONT_RUN == _eMoveState)
	{
		vNextDir += vFrontDir;
		vNextDir += vRightDir;

		SetRotationY(vCurRotY + DirectX::XM_PIDIV4);

		if (_bBindWeapon && !bEnableEditor)
		{
			_pWeapon->SetRelativeRotationX(-17.809f);
			_pWeapon->SetRelativeRotationY(10.821f);
			_pWeapon->SetRelativeRotationZ(155.865f);
			_pWeapon->SetRelativePosition(0.437f, 0.293f, 0.053f);
			_pWeapon->SetScale(0.56f, 0.56f, 0.56f);
			_pWeapon->SetOwnerRotationY(GetRotationY()); // Camera Update 로 인해 추가.

			_bLeftHand = AK_TRUE;
			_bRightHand = AK_FALSE;

			if (_bFire)
			{
				_pWeapon->SetRelativeRotationX(20.093f);
				_pWeapon->SetRelativeRotationY(23.338f);
				_pWeapon->SetRelativeRotationZ(-167.379f);
				_pWeapon->SetRelativePosition(0.418f, 0.309f, 0.014f);

				_eAnimState = PLAYER_ANIM_STATE::PLAYER_ANIM_STATE_RIFLE_RUN_FIRE;
			}
		}
	}
	else if (PLAYER_MOVE_STATE::PLAYER_MOVE_STATE_DIAGONAL_LEFT_FRONT_RUN == _eMoveState)
	{
		vNextDir += vFrontDir;
		vNextDir -= vRightDir;

		SetRotationY(vCurRotY - DirectX::XM_PIDIV4);

		if (_bBindWeapon && !bEnableEditor)
		{
			_pWeapon->SetRelativeRotationX(-17.809f);
			_pWeapon->SetRelativeRotationY(10.821f);
			_pWeapon->SetRelativeRotationZ(155.865f);
			_pWeapon->SetRelativePosition(0.437f, 0.293f, 0.053f);
			_pWeapon->SetScale(0.56f, 0.56f, 0.56f);
			_pWeapon->SetOwnerRotationY(GetRotationY()); // Camera Update 로 인해 추가.

			_bLeftHand = AK_TRUE;
			_bRightHand = AK_FALSE;

			if (_bFire)
			{
				_pWeapon->SetRelativeRotationX(20.093f);
				_pWeapon->SetRelativeRotationY(23.338f);
				_pWeapon->SetRelativeRotationZ(-167.379f);
				_pWeapon->SetRelativePosition(0.418f, 0.309f, 0.014f);

				_eAnimState = PLAYER_ANIM_STATE::PLAYER_ANIM_STATE_RIFLE_RUN_FIRE;
			}
		}
	}
	else if (PLAYER_MOVE_STATE::PLAYER_MOVE_STATE_DIAGONAL_RIGHT_BACK_RUN == _eMoveState)
	{
		vNextDir -= vFrontDir;
		vNextDir += vRightDir;

		SetRotationY(vCurRotY + DirectX::XM_PI - DirectX::XM_PIDIV4);

		if (_bBindWeapon && !bEnableEditor)
		{
			_pWeapon->SetRelativeRotationX(-17.809f);
			_pWeapon->SetRelativeRotationY(10.821f);
			_pWeapon->SetRelativeRotationZ(155.865f);
			_pWeapon->SetRelativePosition(0.437f, 0.293f, 0.053f);
			_pWeapon->SetScale(0.56f, 0.56f, 0.56f);
			_pWeapon->SetOwnerRotationY(GetRotationY()); // Camera Update 로 인해 추가.

			_bLeftHand = AK_TRUE;
			_bRightHand = AK_FALSE;

			if (_bFire)
			{
				_pWeapon->SetRelativeRotationX(20.093f);
				_pWeapon->SetRelativeRotationY(23.338f);
				_pWeapon->SetRelativeRotationZ(-167.379f);
				_pWeapon->SetRelativePosition(0.418f, 0.309f, 0.014f);

				_eAnimState = PLAYER_ANIM_STATE::PLAYER_ANIM_STATE_RIFLE_RUN_FIRE;
			}
		}
	}
	else if (PLAYER_MOVE_STATE::PLAYER_MOVE_STATE_DIAGONAL_LEFT_BACK_RUN == _eMoveState)
	{
		vNextDir -= vFrontDir;
		vNextDir -= vRightDir;

		SetRotationY(vCurRotY + DirectX::XM_PI + DirectX::XM_PIDIV4);

		if (_bBindWeapon && !bEnableEditor)
		{
			_pWeapon->SetRelativeRotationX(-17.809f);
			_pWeapon->SetRelativeRotationY(10.821f);
			_pWeapon->SetRelativeRotationZ(155.865f);
			_pWeapon->SetRelativePosition(0.437f, 0.293f, 0.053f);
			_pWeapon->SetScale(0.56f, 0.56f, 0.56f);
			_pWeapon->SetOwnerRotationY(GetRotationY()); // Camera Update 로 인해 추가.

			_bLeftHand = AK_TRUE;
			_bRightHand = AK_FALSE;

			if (_bFire)
			{
				_pWeapon->SetRelativeRotationX(20.093f);
				_pWeapon->SetRelativeRotationY(23.338f);
				_pWeapon->SetRelativeRotationZ(-167.379f);
				_pWeapon->SetRelativePosition(0.418f, 0.309f, 0.014f);

				_eAnimState = PLAYER_ANIM_STATE::PLAYER_ANIM_STATE_RIFLE_RUN_FIRE;
			}
		}
	}
	else
	{
		__debugbreak();
	}


	// 방향 정규화
	vNextDir.Normalize();

	// Run 적용
	if (bSpeedUp)
	{
		fSpeed *= 3.5f;
	}
	Vector3 vNextVelocity = vNextDir * fSpeed;

	// 중력적용
	if (!_bGroundCollision)
	{
		vNextVelocity = Vector3(vNextVelocity.x, pRigidBody->GetVelocity().y, vNextVelocity.z);
	}

	// Jump
	if (pGameInput->KeyFirstDown(KEY_INPUT_SPACE))
	{
		// vNextVelocity += Vector3(0.0f, 5.0f, 0.0f);

		_eAnimState = PLAYER_ANIM_STATE::PLAYER_ANIM_STATE_JUMP;

		_bGroundCollision = AK_FALSE;
		_bJumping = AK_TRUE;
	}

	// 무기 장착 시
	if (_bBindWeapon)
	{
		Matrix* pFinalTransform = GetModel(GetModelContextIndex())->GetCurrentModelContext()->pAnimation->GetFinalTransforms();

		// Gun model default matrix.
		if (_bRightHand)
		{
			Matrix mRightHandAnimTransfrom = pFinalTransform[34].Transpose();
			_mHandAnimTransform = mRightHandAnimTransfrom;
		}
		if (_bLeftHand)
		{
			Matrix mLeftHandAnimTransform = pFinalTransform[10].Transpose();
			_mHandAnimTransform = mLeftHandAnimTransform;
		}

		_pWeapon->SetAnimTransform(&_mHandAnimTransform);
	}

	pRigidBody->SetVelocity(vNextVelocity);

	if (GetApp()->EnableEditor())
	{
		SetRotationX(0.0f);
		SetRotationY(DirectX::XM_PI);
		SetRotationZ(0.0f);

		pRigidBody->SetVelocity(Vector3(0.0f));
	}

	/*
	=======================
	Update animation.
	=======================
	*/
	AkBool bEndAnim = PlayAnimation(fDeltaTime, GAME_ANIM_PLAYER_ANIM_FILE_NAME[AkU32(_eAnimState)]);

	if (PLAYER_ANIM_STATE::PLAYER_ANIM_STATE_JUMP == _eAnimState)
	{
		if (bEndAnim)
		{
			_bJumping = AK_FALSE;
		}
	}
}

void UPlayer::FinalUpdate(const AkF32 fDeltaTime)
{
	UPlayerModel* pModel = (UPlayerModel*)GetModel(GetModelContextIndex());
	UGravity* pGravity = GetGravity();
	URigidBody* pRigidBody = GetRigidBody();
	UCollider* pCollider = GetCollider();
	UInGameCamera* pCamera = GetCamera();

	// Gravity 업데이트
	if (!_bGroundCollision)
	{
		pGravity->Update(fDeltaTime);
	}

	// Rigid Body 업데이트
	pRigidBody->Update(fDeltaTime);

	// 첫프레임 위치 보정
	Vector3 vPos = GetPosition();

	if (_bFirst)
	{
		SetPosition(vPos.x, 0.5f, vPos.z);

		_bFirst = AK_FALSE;
	}

	// Collider 업데이트
	pCollider->Update(fDeltaTime);

	// Model 의 위치 변경
	UpdateModelTransform();

	// 카메라 위치 업데이트
	if (!GetApp()->EnableEditor())
		pCamera->Update(fDeltaTime);
}

void UPlayer::RenderShadow()
{
	RenderShadowOfModel();
}

void UPlayer::Render()
{
	UApplication* pApp = GetApp();
	IRenderer* pRenderer = pApp->GetRenderer();

	// Render Model.
	RenderModel();

	// Render normal.
	if (IsDrawNoraml())
	{
		RenderNormal();
	}

	if (pApp->EnableEditor())
	{
		UCollider* pCollider = GetCollider();

		pCollider->Render();
	}
}

void UPlayer::OnCollision(UCollider* pOther)
{
	UActor* pOwner = pOther->GetOwner();
	URigidBody* pRigidBody = GetRigidBody();
	const wchar_t* wcName = pOwner->GetName();

	if (!wcscmp(L"Map", wcName))
	{
		AkTriangle_t* pTri = pOther->GetTriangle();
		AkF32 fNdotY = pTri->vNormal.Dot(Vector3(0.0f, 1.0f, 0.0f));
		if (0.0f <= fNdotY - 1.0f && fNdotY - 1.0f <= 1e-5f)
		{
			SetGroundCollision(AK_TRUE);
		}

		AkSphere_t* pSphere = GetCollider()->GetBoundingSphere();
		Vector3 vTriToSphere = pSphere->vCenter - pTri->vP[0];
		AkF32 fDistance = vTriToSphere.Dot(pTri->vNormal);
		Vector3 vProjectedCenter = pSphere->vCenter - pTri->vNormal * (fDistance - pSphere->fRadius);

		SetPosition(vProjectedCenter.x, vProjectedCenter.y, vProjectedCenter.z);
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

void UPlayer::OnCollisionEnter(UCollider* pOther)
{
	IRenderer* pRenderer = GetApp()->GetRenderer();
	UActor* pOwner = pOther->GetOwner();
	const wchar_t* wcName = pOwner->GetName();

	if (!wcscmp(L"Map", wcName))
	{
		AkTriangle_t* pTri = pOther->GetTriangle();
		AkF32 fNdotY = pTri->vNormal.Dot(Vector3(0.0f, 1.0f, 0.0f));
		if (0.0f <= fNdotY - 1.0f && fNdotY - 1.0f <= 1e-5f)
		{
			SetGroundCollision(AK_TRUE);
		}

		AkSphere_t* pSphere = GetCollider()->GetBoundingSphere();
		Vector3 vTriToSphere = pSphere->vCenter - pTri->vP[0];
		AkF32 fDistance = vTriToSphere.Dot(pTri->vNormal);
		Vector3 vProjectedCenter = pSphere->vCenter - pTri->vNormal * (fDistance - pSphere->fRadius);

		SetPosition(vProjectedCenter.x, vProjectedCenter.y, vProjectedCenter.z);
	}
	if (!wcscmp(L"Dancer", wcName))
	{
	}
	if (!wcscmp(L"BRS_74", wcName))
	{
		UWeapon* pWeapon = (UWeapon*)pOwner;
		pWeapon->AttachOwner(this);
	}
	if (!wcscmp(L"Box", wcName))
	{

	}
}

void UPlayer::OnCollisionExit(UCollider* pOther)
{
	IRenderer* pRenderer = GetApp()->GetRenderer();
	UActor* pOwner = pOther->GetOwner();
	const wchar_t* wcName = pOwner->GetName();

	if (!wcscmp(L"Map", wcName))
	{

		SetGroundCollision(AK_FALSE);

	}
}

void UPlayer::BindWeapon(UWeapon* pWeapon)
{
	_pWeapon = pWeapon;

	_bBindWeapon = AK_TRUE;
}

void UPlayer::CleanUp()
{
	DesteoyRigidBody();

	DestroyGravity();

	DestroyCollider();

	DestroyCamera();
}
