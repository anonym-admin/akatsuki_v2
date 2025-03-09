#include "pch.h"
#include "CapsuleCollider.h"
#include "Transform.h"
#include "BoxCollider.h"

/*
====================
Capsule Collider
====================
*/

CapsuleCollider::CapsuleCollider(Actor* pOwner, AkF32 fRadius, AkF32 fHeight, AkU32 uStack, AkU32 uSlice, const Vector3* pColor)
	: Collider(pOwner)
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

	_fRadius = fRadius;
	_fHeight = fHeight;

	return AK_TRUE;
}

AkBool CapsuleCollider::RayIntersect(DirectX::SimpleMath::Ray tRay, Vector3* pOutHitPos, AkF32* pOutDist)
{
	return AkBool();
}

AkBool CapsuleCollider::BoxIntersect(BoxCollider* pCollider)
{
	Vector3 vUp = _pTransform->Up();
	Vector3 vO = _pTransform->GetPosition() - vUp * Height() * 0.5f;

	Vector3 vA = pCollider->GetTransform()->GetPosition() - vO;

	AkF32 t = vA.Dot(vUp);
	t = max(0, t);
	t = min(Height(), t);

	Vector3 vP = vO + vUp * t;

	Vector3 vPos = _pTransform->GetPosition();
	return pCollider->SphereIntersect(&vPos, Radius());
}

AkBool CapsuleCollider::SphereIntersect(SphereCollider* pCollider)
{
	return AkBool();
}

AkBool CapsuleCollider::CapsuleIntersect(CapsuleCollider* pCapsule)
{
	return AkBool();
}

void CapsuleCollider::OnCollisionEnter(Collider* pCollider)
{
	Vector3 vColor = Vector3(1.0f, 0.0f, 0.0f);
	SetColor(&vColor);

	_pOwner->OnCollisionEnter(pCollider);
}

void CapsuleCollider::OnCollision(Collider* pCollider)
{
	Vector3 vColor = Vector3(1.0f, 0.0f, 0.0f);
	SetColor(&vColor);

	_pOwner->OnCollision(pCollider);
}

void CapsuleCollider::OnCollisionExit(Collider* pCollider)
{
	Vector3 vColor = Vector3(0.0f, 0.5f, 0.0f);
	SetColor(&vColor);

	_pOwner->OnCollisionExit(pCollider);
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