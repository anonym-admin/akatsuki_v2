#include "pch.h"
#include "BoxCollider.h"
#include "Transform.h"
#include "CapsuleCollider.h"

/*
====================
Box Collider
====================
*/

BoxCollider::BoxCollider(Actor* pOwner, const Vector3* pMin, const Vector3* pMax, const Vector3* pColor)
	: Collider(pOwner)
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

	_vMin = *pMin;
	_vMax = *pMax;

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
	Vector3 vPos = _pTransform->GetPosition();
	Vector4 vQuat = _pTransform->GetRotation();

	Matrix mTranslation = Matrix::CreateTranslation(vPos);
	Matrix mRotation = Matrix::CreateFromQuaternion(vQuat);
	
	Matrix mInvWorld = (mRotation * mTranslation).Invert();

	Vector3 vSpherePos = Vector3::Transform(*pCenter, mInvWorld); // To Model Coord.

	Vector3 vMin = _vMin * _pTransform->GetScale();
	Vector3 vMax = _vMax * _pTransform->GetScale();

	Vector3 vPoint = Vector3(0.0f);
	vPoint.x = max(vMin.x, min(vSpherePos.x, vMax.x));
	vPoint.y = max(vMin.y, min(vSpherePos.y, vMax.y));
	vPoint.z = max(vMin.z, min(vSpherePos.z, vMax.z));

	vPoint -= vSpherePos;

	return vPoint.Length() <= fRadius;
}

void BoxCollider::OnCollisionEnter(Collider* pCollider)
{
	Vector3 vColor = Vector3(1.0f, 0.0f, 0.0f);
	SetColor(&vColor);

	_pOwner->OnCollisionEnter(pCollider);
}

void BoxCollider::OnCollision(Collider* pCollider)
{
	Vector3 vColor = Vector3(1.0f, 0.0f, 0.0f);
	SetColor(&vColor);

	_pOwner->OnCollision(pCollider);
}

void BoxCollider::OnCollisionExit(Collider* pCollider)
{
	Vector3 vColor = Vector3(0.0f, 0.5f, 0.0f);
	SetColor(&vColor);

	_pOwner->OnCollisionExit(pCollider);
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



