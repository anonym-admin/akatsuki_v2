#pragma once

#include "Weapon.h"

/*
=============
BRS_74
=============
*/

class UBRS_74 : public UWeapon
{
public:
	UBRS_74();
	~UBRS_74();

	virtual AkBool Initialize(Application* pApp);
	virtual AkBool Initialize(Application* pApp, const Vector3* pExtent, const Vector3* pCenter);
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
};

