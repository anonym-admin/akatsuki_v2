#pragma once

/*
==============
URigidBody
==============
*/

class UActor;

class URigidBody
{
public:
	AkBool Initialize(UActor* pOwner);
	void Update(const AkF32 fDeltaTime);

	void AddForce(Vector3 vForce) { _vForce += vForce; }
	void AddVelocity(Vector3 vVelocity) { _vVelocity += vVelocity; }
	void SetMass(AkF32 fMass) { _fMass = fMass; }
	void SetVelocity(Vector3 vVelocity);
	void SetVelocity(AkF32 fX, AkF32 fY, AkF32 fZ);
	void SetMaxVeleocity(AkF32 fMaxVelocity) { _fMaxVelocity = fMaxVelocity; }
	void SetFrictionCoef(AkF32 fFricCoeff) { _fFricCoeff = fFricCoeff; }
	AkF32 GetMass() { return _fMass; }
	Vector3 GetVelocity() { return _vVelocity; }

private:
	void Move(const AkF32 fDeltaTime);

private:
	UActor* _pOwner = nullptr;
	Vector3 _vForce = Vector3(0.0f);
	Vector3 _vVelocity = Vector3(0.0f);
	AkF32 _fMaxVelocity = 100.0f;
	Vector3 _vAccel = Vector3(0.0f);
	AkF32 _fMass = 1.0f;
	AkF32 _fFricCoeff = 100.0f;
};

