#include "pch.h"
#include "Ocean.h"

Ocean::Ocean()
{
	if (!Initialize())
	{
		__debugbreak();
	}
}

Ocean::~Ocean()
{
	CleanUp();
}

AkBool Ocean::Initialize()
{
	_pOceanObj = GRenderer->CreateOceanObject();

	_pTransform = CreateTransform();
	_pTransform->SetRotation(0.0f, DirectX::XM_PIDIV2, 0.0f);
	_pTransform->Update();

	return AK_TRUE;
}

void Ocean::Update()
{
}

void Ocean::FinalUpdate()
{
	_pTransform->Update();
}

void Ocean::Render()
{
	GRenderer->RenderOcean(_pOceanObj, GTimer->GetTotalTime(), &_pTransform->GetWorldTransform());
}

void Ocean::CleanUp()
{
	if (_pOceanObj)
	{
		_pOceanObj->Release();
		_pOceanObj = nullptr;
	}
}
