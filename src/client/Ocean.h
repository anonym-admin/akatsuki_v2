#pragma once

#include "ModelObject.h"

class Ocean : public ModelObject
{
public:
	Ocean();
	~Ocean();

	AkBool Initialize();
	virtual void Update() override;
	virtual void FinalUpdate() override;
	virtual void Render() override;
	virtual void RenderDepthMap() override {};
	virtual void RenderShadowMaps() override {};

private:
	void CleanUp();

private:
	IEnvironmentObject* _pOceanObj = nullptr;
};

