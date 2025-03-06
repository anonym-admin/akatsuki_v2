#include "pch.h"
#include "RigidBody.h"
#include "Actor.h"

/*
==============
URigidBody
==============
*/

AkBool URigidBody::Initialize(UActor* pOwner)
{
    _pOwner = pOwner;

    return AK_TRUE;
}

void URigidBody::Update(const AkF32 fDeltaTime)
{
    AkF32 fForce = _vForce.Length();

    if (0.0f != fForce)
    {
        _vForce.Normalize();

        AkF32 fAccel = fForce / _fMass;

        _vAccel = _vForce * fAccel;

        _vVelocity += _vAccel * fDeltaTime;
    }

    if (_vVelocity.Length() >= 0.0f)
    {
        Vector3 vVelocityDir = _vVelocity;
        vVelocityDir.Normalize();

        Vector3 vFriction = -vVelocityDir * _fFricCoeff * fDeltaTime;
        if (_vVelocity.Length() < vFriction.Length())
        {
            _vVelocity = Vector3(0.0f);
        }
        else
        {
            _vVelocity += vFriction;
        }
    }

    if (_fMaxVelocity < _vVelocity.Length())
    {
        _vVelocity.Normalize();
        _vVelocity *= _fMaxVelocity;
    }

    Move(fDeltaTime);

    _vForce = Vector3(0.0f);
}

void URigidBody::SetVelocity(Vector3 vVelocity)
{
    if (vVelocity.Length() > _fMaxVelocity)
    {
        vVelocity.Normalize();
        vVelocity *= _fMaxVelocity;
    }

    _vVelocity = vVelocity;
}

void URigidBody::SetVelocity(AkF32 fX, AkF32 fY, AkF32 fZ)
{
    _vVelocity = Vector3(fX, fY, fZ);
}

void URigidBody::Move(const AkF32 fDeltaTime)
{
    const AkF32 fSpeed = _vVelocity.Length();

    if (0.0f != fSpeed)
    {
        Vector3 vDir = _vVelocity;
        vDir.Normalize();

        Vector3 vPos = _pOwner->GetPosition();

        vPos += _vVelocity * fDeltaTime;

        _pOwner->SetPosition(vPos.x, vPos.y, vPos.z);
    }
}
