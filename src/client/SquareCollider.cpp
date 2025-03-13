#include "pch.h"
#include "SquareCollider.h"

/*
================
Square Collider
================
*/

SquareCollider::SquareCollider(AkI32 iLeft, AkI32 iTop, AkI32 iRight, AkI32 iBottom)
    : Collider(nullptr)
{
    if (!Initialize())
    {
        __debugbreak();
    }
}

SquareCollider::SquareCollider(Actor* pOwner)
    : Collider(pOwner)
{
    if (!Initialize())
    {
        __debugbreak();
    }
}

SquareCollider::~SquareCollider()
{
    CleanUp();
}

AkBool SquareCollider::Initialize(AkI32 iLeft, AkI32 iTop, AkI32 iRight, AkI32 iBottom)
{
    _eType = COLLIDER_TYPE::SQUARE;

    _iLeft = iLeft;
    _iTop = iTop;
    _iRight = iRight;
    _iBottom = iBottom;

    return AK_TRUE;
}

AkBool SquareCollider::Initialize()
{
    _eType = COLLIDER_TYPE::SQUARE;

    return AK_TRUE;
}

AkBool SquareCollider::RayIntersect(DirectX::SimpleMath::Ray tRay, Vector3* pOutHitPos, AkF32* pOutDist)
{
    return AK_FALSE;
}

AkBool SquareCollider::BoxIntersect(BoxCollider* pCollider)
{
    return AK_FALSE;
}

AkBool SquareCollider::SphereIntersect(SphereCollider* pCollider)
{
    return AK_FALSE;
}

AkBool SquareCollider::CapsuleIntersect(CapsuleCollider* pCapsule)
{
    return AK_FALSE;
}

AkBool SquareCollider::MouseIntersect() // Mouse intersect
{
    if (_iLeft < MOUSE_X && MOUSE_X < _iRight && _iTop < MOUSE_Y && MOUSE_Y < _iBottom)
    {
        return AK_TRUE;
    }
    else
    {
        return AK_FALSE;
    }
}

void SquareCollider::OnCollisionEnter(Collider* pCollider)
{
    Vector3 vColor = Vector3(1.0f, 0.0f, 0.0f);
    SetColor(&vColor);

    _pOwner->OnCollisionEnter(pCollider);
}

void SquareCollider::OnCollision(Collider* pCollider)
{
    Vector3 vColor = Vector3(1.0f, 0.0f, 0.0f);
    SetColor(&vColor);

    _pOwner->OnCollision(pCollider);
}

void SquareCollider::OnCollisionExit(Collider* pCollider)
{
    Vector3 vColor = Vector3(1.0f, 0.0f, 0.0f);
    SetColor(&vColor);

    _pOwner->OnCollisionExit(pCollider);
}

AkF32 SquareCollider::Radius()
{
    return 0.0f;
}

void SquareCollider::CleanUp()
{
}
