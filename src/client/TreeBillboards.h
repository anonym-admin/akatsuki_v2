#pragma once

#include "Actor.h"

/*
===============
Tree Billboard
===============
*/

class Billboard : public Actor
{
public:
	Billboard(const wchar_t* wcTexArray, VertexSize_t* pVertices, AkU32 uNum);
	~Billboard();

	AkBool Initialize(const wchar_t* wcTexArray, VertexSize_t* pVertices, AkU32 uNum);
	virtual void Update() override;
	virtual void FinalUpdate() override;
	virtual void RenderShadowMaps() override;
	virtual void RenderDepthMap() override;
	virtual void Render() override;

	virtual void OnCollision(class Collider* pOther) {};
	virtual void OnCollisionEnter(class  Collider* pOther) {};
	virtual void OnCollisionExit(class Collider* pOther) {};

private:
	void CleanUp();
};

