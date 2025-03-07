#pragma once

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

	Vector3 GetScale() { return _vScale; }
	Vector3 GetRotation() { return _vRotation; }
	Vector3 GetPosition() { return _vPosition; }
	Matrix& GetWorldTransform() { return _mWorldRow; }
	Matrix* GetWorldTransformAddr() { return &_mWorldRow; }

private:
	TRANSFORM_MODE Mode = TRANSFORM_MODE::YPR;
	Vector3 _vScale = Vector3(1.0f);
	Vector3 _vPosition = Vector3(0.0f);
	Vector3 _vRotation = Vector3(0.0f); // Yaw Pich Roll
	Matrix _mWorldRow = Matrix();
	const Matrix* _pParent = nullptr;
};

