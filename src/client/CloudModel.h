#pragma once

#include "Model.h"

class CloudModel : public Model
{
public:
	CloudModel();
	~CloudModel();

	AkBool Initialize();
	virtual void Render() override;
	virtual void RenderGUI() override;

	void GetMinMax(Vector3* pOutMin, Vector3* pOutMax);

private:
	void CleanUp();

private:
	IEnvironmentObject* _pCloudObj = nullptr;

	AkF32 fLightAbsorptionCoeff = 5.0f;
	Vector3 vLightDir = Vector3(0.0f, 1.0f, 0.0f);
	AkF32 fDensityAbsorption = 10.0f;
	Vector3 vLightColor = Vector3(1.0f, 1.0f, 1.0f) * 40.0f;
	AkF32 fAniso = 0.3f;

	AkF32 fAnimSpeed = 0.0f;

	Vector3 _vMin = Vector3(0.0f);
	Vector3 _vMax = Vector3(0.0f);
};

