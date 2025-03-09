#pragma once

#include "Collider.h"

/*
====================
Sphere Collider
====================
*/

class SphereCollider : public Collider
{
public:
	SphereCollider(Actor* pOwner, AkF32 fRadius = 0.5f, AkU32 uStack = 16, AkU32 uSlice = 32, const Vector3* pColor = nullptr);
	~SphereCollider();

	AkBool Initialize(AkF32 fRadius, AkU32 uStack, AkU32 uSlice, const Vector3* pColor);

	virtual AkBool RayIntersect(DirectX::SimpleMath::Ray tRay, Vector3* pOutHitPos = nullptr, AkF32* pOutDist = nullptr) override;
	virtual AkBool BoxIntersect(BoxCollider* pCollider) override;
	virtual AkBool SphereIntersect(SphereCollider* pCollider) override;
	virtual AkBool CapsuleIntersect(CapsuleCollider* pCapsule) override;

	virtual void OnCollisionEnter(Collider* pCollider) override;
	virtual void OnCollision(Collider* pCollider) override;
	virtual void OnCollisionExit(Collider* pCollider) override;

	AkF32 Radius();

private:
	AkF32 _fRadius = 1.0f;
};