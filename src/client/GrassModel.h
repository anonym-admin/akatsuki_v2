#pragma once

#include "Model.h"

class GrassModel : public Model
{
public:
	struct GrassInfo
	{
		AkF32 fWindStrength = 1.0f;
		AkU32 uInstanceCount = 0;
	} _GrassInfo;

	GrassModel(GrassInfo* pInfo);
	~GrassModel();

	AkBool Initialize(GrassInfo* pInfo);
	virtual void Render() override;
	virtual void RenderGUI() override;

	void GetMinMax(Vector3* pOutMin, Vector3* pOutMax);

private:
	void CleanUp();

private:
	IMeshObject* _pGrassObj = nullptr;
	Vector3 _vMin = Vector3(AK_MAX_F32);
	Vector3 _vMax = Vector3(-AK_MAX_F32);
};

