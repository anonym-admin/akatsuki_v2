#include "pch.h"
#include "SphereColiider.h"
#include "Transform.h"

/*
====================
Sphere Collider
====================
*/

SphereCollider::SphereCollider(Actor* pOwner, AkF32 fRadius, AkU32 uStack, AkU32 uSlice, const Vector3* pColor)
	: Collider(pOwner)
{
	if (!Initialize(fRadius, uStack, uSlice, pColor))
	{
		__debugbreak();
	}
}

SphereCollider::~SphereCollider()
{
}

AkBool SphereCollider::Initialize(AkF32 fRadius, AkU32 uStack, AkU32 uSlice, const Vector3* pColor)
{
	Vector3 vColor = Vector3(0.0f, 0.5f, 0.0f);
	if (pColor)
		vColor = *pColor;

	_eType = COLLIDER_TYPE::SPHERE;

	LineData_t* pLineSphere = GeometryGenerator::MakeSphere(fRadius, uSlice, uStack, &vColor);
	_pLineObj = GRenderer->CreateLineObject();
	_pLineObj->CreateLineBuffers(pLineSphere);
	GeometryGenerator::DestroyGeometry(pLineSphere);

	_fRadius = fRadius;

	return AK_TRUE;
}

AkBool SphereCollider::RayIntersect(DirectX::SimpleMath::Ray tRay, Vector3* pOutHitPos, AkF32* pOutDist)
{
	return AkBool();
}

AkBool SphereCollider::BoxIntersect(BoxCollider* pCollider)
{
	return AkBool();
}

AkBool SphereCollider::SphereIntersect(SphereCollider* pCollider)
{
	return AkBool();
}

AkBool SphereCollider::CapsuleIntersect(CapsuleCollider* pCapsule)
{
	return AkBool();
}

void SphereCollider::OnCollisionEnter(Collider* pCollider)
{
	_pOwner->OnCollisionEnter(pCollider);
}

void SphereCollider::OnCollision(Collider* pCollider)
{
	_pOwner->OnCollision(pCollider);
}

void SphereCollider::OnCollisionExit(Collider* pCollider)
{
	_pOwner->OnCollisionExit(pCollider);
}

AkF32 SphereCollider::Radius()
{
	Vector3 vScale = _pTransform->GetScale();
	return _fRadius * max(vScale.x, max(vScale.y, vScale.z));
}
