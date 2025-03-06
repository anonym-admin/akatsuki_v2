#pragma once

#include "Actor.h"

/*
==========
Weapon
==========
*/

class UPlayer;

class UWeapon : public UActor
{
public:
	UWeapon();
	virtual ~UWeapon();

	virtual AkBool Initialize(UApplication* pApp) = 0;
	virtual AkBool Initialize(UApplication* pApp, const Vector3* pExtent, const Vector3* pCenter) = 0;
	virtual void Update(const AkF32 fDeltaTime) = 0;
	virtual void FinalUpdate(const AkF32 fDeltaTime) = 0;
	virtual void Render() = 0;

	virtual void OnCollision(UCollider* pOther);
	virtual void OnCollisionEnter(UCollider* pOther);
	virtual void OnCollisionExit(UCollider* pOther);

	void SetOwnerRotationY(AkF32 fRot);
	void SetRelativeRotationX(AkF32 fRot);
	void SetRelativeRotationY(AkF32 fRot);
	void SetRelativeRotationZ(AkF32 fRot);
	void SetRelativePosition(AkF32 fX, AkF32 fY, AkF32 fZ);
	void SetAnimTransform(Matrix* pMat);

	void AttachOwner(UPlayer* pOwner);

protected:
	virtual void UpdateModelTransform() override;

private:
	virtual void CleanUp() = 0;

private:
	UPlayer* _pOwner = nullptr;
	Vector3 _vRelativePos = Vector3(0.0f);
	Matrix _mAnimTransform = Matrix();
	Vector3 _vRelativeRot = Vector3(0.0f);
	AkF32 _fOwnerRotY = 0.0f;
};
