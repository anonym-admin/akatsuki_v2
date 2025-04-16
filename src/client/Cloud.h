#pragma once

#include "ModelObject.h"

/*
=======
Cloud
=======
*/

class CloudModel;

class Cloud : public ModelObject
{
public:
	Cloud();
	Cloud(const Cloud& Other);
	~Cloud();

	AkBool Initialize();
	virtual void Update() override;
	virtual void FinalUpdate() override;
	virtual void Render() override;
	virtual void RenderDepthMap() override {};
	virtual void RenderShadowMaps() override {};

	virtual void RenderGUI() override;

	virtual Cloud* Clone() override;

private:
	void CleanUp();

private:
	CloudModel* _pCloudModel = nullptr;
};

