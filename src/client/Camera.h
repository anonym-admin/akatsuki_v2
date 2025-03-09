#pragma once

/*
===================
Camera base class
===================
*/

class Transform;
class Actor;

class Camera
{
public:
	Camera(const Vector3* pPos, const Vector3* pYawPirchRoll);
	~Camera();

	AkBool Initialize(const Vector3* pPos, const Vector3* pYawPirchRoll);
	void Update();
	void Render();
	void SetOwner(Actor* pOwner);
	void ToggleViewMode();
	Transform* GetTransform() { return _pTransform; }

private:
	void CleanUp();

	void MoveFree();
	void MoveEditor();
	void MoveFollow();
	void RotateEditor();
	void RotateFollow();

	void SetPosition(const Vector3* pPos);
	void SetRotation(const Vector3* pYawPitchRoll);
	
private:
	Actor* _pOwner = nullptr;
	Vector3 _vOwnerInitRot = Vector3(0.0f);

	Vector3 _vRelativePos = Vector3(0.0f);
	Vector3 _vFollowPos = Vector3(0.0f);
	Transform* _pTransform = nullptr;
	AkF32 _fSpeed = 1.0f;

	// Release for follow camera.
	AkBool _bIsView = AK_FALSE;

public:
	CAMERA_MODE Mode = {};
};

