#include "pch.h"
#include "Controller.h"
#include "Transform.h"
#include "RigidBody.h"
#include "Camera.h"
#include "Swat.h"

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
	static Swat::ANIM_STATE PrevAnimState = Swat::IDLE;

	Swat* pSwat = (Swat*)_pOwner;
	Swat::ANIM_STATE AnimState = pSwat->AnimState;

	if (LBTN_DOWN)
	{
		if (Swat::IDLE == AnimState)
		{
			pSwat->Attack = AK_TRUE;
			if (Swat::IDLE == PrevAnimState)
			{
				pSwat->SetAnimation(Swat::PUNCHING_01);
				PrevAnimState = Swat::PUNCHING_01;

			}
			else if (Swat::PUNCHING_01 == PrevAnimState)
			{
				pSwat->SetAnimation(Swat::PUNCHING_02);
				PrevAnimState = Swat::IDLE; // Return Idle.
			}
		}
	}
	if (LBTN_HOLD)
	{
		if (Swat::RIFLE_IDLE == AnimState)
		{
			pSwat->Fire = AK_TRUE;
			pSwat->SetAnimation(Swat::RIFLE_FIRE);
		}
	}
}

void Controller::KeyBoard()
{
	Swat* pSwat = (Swat*)_pOwner;

	if (pSwat->Attack || pSwat->Jumping)
		return;

	Swat::ANIM_STATE AnimState = pSwat->AnimState;
	RigidBody* pRigidBody = pSwat->GetRigidBody();

	Vector3 vVelocity = Vector3(0.0f);
	AkF32 fMoveSpeed = 0.15f;

	if (KEY_HOLD(KEY_INPUT_W))
	{
		vVelocity += pSwat->GetTransform()->Front();
		if (KEY_HOLD(KEY_INPUT_LSHIFT))
		{
			vVelocity *= pSwat->GetRunSpeed();
			pRigidBody->SetMaxVeleocity(pSwat->GetRunSpeed());
		}
	}
	if (KEY_HOLD(KEY_INPUT_S))
	{
		vVelocity -= pSwat->GetTransform()->Front();
	}
	if (KEY_HOLD(KEY_INPUT_D))
	{
		vVelocity += pSwat->GetTransform()->Right();
	}
	if (KEY_HOLD(KEY_INPUT_A))
	{
		vVelocity -= pSwat->GetTransform()->Right();
	}
	if (KEY_DOWN(KEY_INPUT_V))
	{
		pSwat->GetCamera()->ToggleViewMode();
	}
	if (KEY_DOWN(KEY_INPUT_SPACE))
	{
		if (Swat::F_RUN == AnimState || Swat::F_WALK == AnimState)
		{
			pSwat->Jumping = AK_TRUE;
			pSwat->SetAnimation(Swat::RUN_JUMP);
		}
		if (Swat::IDLE == AnimState || Swat::RIFLE_IDLE == AnimState)
		{
			pSwat->Jumping = AK_TRUE;
			pSwat->SetAnimation(Swat::IDLE_JUMP);
		}
	}

	vVelocity.Normalize();
	vVelocity *= fMoveSpeed;
	pRigidBody->AddVelocity(&vVelocity);
}
