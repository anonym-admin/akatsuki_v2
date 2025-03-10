#pragma once

#include "Actor.h"

/*
===============
Tree Billboard
===============
*/

class TreeBillboard : public Actor
{
public:
	TreeBillboard();
	~TreeBillboard();

	AkBool Initialize();
	virtual void Update() override;
	virtual void FinalUpdate() override;
	virtual void RenderShadow() {};
	virtual void Render() override;

	virtual void OnCollision(class Collider* pOther) {};
	virtual void OnCollisionEnter(class  Collider* pOther) {};
	virtual void OnCollisionExit(class Collider* pOther) {};

private:
	void CleanUp();

private:
	void* _pTreeTextureArray = nullptr;
};

