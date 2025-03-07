#include "pch.h"
#include "Gravity.h"
#include "Actor.h"
#include "RigidBody.h"

Gravity::Gravity(Actor* pOwner)
{
	if (!Initialize(pOwner))
	{
		__debugbreak();
	}
}

Gravity::~Gravity()
{
}

AkBool Gravity::Initialize(Actor* pOwner)
{
	_pOwner = pOwner;

	return AK_TRUE;
}

void Gravity::Update(const AkF32 fDeltaTime)
{
	RigidBody* pRigidBody = _pOwner->GetRigidBody();

	Vector3 vOldVelocity = pRigidBody->GetVelocity();

	Vector3 vNewVelocity = vOldVelocity + _fGforce * fDeltaTime;

	pRigidBody->SetVelocity(vNewVelocity);
}

