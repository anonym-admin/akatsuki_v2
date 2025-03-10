#pragma once

#include "Spawn.h"

/*
==========
Weapon
==========
*/

class Actor;
class Bullet;

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

	void AttachOwner(Actor* pOwner) {_pOwner = pOwner;}

	virtual Weapon* Clone() = 0;

	Bullet* CreateBullet();
	void DeleteBullet(Bullet* pBullet);

private:
	void CleanUp();

protected:
	Actor* _pOwner = nullptr;

	List_t* _pBulletHead = nullptr;
	List_t* _pBulletTail = nullptr;
};
