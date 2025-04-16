#include "pch.h"
#include "Ocean.h"
#include "OceanModel.h"

Ocean::Ocean()
{
	if (!Initialize())
	{
		__debugbreak();
	}
}

Ocean::Ocean(const Ocean& Other)
	: ModelObject()
{
	// Copy Model.
	_pOceanModel = Other._pOceanModel;
	_pOceanModel->AddRef();

	// Create Transform.
	_pTransform = CreateTransform();
}

Ocean::~Ocean()
{
	CleanUp();
}

AkBool Ocean::Initialize()
{
	// Create Model.
	_pOceanModel = new OceanModel;
	
	// Create Transform.
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
	_pOceanModel->UpdateWorldRow(&_pTransform->GetWorldTransform());

	_pOceanModel->Render();
}

Ocean* Ocean::Clone()
{
	return new Ocean(*this);
}

void Ocean::CleanUp()
{
	if (_pOceanModel)
	{
		_pOceanModel->Release();
		_pOceanModel = nullptr;
	}
}
