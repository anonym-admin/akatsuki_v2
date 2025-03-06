#include "pch.h"
#include "Actor.h"
#include "PlayerModel.h"
#include "GameInput.h"
#include "Application.h"
#include "RigidBody.h"
#include "Gravity.h"
#include "Camera.h"
#include "Collider.h"

Actor::~Actor()
{
	CleanUp();
}

Collider* Actor::CreateCollider()
{
	_pCollider = new Collider;
	_pCollider->Initialize(this, _pApp->GetRenderer());
	return _pCollider;
}

RigidBody* Actor::CreateRigidBody()
{
	_pRigidBody = new RigidBody;
	_pRigidBody->Initialize(this);
	return _pRigidBody;
}

Gravity* Actor::CreateGravity()
{
	_pGravity = new Gravity;
	_pGravity->Initialize(this);
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
}





