#pragma once

#include "Spawn.h"

class Controller;

class Player : public Spawn
{
public:
	virtual ~Player();

	virtual void Update() = 0;
	virtual void FinalUpdate() = 0;
	virtual void Render() = 0;
	virtual void RenderShadow() = 0;

	virtual void OnCollisionEnter(Collider* pOther) = 0;
	virtual void OnCollision(Collider* pOther) = 0;
	virtual void OnCollisionExit(Collider* pOther) = 0;

	virtual Player* Clone() final { return nullptr; };

protected:
	Controller* CreateController();

private:
	void CleanUp();

protected:
	Controller* _pController;
};

