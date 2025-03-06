#pragma once

class UApplication;
class UActor;

/*
===================
Camera base class
===================
*/

class UCamera
{
public:
	virtual AkBool Initialize(UApplication* pApp) = 0;
	void SetCameraPosition(AkF32 fX, AkF32 fY, AkF32 fZ);
	void SetCameraDirection(AkF32 fYaw, AkF32 fPitch, AkF32 fRoll);
	Vector3 GetCameraPosition() { return _vCamPos; }
	Vector3 GetCameraDirection() { return _vCamCurDir; }
	
protected:
	UApplication* _pApp = nullptr;
	IRenderer* _pRenderer = nullptr;
	Vector3 _vCamInitDir = Vector3(0.0f, 0.0f, 1.0f);
	Vector3 _vCamCurDir = Vector3(0.0f);
	Vector3 _vCamPos = Vector3(0.0f);
	AkF32 _fCamSpeed = 1.0f;
};

/*
===================
EditorCamera
===================
*/

class UEditorCamera : public UCamera
{
public:
	virtual AkBool Initialize(UApplication* pApp) override;
	
	void Update(const AkF32 fDeltaTime);

private:
	AkBool _bMovaUpDown = AK_FALSE;
};

/*
===================
InGameCamera
===================
*/

class UInGameCamera : public UCamera
{
public:
	virtual AkBool Initialize(UApplication* pApp) override;
	AkBool Initialize(UApplication* pApp, UActor* pOwner, Vector3 vRelativePos);
	void Update(const AkF32 fDeltaTime);
	void Render();

	void SetOwner(UActor* pOwner) { _pOwner = pOwner; }
	void SetRalativePosition(Vector3 vRelativePos);

private:
	UActor* _pOwner = nullptr;
	Vector3 _vRelativePos = Vector3(0.0f);

	AkF32 _fOwnerInitRot = DirectX::XM_PI;
};

