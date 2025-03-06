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
	void SetPosition(const Vector3* pPos);
	void SetRotation(const Vector3* pYawPitchRoll);
	void SetOwner(Actor* pOwner);
	Vector3 GetPosition();
	Vector3 GetDirection();
	Transform* GetTransform() { return _pTransform; }

private:
	void CleanUp();

	void MoveFree();
	void MoveEditor();
	void MoveFollow();
	void RotateEditor();
	void RotateFollow();
	
private:
	Actor* _pOwner = nullptr;
	Vector3 _vOwnerInitRot = Vector3(0.0f);
	Vector3 _vCamInitPos = Vector3(0.0f);
	Vector3 _vCamFollowPos = Vector3(0.0f);
	Vector3 _vCamInitDir = Vector3(0.0f, 0.0f, 1.0f);
	Transform* _pTransform = nullptr;
	AkF32 _fCamSpeed = 1.0f;

public:
	CAMERA_MODE Mode = {};
};

