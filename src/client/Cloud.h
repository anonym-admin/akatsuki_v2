#pragma once

#include "ModelObject.h"

/*
=======
Cloud
=======
*/

class Cloud : public ModelObject
{
public:
	Cloud();
	~Cloud();

	AkBool Initialize();
	virtual void Update() override;
	virtual void FinalUpdate() override;
	virtual void Render() override;

private:
	void CleanUp();

private:
	IEnvironmentObject* _pCloudObj = nullptr;

	AkF32 fLightAbsorptionCoeff = 5.0f; 
	Vector3 vLightDir = Vector3(0.0f, 1.0f, 0.0f);
	AkF32 fDensityAbsorption = 10.0f;
	Vector3 vLightColor = Vector3(1.0f, 1.0f, 1.0f) * 40.0f;
	AkF32 fAniso = 0.3f;
};

