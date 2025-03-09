#include "pch.h"
#include "Transform.h"

/*
==========
Transform
==========
*/

void Transform::Update()
{
	_mWorldRow = Matrix::CreateScale(_vRelativeScale) * Matrix::CreateFromYawPitchRoll(_vRelativeRotation.x, _vRelativeRotation.y, _vRelativeRotation.z) * Matrix::CreateTranslation(_vRelativePosition);

	if (_pParent)
	{
		_mWorldRow = _mWorldRow * (*_pParent);
	}

	_mWorldRow.Decompose(_vScale, _vRotation, _vPosition);
}

void Transform::Render()
{
}

void Transform::SetScale(const Vector3* pScale)
{
	_vRelativeScale = *pScale;
}

void Transform::SetRotation(const Vector3* pYawPitchRoll)
{
	_vRelativeRotation = *pYawPitchRoll;
}

void Transform::SetPosition(const Vector3* pPos)
{
	_vRelativePosition = *pPos;
}

void Transform::SetScale(AkF32 fX, AkF32 fY, AkF32 fZ)
{
	_vRelativeScale = Vector3(fX, fY, fZ);
}

void Transform::SetRotation(AkF32 fYaw, AkF32 fPitch, AkF32 fRoll)
{
	_vRelativeRotation = Vector3(fYaw, fPitch, fRoll);
}

void Transform::SetPosition(AkF32 fX, AkF32 fY, AkF32 fZ)
{
	_vRelativePosition = Vector3(fX, fY, fZ);
}

void Transform::SetFront(const Vector3* pFront)
{
	_vFront = *pFront;
}

void Transform::SetRight(const Vector3* pRight)
{
	_vRight = *pRight;
}

void Transform::SetFront(AkF32 fX, AkF32 fY, AkF32 fZ)
{
	_vFront = Vector3(fX, fY, fZ);
}

void Transform::SetRight(AkF32 fX, AkF32 fY, AkF32 fZ)
{
	_vRight = Vector3(fX, fY, fZ);
}

Vector3 Transform::GetScale()
{
	Vector3 vScale = _vRelativeScale;
	if (_pParent)
		vScale = _vScale;
	return vScale;
}

/*TODO*/
Vector4 Transform::GetRotation()
{
	// Yaw Pitch Roll.
	Vector4 vRot = Vector4(_vRelativeRotation.x, _vRelativeRotation.y, _vRelativeRotation.z, 0.0f);
	
	if (_pParent)
	{
		// Quat.
		vRot = _vRotation;
	}

	return vRot;
}

Vector3 Transform::GetPosition()
{
	Vector3 vPos = _vRelativePosition;
	if (_pParent)
		vPos = _vPosition;
	return vPos;
}

Vector3 Transform::Front()
{
	Matrix mWorldTranslationZero = _mWorldRow;
	mWorldTranslationZero.Translation(Vector3(0.0f));
	Vector3 vFront = Vector3::Transform(_vFront, mWorldTranslationZero);
	vFront.Normalize();
	return vFront;
}

Vector3 Transform::Right()
{
	Matrix mWorldTranslationZero = _mWorldRow;
	mWorldTranslationZero.Translation(Vector3(0.0f));
	Vector3 vRight = Vector3::Transform(_vRight, mWorldTranslationZero);
	vRight.Normalize();
	return vRight;
}

Vector3 Transform::Up()
{
	Matrix mWorldTranslationZero = _mWorldRow;
	mWorldTranslationZero.Translation(Vector3(0.0f));
	Vector3 vUp = Vector3::Transform(_vUp, mWorldTranslationZero);
	vUp.Normalize();
	return vUp;
}
