#pragma once

#include "Spawn.h"

/*
==========
Weapon
==========
*/

class Actor;

class Weapon : public Spawn
{
public:
	Weapon();
	virtual ~Weapon();

	AkBool Initialize();
	virtual void Update() = 0;
	virtual void FinalUpdate() = 0;
	virtual void Render() = 0;
	virtual void RenderShadow() = 0;

	virtual void OnCollisionEnter(Collider* pOther) = 0;
	virtual void OnCollision(Collider* pOther) = 0;
	virtual void OnCollisionExit(Collider* pOther) = 0;

	void SetOwnerRotationY(AkF32 fRot);

	void SetRelativeRotationX(AkF32 fRot);
	void SetRelativeRotationY(AkF32 fRot);
	void SetRelativeRotationZ(AkF32 fRot);

	void SetRelativeRotation(const Vector3* pYawPitchRoll);

	void SetRelativePosition(AkF32 fX, AkF32 fY, AkF32 fZ);
	void SetRelativePosition(const Vector3* pPos);

	void SetAnimTransform(Matrix* pMat);

	void AttachOwner(Actor* pOwner) {_pOwner = pOwner;}

	virtual Weapon* Clone() = 0;

private:
	void CleanUp();

protected:
	Actor* _pOwner = nullptr;
	Vector3 _vRelativePos = Vector3(0.0f);
	Vector3 _vRelativeRot = Vector3(0.0f);
	Matrix _mAnimTransform = Matrix();
	AkF32 _fOwnerRotY = 0.0f;
};
