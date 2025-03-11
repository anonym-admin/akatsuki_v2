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
	SquareCollider(Actor* pOwner);
	~SquareCollider();

	AkBool Initialize();
	virtual AkBool RayIntersect(DirectX::SimpleMath::Ray tRay, Vector3* pOutHitPos = nullptr, AkF32* pOutDist = nullptr) override;
	virtual AkBool BoxIntersect(BoxCollider* pCollider) override;
	virtual AkBool SphereIntersect(SphereCollider* pCollider) override;
	virtual AkBool CapsuleIntersect(CapsuleCollider* pCapsule) override;

	virtual void OnCollisionEnter(Collider* pCollider) override;
	virtual void OnCollision(Collider* pCollider) override;
	virtual void OnCollisionExit(Collider* pCollider) override;

	virtual AkF32 Radius() override;

private:
	void CleanUp();
};

