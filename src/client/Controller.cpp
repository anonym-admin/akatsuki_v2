#include "pch.h"
#include "Controller.h"
#include "Transform.h"
#include "RigidBody.h"
#include "Camera.h"
#include "Soldier.h"
#include "BRS_74.h"

Controller::Controller(Player* pOwner)
{
	if (!Initialize(pOwner))
	{
		__debugbreak();
	}
}

Controller::~Controller()
{
}

AkBool Controller::Initialize(Player* pOwner)
{
	_pOwner = pOwner;

	return AK_TRUE;
}

void Controller::Update()
{
	/*Attack or Fire*/
	Mouse();

	/*Move*/
	KeyBoard();
}

void Controller::Mouse()
{
	static Soldier::ANIM_STATE PrevAnimState = Soldier::IDLE;

	Soldier* pSoldier = (Soldier*)_pOwner;
	Soldier::ANIM_STATE AnimState = pSoldier->AnimState;

	if (LBTN_DOWN)
	{
		if (pSoldier->BindWeapon)
		{
			pSoldier->SetAnimation(Soldier::FIRE_STOP, 5.0f);
			pSoldier->Fire = AK_TRUE;
		}
		else
		{
			pSoldier->SetAnimation(Soldier::PUNCH_STOP);
			pSoldier->Attack = AK_TRUE;
		}
	}

	if (LBTN_HOLD)
	{
		if (pSoldier->BindWeapon)
		{
			pSoldier->SetAnimation(Soldier::FIRE_STOP, 5.0f);
			pSoldier->Fire = AK_TRUE;
		}
		else
		{
			pSoldier->SetAnimation(Soldier::PUNCH_STOP);
			pSoldier->Attack = AK_TRUE;
		}
	}

	if (LBTN_UP)
	{
		pSoldier->LBtnUp = AK_TRUE;
	}

	if (RBTN_DOWN)
	{
		if (pSoldier->BindWeapon)
		{
			pSoldier->ChangeCamera();
			pSoldier->Aim = !pSoldier->Aim;
		}
	}
}

void Controller::KeyBoard()
{
	// Motion Blur Test.
	GRenderer->SetMotionBlurScale(0.0f);

	Soldier* pSoldier = (Soldier*)_pOwner;

	if (pSoldier->Attack || pSoldier->Jumping || pSoldier->Fire)
		return;

	Soldier::ANIM_STATE AnimState = pSoldier->AnimState;
	RigidBody* pRigidBody = pSoldier->GetRigidBody();
	pRigidBody->SetMaxVeleocity(pSoldier->GetWalkSpeed());

	Vector3 vVelocity = Vector3(0.0f);
	AkF32 fMoveSpeed = 0.25f;
	AkBool bSpeedUp = AK_FALSE;

	if (KEY_HOLD(KEY_INPUT_W))
	{
		vVelocity += pSoldier->GetTransform()->Front();
		if (KEY_HOLD(KEY_INPUT_LSHIFT))
			bSpeedUp = AK_TRUE;
	}
	if (KEY_HOLD(KEY_INPUT_S))
	{
		vVelocity -= pSoldier->GetTransform()->Front();
		if (KEY_HOLD(KEY_INPUT_LSHIFT))
			bSpeedUp = AK_TRUE;
	}
	if (KEY_HOLD(KEY_INPUT_D))
	{
		vVelocity += pSoldier->GetTransform()->Right();
		if (KEY_HOLD(KEY_INPUT_LSHIFT))
			bSpeedUp = AK_TRUE;
	}
	if (KEY_HOLD(KEY_INPUT_A))
	{
		vVelocity -= pSoldier->GetTransform()->Right();
		if (KEY_HOLD(KEY_INPUT_LSHIFT))
			bSpeedUp = AK_TRUE;
	}
	if (KEY_DOWN(KEY_INPUT_SPACE))
	{
		if (Soldier::WALK <= AnimState && AnimState <= Soldier::WALK_LEFT_BDIAG)
		{
			pSoldier->Jumping = AK_TRUE;
			pSoldier->SetAnimation(Soldier::WALK_JUMP);
		}
		if (Soldier::RIFLE_IDLE == AnimState || AnimState == Soldier::FIRE_STOP)
		{
			pSoldier->Jumping = AK_TRUE;
			pSoldier->SetAnimation(Soldier::RIFLE_JUMP);
		}
	}

	vVelocity.Normalize();
	vVelocity *= fMoveSpeed;
	if (bSpeedUp)
	{
		pRigidBody->SetMaxVeleocity(pSoldier->GetRunSpeed());

		GRenderer->SetMotionBlurScale(0.8f);
	}
	pRigidBody->AddVelocity(&vVelocity);
}
