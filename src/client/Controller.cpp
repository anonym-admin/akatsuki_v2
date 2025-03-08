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
			fMoveSpeed = 3.5f;
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
		pSwat->Jumping = AK_TRUE;
		if (Swat::F_RUN == AnimState || Swat::F_WALK == AnimState)
		{
			pSwat->SetAnimation(Swat::RUN_JUMP);
		}
		if (Swat::IDLE == AnimState)
		{
			pSwat->SetAnimation(Swat::IDLE_JUMP);
		}
	}

	Vector3 vCurVector = pRigidBody->GetVelocity();

	// Walk 유지.
	if (vCurVector.Length() > 2.65f)
	{
		// 해당 코드로 걷기 애니메이션에서 오브젝트 방향과 속도 사이에 차이가 발생함.
		// 그 차이로 인해 움직임 변경.
		vCurVector.Normalize();
		vCurVector *= 2.65f;
		pRigidBody->SetVelocity(&vCurVector);
	}

	// Run 전환.
	vVelocity.Normalize();
	vVelocity *= fMoveSpeed;
	pRigidBody->AddVelocity(&vVelocity);
}

void Controller::Mouse()
{
	Swat* pSwat = (Swat*)_pOwner;
	Swat::ANIM_STATE AnimState = pSwat->AnimState;

	if (LBTN_DOWN)
	{
		pSwat->Attack = AK_TRUE;
		if (Swat::IDLE == AnimState)
		{
			pSwat->SetAnimation(Swat::PUNCHING_01);
		}
	}
}
