#include "pch.h"
#include "Camera.h"
#include "Application.h"
#include "GameInput.h"
#include "RigidBody.h"
#include "Transform.h"
#include "Timer.h"

#include "Soldier.h"

/*
===================
Camera base class
===================
*/

AkBool Camera::UPDATE_CAMERA = AK_TRUE;

Camera::Camera()
{
	_pTransform = new Transform;
}

Camera::Camera(const Vector3* pPos, const Vector3* pYawPirchRoll)
{
	if (!Initialize(pPos, pYawPirchRoll))
	{
		__debugbreak();
	}
}

Camera::Camera(AkF32 fDistance, AkF32 fHeight)
{
	if (!Initialize(fDistance, fHeight))
	{
		__debugbreak();
	}
}

Camera::~Camera()
{
	CleanUp();
}

AkBool Camera::Initialize(const Vector3* pPos, const Vector3* pYawPirchRoll)
{
	_pTransform = new Transform;

	_pTransform->SetPosition(pPos);
	_pTransform->SetRotation(pYawPirchRoll);

	GRenderer->SetCameraPosition(pPos->x, pPos->y, pPos->z);
	GRenderer->RotateYawPitchRollCamera(pYawPirchRoll->x, pYawPirchRoll->y, pYawPirchRoll->z);

	return AK_TRUE;
}

AkBool Camera::Initialize(AkF32 fDistance, AkF32 fHegith)
{
	_fDistance = fDistance;
	_fHeight = fHegith;

	_pTransform = new Transform;

	return AK_TRUE;
}

void Camera::Update()
{
	if (!UPDATE_CAMERA)
		return;

	_pTransform->Update();

	switch (Mode)
	{
	case CAMERA_MODE::FREE:
		MoveFreeMode();
		break;
	case CAMERA_MODE::EDITOR:
		MoveEditorMode();
		break;
	case CAMERA_MODE::FOLLOW:
		MoveFollowMode();
		break;
	}
}

void Camera::RenderGUI()
{
	ImGui::Begin("Camera");
	ImGui::Text("Position: %lf %lf %lf", _pTransform->GetPosition().x, _pTransform->GetPosition().y, _pTransform->GetPosition().z);
	ImGui::Text("YawPitchRoll: %lf %lf %lf", _pTransform->GetRotation().x, _pTransform->GetRotation().y, _pTransform->GetRotation().z);
	ImGui::SliderFloat("Speed", &_fSpeed, 0.0f, 100.0f);
	ImGui::End();
}

void Camera::Render()
{

}

void Camera::SetOwner(Actor* pOwner)
{
	_pOwner = pOwner;
}

void Camera::CleanUp()
{
	if (_pTransform)
	{
		delete _pTransform;
		_pTransform = nullptr;
	}
}

void Camera::MoveFreeMode()
{
	// 01. 회전
	Vector3 vTargetRot = _pOwner->GetTransform()->GetRotation();
	Vector3 vYawPitchRoll = Vector3(0.0f);

	vYawPitchRoll.x = NDC_ACC_X * DirectX::XM_PIDIV2; // Yaw
	// vYawPitchRoll.y = -NDC_Y * 1.5f; // Pitch => 1.5f => 90 degree 도달 시 Up Vector에 의한 회전 방지.

	Vector3 vCurOwnerRot = vYawPitchRoll + Vector3(DirectX::XM_PI, 0.0f, 0.0f); // DirectX::XMVectorLerp(vTargetRot, vYawPitchRoll + Vector3(DirectX::XM_PI, 0.0f, 0.0f), _fRotDamping * DT); // 해당 코드는 카메라가 따라오는 느낌을 주기위해 Lerp 를 진행
	// 게임 오브젝트는 yaw 로만 회적 적용  
	vCurOwnerRot.y = 0.0f;
	vCurOwnerRot.z = 0.0f;

	// 카메라의 회전 적용
	_pTransform->SetRotation(&vYawPitchRoll);

	// 게임오브젝트도 카메라의 방향으로 회전
	_pOwner->GetTransform()->SetRotation(&vCurOwnerRot);

	// Renderer 에 전달
	GRenderer->RotateYawPitchRollCamera(vYawPitchRoll.x, vYawPitchRoll.y, vYawPitchRoll.z);

	// 02. 이동
	Vector3 vFront = Vector3::Transform(Vector3(0.0f, 0.0f, 1.0f), Matrix::CreateFromYawPitchRoll(vYawPitchRoll.x, vYawPitchRoll.y, vYawPitchRoll.z));
	Vector3 vRight = Vector3::Transform(Vector3(1.0f, 0.0f, 0.0f), Matrix::CreateFromYawPitchRoll(vYawPitchRoll.x, vYawPitchRoll.y, vYawPitchRoll.z));

	Vector3 vTargetPos = _pOwner->GetTransform()->GetGlobalPosition();
	Vector3 vDestPos = -vFront * 1.5f + vRight * 0.25f;
	vDestPos += vTargetPos;
	vDestPos.y += 0.375f;

	Vector3 vCurPos = vDestPos; // DirectX::XMVectorLerp(_pTransform->GetPosition(), vDestPos, _fMoveDamping * DT); // 해당 코드는 카메라가 따라오는 느낌을 주기위해 Lerp 를 진행
	_pTransform->SetPosition(&vCurPos);
	GRenderer->SetCameraPosition(vCurPos.x, vCurPos.y, vCurPos.z);
}

