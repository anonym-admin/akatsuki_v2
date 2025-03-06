#pragma once

#include "Actor.h"

class UApplication;

class UDancer : public UActor
{
public:
	UDancer();
	~UDancer();

	virtual AkBool Initialize(UApplication* pApp);
	virtual void Update(const AkF32 fDeltaTime);
	virtual void FinalUpdate(const AkF32 fDeltaTime);
	virtual void RenderShadow();
	virtual void Render();

	virtual void OnCollision(UCollider* pOther);
	virtual void OnCollisionEnter(UCollider* pOther);
	virtual void OnCollisionExit(UCollider* pOther);

private:
	virtual void CleanUp();

private:
	AkBool _bFirst = AK_TRUE;
};

