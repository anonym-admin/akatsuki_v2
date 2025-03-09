#pragma once

#include "Collider.h"

/*
====================
Capsule Collider
====================
*/

class CapsuleCollider : public New_Collider
{
public:
	CapsuleCollider(AkF32 fRadius = 0.5f, AkF32 fHeight = 1.0f, AkU32 uStack = 16, AkU32 uSlice = 32, const Vector3* pColor = nullptr);
	~CapsuleCollider();

	AkBool Initialize(AkF32 fRadius, AkF32 fHeight, AkU32 uStack, AkU32 uSlice, const Vector3* pColor);

	virtual AkBool RayIntersect(DirectX::SimpleMath::Ray tRay, Vector3* pOutHitPos = nullptr, AkF32* pOutDist = nullptr) override;
	virtual AkBool BoxIntersect(BoxCollider* pCollider) override;
	virtual AkBool SphereIntersect(SphereCollider* pCollider) override;
	virtual AkBool CapsuleIntersect(CapsuleCollider* pCapsule) override;

	AkF32 Radius();
	AkF32 Height();

private:
	AkF32 _fRadius = 1.0f;
	AkF32 _fHeight = 2.0f;
};
