#pragma once

#include "ModelObject.h"

/*
=========
Casing
=========
*/

class Casing : public ModelObject
{
public:
	Casing();
	Casing(const wchar_t* wcScript);
	virtual ~Casing();

	AkBool Initialize();
	AkBool Initialize(const wchar_t* wcScript);
	virtual void Update() override;
	virtual void FinalUpdate() override;
	virtual void Render() override;
	virtual void RenderShadowMaps() override;

	virtual void OnCollisionEnter(Collider* pOther) override;
	virtual void OnCollision(Collider* pOther) override;
	virtual void OnCollisionExit(Collider* pOther) override;

	virtual Casing* Clone() override;

private:
	void CleanUp();
};

