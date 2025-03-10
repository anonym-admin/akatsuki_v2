#include "pch.h"
#include "Collider.h"

/*
===========
Collider
===========
*/

AkBool Collider::DRAW_COLLIDER;
AkU32 Collider::ID;

Collider::Collider(Actor* pOwner)
{
	if (!Initialize(pOwner))
	{
		__debugbreak();
	}
}

Collider::~Collider()
{
	CleanUp();
}

AkBool Collider::Initialize(Actor* pOwner)
{
	_iID = (AkI32)++ID;

	_pOwner = pOwner;

	_pTransform = new Transform;

	return AK_TRUE;
}

AkBool Collider::Intersect(Collider* pCollider)
{
	switch (_eType)
	{
	case COLLIDER_TYPE::BOX:
		return BoxIntersect((BoxCollider*)pCollider);
	case COLLIDER_TYPE::SPHERE:
		return BoxIntersect((BoxCollider*)pCollider);
	case COLLIDER_TYPE::CAPSULE:
		return BoxIntersect((BoxCollider*)pCollider);
	case COLLIDER_TYPE::SQUARE:
		return SqaureIntersect();
	}
	return AK_FALSE;
}

void Collider::Update()
{
	_pTransform->SetParent(&_pOwner->GetTransform()->GetWorldTransform());

	_pTransform->Update();
}

void Collider::Render()
{
	if (!_pLineObj)
		return;

	if (!DRAW_COLLIDER)
		return;

	GRenderer->RenderLineObject(_pLineObj, &_pTransform->GetWorldTransform());
}

void Collider::SetColor(const Vector3* pColor)
{
	if (!_pLineObj)
		return;

	_pLineObj->SetColor(pColor->x, pColor->y, pColor->z);
}

void Collider::CleanUp()
{
	if (_pTransform)
	{
		delete _pTransform;
		_pTransform = nullptr;
	}
	if (_pLineObj)
	{
		_pLineObj->Release();
		_pLineObj = nullptr;
	}
}
