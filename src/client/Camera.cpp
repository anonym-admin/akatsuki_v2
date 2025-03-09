#include "pch.h"
#include "Camera.h"
#include "Application.h"
#include "GameInput.h"
#include "Actor.h"
#include "RigidBody.h"
#include "Transform.h"
#include "Timer.h"

/*
===================
Camera base class
===================
*/

Camera::Camera(const Vector3* pPos, const Vector3* pYawPirchRoll)
{
	Initialize(pPos, pYawPirchRoll);
}

Camera::~Camera()
{
	CleanUp();
}

AkBool Camera::Initialize(const Vector3* pPos, const Vector3* pYawPirchRoll)
{
	_pTransform = new Transform;
	_vCamInitPos = *pPos;

	SetPosition(pPos);
	SetRotation(pYawPirchRoll);
	return AK_TRUE;
}

void Camera::Update()
{
	switch (Mode)
	{
	case CAMERA_MODE::FREE:
		MoveFree();

		break;
	case CAMERA_MODE::EDITOR:
		RotateEditor();
		MoveEditor();
		break;

	case CAMERA_MODE::FOLLOW:
		RotateFollow();
		MoveFollow();
		break;
	}
}

void Camera::Render()
{

}

void Camera::SetPosition(const Vector3* pPos)
{
	_pTransform->SetPosition(pPos);
	GRenderer->SetCameraPosition(pPos->x, pPos->y, pPos->z);
}

void Camera::SetRotation(const Vector3* pYawPitchRoll)
{
	_pTransform->SetRotation(pYawPitchRoll);
	GRenderer->RotateYawPitchRollCamera(pYawPitchRoll->x, pYawPitchRoll->y, pYawPitchRoll->z);
}

void Camera::SetOwner(Actor* pOwner)
{
	_pOwner = pOwner;
}

void Camera::ToggleViewMode()
{
	_bIsView = !_bIsView;
}

Vector3 Camera::GetPosition()
{
	return _pTransform->GetPosition();
}

Vector3 Camera::GetDirection()
{
	Vector3 vYawPitchRoll = _pTransform->GetRotation();

	Vector3 vDir = Vector3::Transform(_vCamInitDir, Matrix::CreateFromYawPitchRoll(vYawPitchRoll.x, vYawPitchRoll.y, vYawPitchRoll.z));
	vDir.Normalize();
	return vDir;
}

void Camera::CleanUp()
{
	if (_pTransform)
	{
		delete _pTransform;
		_pTransform = nullptr;
	}
}

void Camera::MoveFree()
{
}

void Camera::MoveEditor()
{
	Vector3 vPos = GetPosition();
	Vector3 vDir = GetDirection();
	Vector3 vUp = Vector3(0.0f, 1.0f, 0.0f);
	Vector3 vRight = vUp.Cross(vDir);
	Vector3 vDeltaPos = Vector3(0.0f);
	vRight.Normalize();

	if (KEY_HOLD(KEY_INPUT_W))
	{
		vDeltaPos += (_fCamSpeed * vDir * DT);
	}
	if (KEY_HOLD(KEY_INPUT_S))
	{
		vDeltaPos += (_fCamSpeed * -vDir * DT);
	}
	if (KEY_HOLD(KEY_INPUT_D))
	{
		vDeltaPos += (_fCamSpeed * vRight * DT);
	}
	if (KEY_HOLD(KEY_INPUT_A))
	{
		vDeltaPos += (_fCamSpeed * -vRight * DT);
	}
	if (KEY_HOLD(KEY_INPUT_Q))
	{
		vDeltaPos += (_fCamSpeed * vUp * DT);
	}
	if (KEY_HOLD(KEY_INPUT_E))
	{
		vDeltaPos += (_fCamSpeed * -vUp * DT);
	}

	vPos += vDeltaPos;
	SetPosition(&vPos);
	GRenderer->MoveCamera(vDeltaPos.x, vDeltaPos.y, vDeltaPos.z);
}

void Camera::MoveFollow()
{
	Vector3 vTargetPos = _pOwner->GetTransform()->GetPosition();
	Vector3 vFinalPos = vTargetPos + _vCamFollowPos;

	SetPosition(&vFinalPos);
}

void Camera::RotateEditor()
{
	Vector3 vYawPitchRoll = Vector3(0.0f);

	vYawPitchRoll.x = NDC_ACC_X * DirectX::XM_PI; // Yaw
	vYawPitchRoll.y = -NDC_Y * DirectX::XM_PIDIV2; // Pitch

	SetRotation(&vYawPitchRoll);
}

void Camera::RotateFollow()
{
	static AkBool bFirst = AK_TRUE;
	if (bFirst)
	{
		_vOwnerInitRot = _pOwner->GetTransform()->GetRotation();
		bFirst = AK_FALSE;
	}

	Vector3 vYawPitchRoll = Vector3(0.0f);

	vYawPitchRoll.x = NDC_ACC_X * DirectX::XM_PIDIV2; // Yaw
	vYawPitchRoll.y = -NDC_Y * 1.5f; // Pitch => 1.5f => 90 degree 도달 시 Up Vector에 의한 회전 방지.

	_vCamFollowPos = Vector3::Transform(_vCamInitPos, Matrix::CreateFromYawPitchRoll(vYawPitchRoll.x, vYawPitchRoll.y, vYawPitchRoll.z));

	// 회전한만큼 









	SetRotation(&vYawPitchRoll);

	// 오너 애니메이션 실행.
	Vector3 vOwnerRot = _vOwnerInitRot + vYawPitchRoll;
	vOwnerRot.y = 0.0f;

	if (!_bIsView)
	{
		_pOwner->GetTransform()->SetRotation(&vOwnerRot);
	}
}

