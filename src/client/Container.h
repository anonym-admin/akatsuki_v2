#pragma once

#include "Spawn.h"

class Container : public Spawn
{
public:
	Container();
	virtual ~Container();

	AkBool Initailize();
	virtual void Update() override;
	virtual void FinalUpdate() override;
	virtual void Render() override;
	virtual void RenderShadow() override;

	virtual void OnCollisionEnter(Collider* pOther) override;
	virtual void OnCollision(Collider* pOther) override;
	virtual void OnCollisionExit(Collider* pOther) override;

	virtual Container* Clone() override;

private:
	void CleanUp();
};

