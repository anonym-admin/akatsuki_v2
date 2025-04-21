#include "pch.h"
#include "Grass.h"
#include "GrassModel.h"

Grass::Grass()
{
	if (!Initialize())
	{
		__debugbreak();
	}
}

Grass::Grass(const Grass& Other)
{
	// Copy Model.
	_pGrassModel = Other._pGrassModel;
	_pGrassModel->AddRef();

	// Create Transform.
	_pTransform = CreateTransform();

	// Create Culling Collider.
	Vector3 vMin = ((BoxCollider*)Other._pCullingCollider)->GetMinWorld();
	Vector3 vMax = ((BoxCollider*)Other._pCullingCollider)->GetMaxWorld();
	_pCullingCollider = CreateBoxCollider(&vMin, &vMax);
}

Grass::~Grass()
{
	CleanUp();
}

AkBool Grass::Initialize()
{
	// Grass Model Info.
	GrassModel::GrassInfo Info;
	Info.uInstanceCount = 2'500;
	Info.fWindStrength = 0.5f;

	// Create Model.
	_pGrassModel = new GrassModel(&Info);

	// Create Transform
	_pTransform = CreateTransform();

	// Create Culling Collider.
	Vector3 vMin = Vector3(0.0f);
	Vector3 vMax = Vector3(0.0f);
	_pGrassModel->GetMinMax(&vMin, &vMax);
	_pCullingCollider = CreateBoxCollider(&vMin, &vMax);

	return AK_TRUE;
}

void Grass::Update()
{
}

void Grass::FinalUpdate()
{
	_pTransform->Update();

	_pCullingCollider->Update();
}

void Grass::Render()
{
	_pGrassModel->UpdateWorldRow(&_pTransform->GetWorldTransform());

	_pGrassModel->Render();
}

void Grass::RenderGUI()
{
	ModelObject::RenderGUI();

	_pGrassModel->RenderGUI();
}

Grass* Grass::Clone()
{
	return new Grass(*this);
}

void Grass::CleanUp()
{
	if (_pGrassModel)
	{
		_pGrassModel->Release();
		_pGrassModel = nullptr;
	}
}
