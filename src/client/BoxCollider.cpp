#include "pch.h"
#include "BoxCollider.h"
#include "Transform.h"
#include "CapsuleCollider.h"

/*
====================
Box Collider
====================
*/

BoxCollider::BoxCollider(const Vector3* pMin, const Vector3* pMax, const Vector3* pColor)
{
	if (!Initialize(pMin, pMax, pColor))
	{
		__debugbreak();
	}
}

BoxCollider::~BoxCollider()
{
}

AkBool BoxCollider::Initialize(const Vector3* pMin, const Vector3* pMax, const Vector3* pColor)
{
	Vector3 vColor = Vector3(0.0f, 0.5f, 0.0f);
	if (pColor)
		vColor = *pColor;

	// Set type.
	_eType = COLLIDER_TYPE::BOX;

	// Create the render obj.
	LineData_t* pLineBox = GeometryGenerator::MakeCube(pMin, pMax, &vColor);
	_pLineObj = GRenderer->CreateLineObject();
	_pLineObj->CreateLineBuffers(pLineBox);
	GeometryGenerator::DestroyGeometry(pLineBox);

	return AK_TRUE;
}

AkBool BoxCollider::RayIntersect(DirectX::SimpleMath::Ray tRay, Vector3* pOutHitPos, AkF32* pOutDist)
{
	return AkBool();
}

AkBool BoxCollider::BoxIntersect(BoxCollider* pCollider)
{
	return AkBool();
}

AkBool BoxCollider::SphereIntersect(SphereCollider* pCollider)
{
	return AkBool();
}

AkBool BoxCollider::CapsuleIntersect(CapsuleCollider* pCollider)
{
	return pCollider->BoxIntersect(this);
}

AkBool BoxCollider::SphereIntersect(const Vector3* pCenter, AkF32 fRadius)
{
	return AkBool();
}

Vector3 BoxCollider::GetMinWorld()
{
	Vector3 vMinWorld = Vector3(0.0f);
	vMinWorld = Vector3::Transform(vMinWorld, _pTransform->GetWorldTransform());
	return vMinWorld;
}

Vector3 BoxCollider::GetMaxWorld()
{
	Vector3 vMaxWorld = Vector3(0.0f);
	vMaxWorld = Vector3::Transform(vMaxWorld, _pTransform->GetWorldTransform());
	return vMaxWorld;
}



