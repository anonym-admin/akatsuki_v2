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
