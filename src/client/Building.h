#pragma once

#include "Actor.h"

class UBuilding : public UActor
{
public:
	UBuilding();
	~UBuilding();

	virtual AkBool Initialize(UApplication* pApp);
	virtual void Update(const AkF32 fDeltaTime);
	virtual void FinalUpdate(const AkF32 fDeltaTime);
	virtual void Render();

	virtual void OnCollision(UCollider* pOther);
	virtual void OnCollisionEnter(UCollider* pOther);
	virtual void OnCollisionExit(UCollider* pOther);

private:
	virtual void CleanUp();
};

