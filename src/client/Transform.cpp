#include "pch.h"
#include "Transform.h"

void Transform::Update()
{
	WorldTransform = Matrix::CreateScale(Scale) * Matrix::CreateFromYawPitchRoll(Rotation.x, Rotation.y, Rotation.z) * Matrix::CreateTranslation(Position);

	if (_pParent)
	{
		WorldTransform = *_pParent * WorldTransform;
	}
}

void Transform::Render()
{
}
