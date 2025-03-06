#pragma once

#include "Actor.h"

class Application;

class UDancer : public Actor
{
public:
	UDancer();
	~UDancer();

	virtual AkBool Initialize(Application* pApp);
	virtual void Update(const AkF32 fDeltaTime);
	virtual void FinalUpdate(const AkF32 fDeltaTime);
	virtual void RenderShadow();
	virtual void Render();

	virtual void OnCollision(Collider* pOther);
	virtual void OnCollisionEnter(Collider* pOther);
	virtual void OnCollisionExit(Collider* pOther);

private:
	virtual void CleanUp();

private:
	AkBool _bFirst = AK_TRUE;
};

