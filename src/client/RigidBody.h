#pragma once

/*
==============
URigidBody
==============
*/

class Actor;

class RigidBody
{
public:
	RigidBody(Actor* pOwner);

	AkBool Initialize(Actor* pOwner);
	void Update();

	void AddForce(const Vector3* pForce) { _vForce += *pForce; }
	void AddVelocity(const Vector3* pVelocity) { _vVelocity += *pVelocity; }
	void SetVelocity(const Vector3* pVelocity);
	void SetMaxVeleocity(AkF32 fMaxVelocity) { _fMaxVelocity = fMaxVelocity; }
	void SetFrictionCoef(AkF32 fFricCoeff) { _fFricCoeff = fFricCoeff; }
	void SetMass(AkF32 fMass) { _fMass = fMass; }
	Vector3 GetVelocity() { return _vVelocity; }
	AkF32 GetMaxVelocity() { return _fMaxVelocity; }
	AkF32 GetMass() { return _fMass; }

private:
	void Move();

private:
	Actor* _pOwner = nullptr;
	Vector3 _vForce = Vector3(0.0f);
	Vector3 _vVelocity = Vector3(0.0f);
	AkF32 _fMaxVelocity = 100.0f;
	Vector3 _vAccel = Vector3(0.0f);
	AkF32 _fMass = 1.0f;
	AkF32 _fFricCoeff = 100.0f;
};

