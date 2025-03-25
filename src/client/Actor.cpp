#include "pch.h"
#include "Actor.h"
#include "Application.h"
#include "Camera.h"

/*
=========
Actor
=========
*/

Actor::~Actor()
{
	CleanUp();
}

Collider* Actor::CreateBoxCollider(const Vector3* pMin, const Vector3* pMax, const Vector3* pColor)
{
	Vector3 vMin = Vector3(-0.5f);
	Vector3 vMax = Vector3(0.5f);
	if (pMin)
		vMin = *pMin;
	if (pMax)
		vMax = *pMax;
	Collider* pCollider = new BoxCollider(this, &vMin, &vMax);
	return pCollider;
}

Collider* Actor::CreateSphereCollider(AkF32 fRadius, AkU32 uStack, AkU32 uSlice, const Vector3* pColor)
{
	Collider* pCollider = new SphereCollider(this, fRadius, uStack, uSlice, pColor);
	return pCollider;
}

Collider* Actor::CreateCapsuleCollider(AkF32 fRadius, AkF32 fHeight, AkU32 uStack, AkU32 uSlice, const Vector3* pColor)
{
	Collider* pCollider = new CapsuleCollider(this, fRadius, fHeight, uStack, uSlice, pColor);
	return pCollider;
}

Collider* Actor::CreateSquareCollider()
{
	Collider* pCollider = new SquareCollider(this);
	return pCollider;
}

RigidBody* Actor::CreateRigidBody()
{
	RigidBody* pRigidBody = new RigidBody(this);
	return pRigidBody;
}

Gravity* Actor::CreateGravity()
{
	Gravity* pGravity = new Gravity(this);
	return pGravity;
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

Camera* Actor::CreateCamera(AkF32 fDistance, AkF32 fHeight)
{
	Camera* pCam = new Camera(fDistance, fHeight);
	pCam->Mode = CAMERA_MODE::FOLLOW;
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

void Actor::BindAnimation(Animation* pAnim)
{
	_pAnimation = pAnim;
	if (_pModel)
	{
		((SkinnedModel*)_pModel)->BindAnimation(pAnim);
	}
}

void Actor::UnBindAnimation()
{
	_pAnimation = nullptr;
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





