#include "pch.h"
#include "OceanModel.h"

OceanModel::OceanModel()
{
	if (!Initialize())
	{
		__debugbreak();
	}
}

OceanModel::~OceanModel()
{
	CleanUp();
}

AkBool OceanModel::Initialize()
{
	_pOceanObj = GRenderer->CreateOceanObject();

	// Renderer 에서 설정된 Ocean Object Square 의 크기.
	_vMin = Vector3(-20.0f, -20.0f, 0.0f);
	_vMax = Vector3(20.0f, 20.0f, 0.0f);

	return AK_TRUE;
}

void OceanModel::Render()
{
	GRenderer->RenderOcean(_pOceanObj, GTimer->GetTotalTime(), &_mWorldRow);
}

void OceanModel::CleanUp()
{
	if (_pOceanObj)
	{
		_pOceanObj->Release();
		_pOceanObj = nullptr;
	}
}
