#pragma once

#include "Actor.h"

class UBuilding : public Actor
{
public:
	UBuilding();
	~UBuilding();

	virtual AkBool Initialize(Application* pApp);
	virtual void Update(const AkF32 fDeltaTime);
	virtual void FinalUpdate(const AkF32 fDeltaTime);
	virtual void Render();

	virtual void OnCollision(Collider* pOther);
	virtual void OnCollisionEnter(Collider* pOther);
	virtual void OnCollisionExit(Collider* pOther);

private:
	virtual void CleanUp();
};

