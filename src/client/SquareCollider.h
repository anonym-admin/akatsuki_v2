#pragma once

#include "Collider.h"

/*
================
Square Collider
================
*/

class SquareCollider : public Collider
{
public:
	SquareCollider(AkI32 iLeft, AkI32 iTop, AkI32 iRight, AkI32 iBottom);
	SquareCollider(Actor* pOwner);
	~SquareCollider();

	AkBool Initialize(AkI32 iLeft, AkI32 iTop, AkI32 iRight, AkI32 iBottom);
	AkBool Initialize();
	virtual AkBool RayIntersect(DirectX::SimpleMath::Ray tRay, Vector3* pOutHitPos = nullptr, AkF32* pOutDist = nullptr) override;
	virtual AkBool BoxIntersect(BoxCollider* pCollider) override;
	virtual AkBool SphereIntersect(SphereCollider* pCollider) override;
	virtual AkBool CapsuleIntersect(CapsuleCollider* pCapsule) override;
	virtual AkBool MouseIntersect();

	virtual void OnCollisionEnter(Collider* pCollider) override;
	virtual void OnCollision(Collider* pCollider) override;
	virtual void OnCollisionExit(Collider* pCollider) override;

	virtual AkF32 Radius() override;

private:
	void CleanUp();

private:
	AkI32 _iLeft = 0;
	AkI32 _iTop = 0;
	AkI32 _iRight = 0;
	AkI32 _iBottom = 0;
};

