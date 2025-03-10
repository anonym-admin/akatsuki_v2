#pragma once

#include "Spawn.h"

/*
========
Bullet
=======
*/

class Weapon;

class Bullet : public Spawn
{
public:
	Bullet() = default;
	Bullet(Weapon* pOwner);
	virtual ~Bullet();

	AkBool Initailize(Weapon* pOwner);
	virtual void Update() override;
	virtual void FinalUpdate() override;
	virtual void Render() override;
	virtual void RenderShadow() override;

	virtual void OnCollisionEnter(Collider* pOther) override;
	virtual void OnCollision(Collider* pOther) override;
	virtual void OnCollisionExit(Collider* pOther) override;

	virtual Bullet* Clone() override;

	void SetOwner(Weapon* pOwner) { _pOwner = pOwner; }
	void ReleaseOwner() { _pOwner = nullptr; }

	List_t tFireLink = {};

private:
	void CleanUp();

private:
	Weapon* _pOwner = nullptr;
};

