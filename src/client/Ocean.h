#pragma once

#include "ModelObject.h"

class OceanModel;

class Ocean : public ModelObject
{
public:
	Ocean();
	Ocean(const Ocean& Other);
	~Ocean();

	AkBool Initialize();
	virtual void Update() override;
	virtual void FinalUpdate() override;
	virtual void Render() override;
	virtual void RenderDepthMap() override {};
	virtual void RenderShadowMaps() override {};

	virtual Ocean* Clone() override;

private:
	void CleanUp();

private:
	OceanModel* _pOceanModel = nullptr;
};

