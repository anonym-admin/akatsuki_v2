#include "pch.h"
#include "Gravity.h"
#include "Actor.h"
#include "RigidBody.h"

UGravity::UGravity()
{
}

UGravity::~UGravity()
{
}

AkBool UGravity::Initialize(UActor* pOwner)
{
	_pOwner = pOwner;

	return AK_TRUE;
}

void UGravity::Update(const AkF32 fDeltaTime)
{
	URigidBody* pRigidBody = _pOwner->GetRigidBody();

	Vector3 vOldVelocity = pRigidBody->GetVelocity();

	Vector3 vNewVelocity = vOldVelocity + _fGforce * fDeltaTime;

	pRigidBody->SetVelocity(vNewVelocity);
}

