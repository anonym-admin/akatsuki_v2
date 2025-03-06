#pragma once

#include "Actor.h"

class Spawn : public Actor
{
public:
	Spawn();
	virtual ~Spawn();

	virtual void Update() = 0;
	virtual void FinalUpdate() = 0;
	virtual void Render() = 0;
	virtual void RenderShadow() = 0;

	virtual void OnCollisionEnter(Collider* pOther) = 0;
	virtual void OnCollision(Collider* pOther) = 0;
	virtual void OnCollisionExit(Collider* pOther) = 0;

	virtual Spawn* Clone() = 0;

private:
	void CleanUp();
};


