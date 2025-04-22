#include "pch.h"
#include "Casing.h"

/*
=========
Casing
=========
*/

Casing::Casing()
{
	if (!Initialize())
	{
		__debugbreak();
	}
}

Casing::Casing(const wchar_t* wcScript)
	: ModelObject()
{
	if (!Initialize(wcScript))
	{
		__debugbreak();
	}
}

Casing::~Casing()
{
	CleanUp();
}

AkBool Casing::Initialize()
{
	// Create Model.
	AssetMeshDataContainer_t* pMeshDataContainer = GAssetManager->GetMeshData(L"casing.mesh");
	Vector3 vAlbedo = Vector3(1.0f);
	Vector3 vEmissive = Vector3(0.0f);
	_pModel = CreateModel(pMeshDataContainer->pMeshData, pMeshDataContainer->uMeshDataNum, &vAlbedo, 0.0f, 1.0f, &vEmissive, AK_FALSE);
	_pModel->SetIBLStrength(0.25f);

	// Create Collider.
	Vector3 vMin = Vector3(0.0f);
	Vector3 vMax = Vector3(0.0f);
	CalcColliderMinMax(pMeshDataContainer->pMeshData, pMeshDataContainer->uMeshDataNum, &vMin, &vMax);
	_pCollider = CreateBoxCollider(&vMin, &vMax);

	// Create Casing Bounce Sound.
	// TODO:
	
	// Create Transform.
	_pTransform = CreateTransform();
	_pTransform->SetScale(0.025f, 0.025f, 0.025f);

	// Create Gravity
	_pGravity = CreateGravity();

	// Create Rigid Body.
	_pRigidBody = CreateRigidBody();
	_pRigidBody->SetMaxVeleocity(100.0f);
	_pRigidBody->SetFrictionCoef(0.0f);

	return AK_TRUE;
}

AkBool Casing::Initialize(const wchar_t* wcScript)
{
    return AkBool();
}

void Casing::Update()
{
}

void Casing::FinalUpdate()
{
	_pGravity->Update();

	_pRigidBody->Update();

	_pTransform->Update();
}

void Casing::Render()
{
	_pModel->UpdateWorldRow(&_pTransform->GetWorldTransform());

	_pModel->Render();
}

void Casing::RenderShadowMaps()
{
}

void Casing::OnCollisionEnter(Collider* pOther)
{
}

void Casing::OnCollision(Collider* pOther)
{
}

void Casing::OnCollisionExit(Collider* pOther)
{
}

Casing* Casing::Clone()
{
    return new Casing(*this);
}

void Casing::CleanUp()
{
}
