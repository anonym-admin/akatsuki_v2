#include "pch.h"
#include "CapsuleCollider.h"
#include "Transform.h"

/*
====================
Capsule Collider
====================
*/

CapsuleCollider::CapsuleCollider(AkF32 fRadius, AkF32 fHeight, AkU32 uStack, AkU32 uSlice, const Vector3* pColor)
{
	if (!Initialize(fRadius, fHeight, uStack, uSlice, pColor))
	{
		__debugbreak();
	}
}

CapsuleCollider::~CapsuleCollider()
{
}

AkBool CapsuleCollider::Initialize(AkF32 fRadius, AkF32 fHeight, AkU32 uStack, AkU32 uSlice, const Vector3* pColor)
{
	Vector3 vColor = Vector3(0.0f, 0.5f, 0.0f);
	if (pColor)
		vColor = *pColor;

	_eType = COLLIDER_TYPE::CAPSULE;

	LineData_t* pLineSphere = GeometryGenerator::MakeCapsule(fRadius, fHeight, uSlice, uStack, &vColor);
	_pLineObj = GRenderer->CreateLineObject();
	_pLineObj->CreateLineBuffers(pLineSphere);
	GeometryGenerator::DestroyGeometry(pLineSphere);

	return AK_TRUE;
}

AkBool CapsuleCollider::RayIntersect(DirectX::SimpleMath::Ray tRay, Vector3* pOutHitPos, AkF32* pOutDist)
{
	return AkBool();
}

AkBool CapsuleCollider::BoxIntersect(BoxCollider* pCollider)
{
	return AkBool();
}

AkBool CapsuleCollider::SphereIntersect(SphereCollider* pCollider)
{
	return AkBool();
}

AkBool CapsuleCollider::CapsuleIntersect(CapsuleCollider* pCapsule)
{
	return AkBool();
}

AkF32 CapsuleCollider::Radius()
{
	Vector3 vScale = _pTransform->GetScale();
	return _fRadius * max(vScale.x, max(vScale.y, vScale.z));
}

AkF32 CapsuleCollider::Height()
{
	Vector3 vScale = _pTransform->GetScale();
	return _fHeight * vScale.y;
}