#pragma once

#include "Spawn.h"

/*
===========
Dancer
===========
*/

class Dancer : public Spawn
{
public:
	Dancer();

	AkBool Initialize();
	virtual void Update() override;
	virtual void FinalUpdate() override;
	virtual void Render() override;
	virtual void RenderShadow() override;

	virtual void OnCollisionEnter(Collider* pOther) override;
	virtual void OnCollision(Collider* pOther) override;
	virtual void OnCollisionExit(Collider* pOther) override;

	virtual Dancer* Clone() override;

private:
	void UpdateMove();
	void UpdateAnimation();
};

