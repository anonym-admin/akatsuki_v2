#pragma once

#include "Spawn.h"

/*
=========
Casing
=========
*/

class Casing : public Spawn
{
public:
	Casing();
	virtual ~Casing();

	AkBool Initailize();
	virtual void Update() override;
	virtual void FinalUpdate() override;
	virtual void Render() override;
	virtual void RenderShadow() override;

	virtual void OnCollisionEnter(Collider* pOther) override;
	virtual void OnCollision(Collider* pOther) override;
	virtual void OnCollisionExit(Collider* pOther) override;

	virtual Casing* Clone() override;

private:
	void CleanUp();
};

