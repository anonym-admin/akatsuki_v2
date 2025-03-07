#include "pch.h"
#include "Actor.h"
#include "GameInput.h"
#include "Application.h"
#include "RigidBody.h"
#include "Gravity.h"
#include "Camera.h"
#include "Collider.h"
#include "Animation.h"

Actor::~Actor()
{
	CleanUp();
}

Collider* Actor::CreateCollider()
{
	_pCollider = new Collider(this);
	return _pCollider;
}

RigidBody* Actor::CreateRigidBody()
{
	_pRigidBody = new RigidBody(this);
	return _pRigidBody;
}

Gravity* Actor::CreateGravity()
{
	_pGravity = new Gravity(this);
	return _pGravity;
}

Camera* Actor::CreateCamera(const Vector3* pPos, const Vector3* pYawPitchRoll)
{
	Vector3 vYawPitchRoll = Vector3(0.0f);
	if (!pYawPitchRoll)
		pYawPitchRoll = &vYawPitchRoll;
	Camera* pCam = new Camera(pPos, pYawPitchRoll);
	pCam->Mode = CAMERA_MODE::EDITOR;
	return pCam;
}

N_Animation* Actor::CreateAnimation(AkU32 uMaxClipNum)
{
	N_Animation* pAnim = new N_Animation(uMaxClipNum);
	return pAnim;
}

void Actor::SetAnimation(ANIM_STATE eState)
{
	if (eState != AnimState)
	{
		// printf("%u\n", (AkU32)eState);


		AnimState = eState;
		_pAnimation->PlayClip(GAME_ANIM_PLAYER_ANIM_FILE_NAME[(AkU32)eState], N_ANIM_STATE::LOOP, 0.5f);
	}
}

void Actor::DestroyCollider()
{
	if (_pCollider)
	{
		delete _pCollider;
		_pCollider = nullptr;
	}
}

void Actor::DesteoyRigidBody()
{
	if (_pRigidBody)
	{
		delete _pRigidBody;
		_pRigidBody = nullptr;
	}
}

void Actor::DestroyGravity()
{
	if (_pGravity)
	{
		delete _pGravity;
		_pGravity = nullptr;
	}
}

void Actor::DestroyCamera()
{
	if (_pCamera)
	{
		delete _pCamera;
		_pCamera = nullptr;
	}
}

void Actor::DestroyAnimation()
{
	if (_pAnimation)
	{
		delete _pAnimation;
		_pAnimation = nullptr;
	}
}

void Actor::SetWeapon(Weapon* pWeapon)
{
	BindWeapon = AK_TRUE;
	_pWeapon = pWeapon;
}

void Actor::CleanUp()
{
	AkU32 uRefCount = _uInstanceCount - 1;
	if (uRefCount)
	{
		return;
	}

	DestroyCollider();
	DesteoyRigidBody();
	DestroyGravity();
	DestroyCamera();
	DestroyAnimation();
}





