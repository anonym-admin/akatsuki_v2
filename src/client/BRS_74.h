#pragma once

#include "Weapon.h"

/*
=============
BRS_74
=============
*/

class UBRS_74 : public Weapon
{
public:
	UBRS_74();

	AkBool Initialize();
	virtual void Update() override;
	virtual void FinalUpdate() override;
	virtual void Render() override;
	virtual void RenderShadow() override;

	virtual void OnCollision(Collider* pOther);
	virtual void OnCollisionEnter(Collider* pOther);
	virtual void OnCollisionExit(Collider* pOther);

	virtual UBRS_74* Clone() override;
};

