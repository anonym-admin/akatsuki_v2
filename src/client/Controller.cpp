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
	KeyBoard();
	Mouse();
}

void Controller::KeyBoard()
{
	Swat* pSwat = (Swat*)_pOwner;
	Swat::ANIM_STATE AnimState = pSwat->AnimState;
	RigidBody* pRigidBody = pSwat->GetRigidBody();

	Vector3 vVelocity = Vector3(0.0f);
	AkF32 fMoveSpeed = 1.5f;
	if (KEY_HOLD(KEY_INPUT_W))
	{
		vVelocity += pSwat->GetTransform()->Front();
		if (KEY_HOLD(KEY_INPUT_LSHIFT))
		{
			fMoveSpeed = 5.5f;
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

	vVelocity.Normalize();
	vVelocity *= fMoveSpeed;
	pRigidBody->SetVelocity(&vVelocity);
}

void Controller::Mouse()
{
	// Fire
	if (_pOwner->BindWeapon)
	{
		if (LBTN_DOWN)
		{
		}
		if (LBTN_HOLD)
		{
		}
	}

}
