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
	void SetFront(const Vector3* pFront);
	void SetRight(const Vector3* pRight);
	void SetFront(AkF32 fX, AkF32 fY, AkF32 fZ);
	void SetRight(AkF32 fX, AkF32 fY, AkF32 fZ);
	void SetWorldTransform(Matrix mWorldRow);

	Vector3 GetScale();
	Vector4 GetRotation();
	Vector3 GetPosition();
	Matrix& GetWorldTransform() { return _mWorldRow; }
	Matrix* GetWorldTransformAddr() { return &_mWorldRow; }
	Vector3 Front();
	Vector3 Right();
	Vector3 Up();

private:
	Vector3 _vRelativeScale = Vector3(1.0f);
	Vector3 _vRelativeRotation = Vector3(0.0f); // Yaw Pich Roll
	Vector3 _vRelativePosition = Vector3(0.0f);

	Vector3 _vScale = Vector3(1.0f);
	Vector3 _vPosition = Vector3(0.0f);
	Quaternion _vRotation = Quaternion(); // Quat
	Vector3 _vFront = Vector3(0.0f, 0.0f, 1.0f); 
	Vector3 _vRight = Vector3(1.0f, 0.0f, 0.0f);
	Vector3 _vUp = Vector3(0.0f, 1.0f, 0.0f);
	Matrix _mWorldRow = Matrix();
	const Matrix* _pParent = nullptr;
};

