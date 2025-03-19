#pragma once

/*
==========
Transform
==========
*/

class Transform
{
public:
	void Update();
	void Render();

	void SetParent(const Matrix* pParent) { _pParent = pParent; }

	void SetScale(const Vector3* pScale);
	void SetRotation(const Vector3* pYawPitchRoll);
	void SetPosition(const Vector3* pPos);
	void SetScale(AkF32 fX, AkF32 fY, AkF32 fZ);
	void SetRotation(AkF32 fYaw, AkF32 fPitch, AkF32 fRoll);
	void SetPosition(AkF32 fX, AkF32 fY, AkF32 fZ);
	void SetFront(AkF32 fX, AkF32 fY, AkF32 fZ) { _vFront = Vector3(fX, fY, fZ); }
	void SetRight(AkF32 fX, AkF32 fY, AkF32 fZ) { _vRight = Vector3(fX, fY, fZ); }

	Vector3 GetScale();
	Vector3 GetRotation();
	Vector3 GetPosition();
	Vector3 GetGlobalScale() { return _vGlobalScale; }
	Quaternion GetGlobalRotation() { return _vGlobalRotation; }
	Vector3 GetGlobalPosition() { return _vGlobalPosition; }

	Matrix& GetWorldTransform() { return _mWorldRow; }
	Matrix* GetWorldTransformAddr() { return &_mWorldRow; }
	Vector3 Front();
	Vector3 Right();
	Vector3 Up();

private:
	Vector3 _vScale = Vector3(1.0f);
	Vector3 _vRotation = Vector3(0.0f); // Yaw Pich Roll
	Vector3 _vPosition = Vector3(0.0f);

	Vector3 _vGlobalScale = Vector3(1.0f);
	Quaternion _vGlobalRotation = Quaternion(); // Quat
	Vector3 _vGlobalPosition = Vector3(0.0f);

	Vector3 _vFront = Vector3(0.0f, 0.0f, 1.0f); 
	Vector3 _vRight = Vector3(1.0f, 0.0f, 0.0f);
	Vector3 _vUp = Vector3(0.0f, 1.0f, 0.0f);

	Matrix _mWorldRow = Matrix();

	const Matrix* _pParent = nullptr;
};

