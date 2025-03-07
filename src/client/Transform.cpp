#include "pch.h"
#include "Transform.h"

void Transform::Update()
{
	_mWorldRow = Matrix::CreateScale(_vScale) * Matrix::CreateFromYawPitchRoll(_vRotation.x, _vRotation.y, _vRotation.z) * Matrix::CreateTranslation(_vPosition);

	if (_pParent)
	{
		_mWorldRow = _mWorldRow * (*_pParent);
	}
}

void Transform::Render()
{
}

void Transform::SetScale(const Vector3* pScale)
{
	_vScale = *pScale;
}

void Transform::SetRotation(const Vector3* pYawPitchRoll)
{
	_vRotation = *pYawPitchRoll;
}

void Transform::SetPosition(const Vector3* pPos)
{
	_vPosition = *pPos;
}

void Transform::SetScale(AkF32 fX, AkF32 fY, AkF32 fZ)
{
	_vScale = Vector3(fX, fY, fZ);
}

void Transform::SetRotation(AkF32 fYaw, AkF32 fPitch, AkF32 fRoll)
{
	_vRotation = Vector3(fYaw, fPitch, fRoll);
}

void Transform::SetPosition(AkF32 fX, AkF32 fY, AkF32 fZ)
{
	_vPosition = Vector3(fX, fY, fZ);
}