void Camera::MoveEditorMode()
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
	_pTransform->SetPosition(&vPos);
	GRenderer->SetCameraPosition(vPos.x, vPos.y, vPos.z);

	Vector3 vYawPitchRoll = Vector3(0.0f);

	vYawPitchRoll.x = NDC_X * DirectX::XM_PI; // Yaw
	vYawPitchRoll.y = -NDC_Y * DirectX::XM_PIDIV2; // Pitch

	_pTransform->SetRotation(&vYawPitchRoll);
	GRenderer->RotateYawPitchRollCamera(vYawPitchRoll.x, vYawPitchRoll.y, vYawPitchRoll.z);
}

void Camera::MoveFollowMode()
{
	// 01. 회전
	Vector3 vTargetRot = _pOwner->GetTransform()->GetRotation();
	Vector3 vYawPitchRoll = Vector3(0.0f);

	vYawPitchRoll.x = NDC_ACC_X * DirectX::XM_PIDIV2; // Yaw
	vYawPitchRoll.y = -NDC_Y * 1.5f; // Pitch => 1.5f => 90 degree 도달 시 Up Vector에 의한 회전 방지.

	Vector3 vCurOwnerRot = vYawPitchRoll + Vector3(DirectX::XM_PI, 0.0f, 0.0f); // DirectX::XMVectorLerp(vTargetRot, vYawPitchRoll + Vector3(DirectX::XM_PI, 0.0f, 0.0f), _fRotDamping * DT); // 해당 코드는 카메라가 따라오는 느낌을 주기위해 Lerp 를 진행
	// 게임 오브젝트는 yaw 로만 회적 적용
	vCurOwnerRot.y = 0.0f;
	vCurOwnerRot.z = 0.0f;

	// 카메라의 회전 적용
	_pTransform->SetRotation(&vYawPitchRoll);

	// 게임오브젝트도 카메라의 방향으로 회전
	_pOwner->GetTransform()->SetRotation(&vCurOwnerRot); 
	
	// Renderer 에 전달
	GRenderer->RotateYawPitchRollCamera(vYawPitchRoll.x, vYawPitchRoll.y, vYawPitchRoll.z);

	// 02. 이동
	Vector3 vFront = Vector3::Transform(Vector3(0.0f, 0.0f, 1.0f), Matrix::CreateFromYawPitchRoll(vYawPitchRoll.x, vYawPitchRoll.y, vYawPitchRoll.z));

	Vector3 vTargetPos = _pOwner->GetTransform()->GetGlobalPosition();
	Vector3 vDestPos = -vFront * _fDistance;
	vDestPos += vTargetPos;
	vDestPos.y += _fHeight;

	Vector3 vCurPos = vDestPos; // DirectX::XMVectorLerp(_pTransform->GetPosition(), vDestPos, _fMoveDamping * DT); // 해당 코드는 카메라가 따라오는 느낌을 주기위해 Lerp 를 진행
	_pTransform->SetPosition(&vCurPos);
	GRenderer->SetCameraPosition(vCurPos.x, vCurPos.y, vCurPos.z);

	if (WHEEL_UP)
	{
		Vector3 vFront = _pTransform->Front();

		Vector3 vVelocity = -vFront * _fSpeed;

		Vector3 vPos = _pTransform->GetPosition();

		vPos += vVelocity * DT;

		_pTransform->SetPosition(&vPos);
	}
}



