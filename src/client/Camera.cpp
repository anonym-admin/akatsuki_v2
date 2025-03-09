#include "pch.h"
#include "Camera.h"
#include "Application.h"
#include "GameInput.h"
#include "RigidBody.h"
#include "Transform.h"
#include "Timer.h"
#include "Swat.h"

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

	_vRelativePos = *pPos;

	SetPosition(pPos);
	SetRotation(pYawPirchRoll);

	return AK_TRUE;
}

void Camera::Update()
{
	_pTransform->Update();

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
	Vector3 vPos = _pTransform->GetPosition();
	Vector3 vDir = _pTransform->Front();
	Vector3 vUp = Vector3(0.0f, 1.0f, 0.0f);
	Vector3 vRight = vUp.Cross(vDir);
	Vector3 vDeltaPos = Vector3(0.0f);
	vRight.Normalize();

	if (KEY_HOLD(KEY_INPUT_W))
	{
		vDeltaPos += (_fSpeed * vDir * DT);
	}
	if (KEY_HOLD(KEY_INPUT_S))
	{
		vDeltaPos += (_fSpeed * -vDir * DT);
	}
	if (KEY_HOLD(KEY_INPUT_D))
	{
		vDeltaPos += (_fSpeed * vRight * DT);
	}
	if (KEY_HOLD(KEY_INPUT_A))
	{
		vDeltaPos += (_fSpeed * -vRight * DT);
	}
	if (KEY_HOLD(KEY_INPUT_Q))
	{
		vDeltaPos += (_fSpeed * vUp * DT);
	}
	if (KEY_HOLD(KEY_INPUT_E))
	{
		vDeltaPos += (_fSpeed * -vUp * DT);
	}

	vPos += vDeltaPos;
	SetPosition(&vPos);
	GRenderer->MoveCamera(vDeltaPos.x, vDeltaPos.y, vDeltaPos.z);
}

void Camera::MoveFollow()
{
	Vector3 vTargetPos = _pOwner->GetTransform()->GetPosition();
	Vector3 vFinalPos = vTargetPos + _vFollowPos;

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
		_vOwnerInitRot = Vector3(_pOwner->GetTransform()->GetRotation().x, _pOwner->GetTransform()->GetRotation().y, _pOwner->GetTransform()->GetRotation().z);
		bFirst = AK_FALSE;
	}

	Vector3 vYawPitchRoll = Vector3(0.0f);

	vYawPitchRoll.x = NDC_ACC_X * DirectX::XM_PIDIV2; // Yaw
	vYawPitchRoll.y = -NDC_Y * 1.5f; // Pitch => 1.5f => 90 degree 도달 시 Up Vector에 의한 회전 방지.

	// 카메라 회전 후 위치 계산.
	_vFollowPos = Vector3::Transform(_vRelativePos, Matrix::CreateFromYawPitchRoll(vYawPitchRoll.x, vYawPitchRoll.y, vYawPitchRoll.z));
	SetRotation(&vYawPitchRoll);

	// 현재 플레이어의 이동방향과 카메라 방향 사이의 각도를 계산.
	//Vector3 vOwnerFront = _pOwner->GetTransform()->Front();
	//vOwnerFront.Normalize();
	//Vector3 vDir = _pTransform->Front();
	//vDir.Normalize();
	//AkF32 fCosValue = vDir.Dot(vOwnerFront);

	Vector3 vOwenrYawPitchRoll = _vOwnerInitRot + vYawPitchRoll;
	vOwenrYawPitchRoll.y = 0.0f; // pitch zero.

	Swat* pSwat = (Swat*)_pOwner;
	if (!_bIsView && Swat::IDLE <= pSwat->AnimState && pSwat->AnimState <= Swat::B_WALK)
		_pOwner->GetTransform()->SetRotation(&vOwenrYawPitchRoll);

	// TODO!!
}

