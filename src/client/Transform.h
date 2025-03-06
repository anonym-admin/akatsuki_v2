#pragma once

class Transform
{
public:
	void Update();
	void Render();

	void SetParent(const Matrix* pParent) { _pParent = pParent; }

	Vector3 Scale = Vector3(1.0f);
	Vector3 Position = Vector3(0.0f);
	Vector3 Rotation = Vector3(0.0f); // Yaw Pich Roll

	Matrix WorldTransform = Matrix();

private:
	const Matrix* _pParent = nullptr;
};

