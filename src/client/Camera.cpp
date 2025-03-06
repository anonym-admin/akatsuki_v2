#include "pch.h"
#include "Camera.h"
#include "Application.h"
#include "GameInput.h"
#include "Actor.h"
#include "RigidBody.h"

/*
===================
Camera base class
===================
*/

AkBool UCamera::Initialize(UApplication* pApp)
{
	_pApp = pApp;
	_pRenderer = _pApp->GetRenderer();

	return AK_TRUE;
}

void UCamera::SetCameraPosition(AkF32 fX, AkF32 fY, AkF32 fZ)
{
	_vCamPos = Vector3(fX, fY, fZ);
	_pRenderer->SetCameraPosition(_vCamPos.x, _vCamPos.y, _vCamPos.z);
}

void UCamera::SetCameraDirection(AkF32 fYaw, AkF32 fPitch, AkF32 fRoll)
{
	_vCamCurDir = Vector3::Transform(_vCamInitDir, Matrix::CreateFromYawPitchRoll(fYaw, fPitch, fRoll));
	_pRenderer->RotateYawPitchRollCamera(fYaw, fPitch, fRoll);
}

/*
===================
EditorCamera
===================
*/

AkBool UEditorCamera::Initialize(UApplication* pApp)
{
	if (!UCamera::Initialize(pApp))
	{
		__debugbreak();
		return AK_FALSE;
	}

	// Set camera pos. 
	SetCameraPosition(0.0f, 10.0f, 0.0f);

	SetCameraDirection(0.0f, DirectX::XM_PIDIV2, 0.0f);

	return AK_TRUE;
}

void UEditorCamera::Update(const AkF32 fDeltaTime)
{
	UGameInput* pGameInput = _pApp->GetGameInput();

	if (pGameInput->KeyFirstDown(KEY_INPUT_LSHIFT))
	{
		_bMovaUpDown = !_bMovaUpDown;
	}

	// Camera 이동
	if (pGameInput->KeyHoldDown(KEY_INPUT_F5))
	{
		_pRenderer->MoveCamera(-fDeltaTime * _fCamSpeed, 0.0f, 0.0f);

		_vCamPos.x += -fDeltaTime * _fCamSpeed;
	}
	if (pGameInput->KeyHoldDown(KEY_INPUT_F6))
	{
		_pRenderer->MoveCamera(fDeltaTime * _fCamSpeed, 0.0f, 0.0f);

		_vCamPos.x += fDeltaTime * _fCamSpeed;
	}
	if (pGameInput->KeyHoldDown(KEY_INPUT_F7))
	{
		if (_bMovaUpDown)
		{
			_pRenderer->MoveCamera(0.0f, -fDeltaTime * _fCamSpeed, 0.0f);

			_vCamPos.y += -fDeltaTime * _fCamSpeed;
		}
		else
		{
			_pRenderer->MoveCamera(0.0f, 0.0f, -fDeltaTime * _fCamSpeed);

			_vCamPos.z += -fDeltaTime * _fCamSpeed;
		}
	}
	if (pGameInput->KeyHoldDown(KEY_INPUT_F8))
	{
		if (_bMovaUpDown)
		{
			_pRenderer->MoveCamera(0.0f, fDeltaTime * _fCamSpeed, 0.0f);

			_vCamPos.y += fDeltaTime * _fCamSpeed;
		}
		else
		{
			_pRenderer->MoveCamera(0.0f, 0.0f, fDeltaTime * _fCamSpeed);

			_vCamPos.z += fDeltaTime * _fCamSpeed;
		}
	}

	const AkF32 fNdcX = _pApp->GetClampNdcX();
	const AkF32 fNdcY = _pApp->GetClampNdcY();

	// Camera 회전
	AkF32 fYaw = DirectX::XM_2PI * fNdcX;
	AkF32 fPitch = -DirectX::XM_PIDIV2 * fNdcY;

	_pRenderer->RotateYawPitchRollCamera(fYaw, fPitch, 0.0f);

	Vector3 vCamDir = Vector3(0.0f, -1.0f, 0.0f);
	vCamDir = Vector3::Transform(vCamDir, Matrix::CreateFromYawPitchRoll(fYaw, fPitch, 0.0f));
	vCamDir.Normalize();
	_vCamCurDir = vCamDir;
}

/*
===================
InGameCamera
===================
*/

AkBool UInGameCamera::Initialize(UApplication* pApp)
{
	if (!UCamera::Initialize(pApp))
	{
		__debugbreak();
		return AK_FALSE;
	}

	return AK_TRUE;
}

AkBool UInGameCamera::Initialize(UApplication* pApp, UActor* pOwner, Vector3 vRelativePos)
{
	if (!UCamera::Initialize(pApp))
	{
		__debugbreak();
		return AK_FALSE;
	}
	_pOwner = pOwner;

	// Init camera.
	Vector3 vOwnerPos = pOwner->GetPosition();

	_vCamPos = vOwnerPos + vRelativePos;

	_pRenderer->SetCameraPosition(vRelativePos.x, vRelativePos.y, vRelativePos.z);

	_vRelativePos = vRelativePos;

	return AK_TRUE;
}

void UInGameCamera::Update(const AkF32 fDeltaTime)
{
	const AkF32 fNdcX = _pApp->_fNdcX;
	const AkF32 fNdcY = _pApp->GetClampNdcY();

	AkF32 fYaw = DirectX::XM_PIDIV2 * fNdcX;
	AkF32 fPitch = -1.5f * fNdcY;

	//// Dir (카메라를 오너 기준으로 회전 => 오너도 같은 방향을 향해 회전)

	//// 상대위치만큼 이동
	Vector3 vRelativePos = _vRelativePos;

	//// 카메라의 위치를 회전 시킨다.
	vRelativePos = Vector3::Transform(vRelativePos, Matrix::CreateFromYawPitchRoll(fYaw, fPitch, 0.0f));

	_pRenderer->RotateYawPitchRollCamera(fYaw, fPitch, 0.0f);

	Vector3 vCamDir = _vCamInitDir;
	vCamDir = Vector3::Transform(vCamDir, Matrix::CreateFromYawPitchRoll(fYaw, fPitch, 0.0f));
	vCamDir.Normalize();
	_vCamCurDir = vCamDir;

	// 오너의 방향도 회전
	_pOwner->SetRotationY(_fOwnerInitRot + fYaw);

	//// 카메라의 위치를 지정
	Vector3 vOwnerPos = _pOwner->GetPosition();

	_vCamPos = vOwnerPos + vRelativePos;

	_pRenderer->SetCameraPosition(_vCamPos.x, _vCamPos.y, _vCamPos.z);
}

void UInGameCamera::Render()
{
}

void UInGameCamera::SetRalativePosition(Vector3 vRelativePos)
{
	_vRelativePos = vRelativePos;
}
