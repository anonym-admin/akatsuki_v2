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
	Soldier* pSwat = (Soldier*)_pOwner;

	if (pSwat->Attack || pSwat->Jumping || pSwat->Fire)
		return;

	Soldier::ANIM_STATE AnimState = pSwat->AnimState;
	RigidBody* pRigidBody = pSwat->GetRigidBody();

	Vector3 vVelocity = Vector3(0.0f);
	AkF32 fMoveSpeed = 0.15f;

	if (KEY_HOLD(KEY_INPUT_W))
	{
		vVelocity += pSwat->GetTransform()->Front();

		//Vector3 vFront = pSwat->GetTransform()->Front();
		//if (KEY_HOLD(KEY_INPUT_LSHIFT))
		//{
		//	vVelocity *= pSwat->GetRunSpeed();
		//	pRigidBody->SetMaxVeleocity(pSwat->GetRunSpeed());
		//}
	}
	if (KEY_HOLD(KEY_INPUT_S))
	{
		vVelocity -= pSwat->GetTransform()->Front();
	}
	if (KEY_HOLD(KEY_INPUT_D))
	{
		// 게임 오브젝트를 90도 만큼 회전시킨다.
		vVelocity += pSwat->GetTransform()->Right();
	}
	if (KEY_HOLD(KEY_INPUT_A))
	{
		vVelocity -= pSwat->GetTransform()->Right();
	}
	if (KEY_DOWN(KEY_INPUT_V))
	{
		// TODO...
	}
	if (KEY_DOWN(KEY_INPUT_SPACE))
	{
		//if (Soldier::F_RUN == AnimState || Soldier::F_WALK == AnimState)
		//{
		//	pSwat->Jumping = AK_TRUE;
		//	pSwat->SetAnimation(Soldier::RUN_JUMP);
		//}
		//if (Soldier::IDLE == AnimState || Soldier::RIFLE_IDLE == AnimState)
		//{
		//	pSwat->Jumping = AK_TRUE;
		//	pSwat->SetAnimation(Soldier::IDLE_JUMP);
		//}
	}

	vVelocity.Normalize();
	vVelocity *= fMoveSpeed;
	pRigidBody->AddVelocity(&vVelocity);
}
