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
	static AkBool UPDATE_CAMERA;

	Camera(const Vector3* pPos, const Vector3* pYawPirchRoll);
	Camera(AkF32 fDistance, AkF32 fHeight);
	~Camera();

	AkBool Initialize(const Vector3* pPos, const Vector3* pYawPirchRoll);
	AkBool Initialize(AkF32 fDistance, AkF32 fHegith);
	void Update();
	void RenderGUI();
	void Render();
	void SetOwner(Actor* pOwner);
	Transform* GetTransform() { return _pTransform; }

private:
	void CleanUp();

	void MoveFreeMode();
	void MoveEditorMode();
	void MoveFollowMode();
	
private:
	Actor* _pOwner = nullptr;
	Transform* _pTransform = nullptr;

	AkF32 _fSpeed = 1.0f;
	AkF32 _fRotDamping = 30.0f;
	AkF32 _fMoveDamping = 20.0f;
	AkF32 _fDistance = 0.0f;
	AkF32 _fHeight = 0.0f;

public:
	CAMERA_MODE Mode = {};
};

