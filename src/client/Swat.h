#pragma once

#include "Player.h"

class Weapon;

class Swat : public Player
{
public:
	Swat();
	virtual ~Swat();

	AkBool Initialize();
	virtual void Update() override;
	virtual void FinalUpdate() override;
	virtual void Render() override;
	virtual void RenderShadow() override;

	virtual void OnCollisionEnter(Collider* pOther) override;
	virtual void OnCollision(Collider* pOther) override;
	virtual void OnCollisionExit(Collider* pOther) override;

private:
	void CleanUp();

	void Idle();
	void UpdateMove();
	void UpdateFire();
	void UpdateAnimation();
	void UpdateWeapon();

private:
	Matrix _mHandAnimTransform = Matrix();
};

