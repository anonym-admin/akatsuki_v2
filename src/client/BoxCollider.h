#pragma once

#include "Collider.h"

/*
====================
Box Collider
====================
*/

class BoxCollider : public Collider
{
public:
	BoxCollider(Actor* pOwner, const Vector3* pMin, const Vector3* pMax, const Vector3* pColor = nullptr);
	~BoxCollider();

	AkBool Initialize(const Vector3* pMin, const Vector3* pMax, const Vector3* pColor);

	virtual AkBool RayIntersect(DirectX::SimpleMath::Ray tRay, Vector3* pOutHitPos = nullptr, AkF32* pOutDist = nullptr) override;
	virtual AkBool BoxIntersect(BoxCollider* pCollider) override;
	virtual AkBool SphereIntersect(SphereCollider* pCollider) override;
	virtual AkBool CapsuleIntersect(CapsuleCollider* pCollider) override;
	AkBool SphereIntersect(const Vector3* pCenter, AkF32 fRadius);

	virtual void OnCollisionEnter(Collider* pCollider) override;
	virtual void OnCollision(Collider* pCollider) override;
	virtual void OnCollisionExit(Collider* pCollider) override;

	Vector3 GetMinWorld();
	Vector3 GetMaxWorld();

private:
	Vector3 _vMin = Vector3(0.0f);
	Vector3 _vMax = Vector3(0.0f);
};
