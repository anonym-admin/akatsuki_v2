#include "pch.h"
#include "BRS_74.h"
#include "Application.h"
#include "EventManager.h"
#include "AssetManager.h"
#include "Collider.h"
#include "Transform.h"
#include "Model.h"

/*
=============
BRS_74
=============
*/

UBRS_74::UBRS_74()
{
	if (!Initialize())
	{
		__debugbreak();
	}
}

AkBool UBRS_74::Initialize()
{	
	// Create Model.
	AssetMeshDataContainer_t* pMeshDataContainer = GAssetManager->GetMeshDataContainer(ASSET_MESH_DATA_TYPE::ASSET_MESH_DATA_TYPE_BRS_74);
	Vector3 vAlbedo = Vector3(1.0f);
	Vector3 vEmissive = Vector3(0.0f);
	_pModel = CreateModel(pMeshDataContainer, &vAlbedo, 0.0f, 1.0f, &vEmissive, AK_FALSE);
	GAssetManager->DeleteMeshData(ASSET_MESH_DATA_TYPE::ASSET_MESH_DATA_TYPE_BRS_74);

	// Create Bounding Box.
	Vector3 vMin = Vector3(0.0f);
	Vector3 vMax = Vector3(0.25f);
	_pCollider->CreateBoundingBox(&vMin, &vMax);

	return AK_TRUE;
}

void UBRS_74::Update()
{
}

void UBRS_74::FinalUpdate()
{
	_pTransform->Update();

	_pCollider->Update();

	Matrix mFinalWorldTransform = _pOwner ? _pTransform->GetWorldTransform() * _pOwner->GetTransform()->GetWorldTransform() : _pTransform->GetWorldTransform();
	_pModel->UpdateWorldRow(&mFinalWorldTransform);
}

void UBRS_74::Render()
{
	_pModel->Render();
}

void UBRS_74::RenderShadow()
{
	_pModel->RenderShadow();
}

void UBRS_74::OnCollisionEnter(Collider* pOther)
{
	Actor* pActor = pOther->GetOwner();
	const wchar_t* wcName = pActor->Name;

	if (!wcscmp(wcName, L"Swat"))
	{
		// Bind Weapon.
		pActor->SetWeapon(this);
		AttachOwner(pActor);
	}
}

void UBRS_74::OnCollision(Collider* pOther)
{
}

void UBRS_74::OnCollisionExit(Collider* pOther)
{
}

UBRS_74* UBRS_74::Clone()
{
	Spawn::Clone();
	return new UBRS_74();
}


