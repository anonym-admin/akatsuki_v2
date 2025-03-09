#include "pch.h"
#include "Actor.h"
#include "GameInput.h"
#include "Application.h"
#include "RigidBody.h"
#include "Gravity.h"
#include "Camera.h"
#include "BoxCollider.h"
#include "SphereColiider.h"
#include "CapsuleCollider.h"
#include "Animation.h"

/*
=========
Actor
=========
*/

Actor::~Actor()
{
	CleanUp();
}

Collider* Actor::CreateCollider()
{
	_pCollider = new Collider(this);
	return _pCollider;
}

New_Collider* Actor::CreateBoxColliderTest(const Vector3* pMin, const Vector3* pMax, const Vector3* pColor)
{
	Vector3 vMin = Vector3(-0.5f);
	Vector3 vMax = Vector3(0.5f);
	if (pMin)
		vMin = *pMin;
	if (pMax)
		vMax = *pMax;
	_pTest = new BoxCollider(&vMin, &vMax);
	return _pTest;
}

New_Collider* Actor::CreateSphereColliderTest(AkF32 fRadius, AkU32 uStack, AkU32 uSlice, const Vector3* pColor)
{
	_pTest = new SphereCollider(fRadius, uStack, uSlice, pColor);
	return _pTest;
}

New_Collider* Actor::CreateCapsuleColliderTest(AkF32 fRadius, AkF32 fHeight, AkU32 uStack, AkU32 uSlice, const Vector3* pColor)
{
	_pTest = new CapsuleCollider(fRadius, fHeight, uStack, uSlice, pColor);
	return _pTest;
}

void Actor::DestroyColliderTest()
{
	if (_pTest)
	{
		delete _pTest;
		_pTest = nullptr;
	}
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
	pCam->Mode = CAMERA_MODE::FREE;
	return pCam;
}

Animation* Actor::CreateAnimation(AssetMeshDataContainer_t* pMeshDataContainer, const wchar_t* wcIdleClipName, AkU32 uMaxClipNum)
{
	Animation* pAnim = new Animation(pMeshDataContainer, wcIdleClipName, uMaxClipNum);
	return pAnim;
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


	DestroyColliderTest();



	DestroyCollider();
	DesteoyRigidBody();
	DestroyGravity();
	DestroyCamera();
	DestroyAnimation();
}





