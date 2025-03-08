#include "pch.h"
#include "Dancer.h"
#include "Collider.h"
#include "RigidBody.h"
#include "Gravity.h"
#include "Application.h"
#include "AssetManager.h"
#include "SkinnedModel.h"
#include "Transform.h"

/*
===========
Dancer
===========
*/

Dancer::Dancer()
{
	if (!Initialize())
	{
		__debugbreak();
	}
}

AkBool Dancer::Initialize()
{
	//// Create Model.
	//AssetMeshDataContainer_t* pMeshDataContainer = GAssetManager->GetMeshDataContainer(ASSET_MESH_DATA_TYPE::ASSET_MESH_DATA_TYPE_DANCER);
	//Vector3 vAlbedo = Vector3(1.0f);
	//Vector3 vEmissive = Vector3(0.0f);
	//_pModel = CreateModel(pMeshDataContainer, &vAlbedo, 0.0f, 1.0f, &vEmissive, AK_TRUE);
	//GAssetManager->DeleteMeshData(ASSET_MESH_DATA_TYPE::ASSET_MESH_DATA_TYPE_DANCER);

	////// Bind Animation.
	////((SkinnedModel*)_pModel)->BindAnimation(GAnimator->GetAnimation(GAME_ANIMATION_TYPE::GAME_ANIM_TYPE_DANCER));
	////AnimState = ANIM_STATE::PLAYER_ANIM_STATE_IDLE;

	//// Create Trnasform.
	//_pTransform = CreateTransform();

	//// Create Collider.
	//Vector3 vCenter = Vector3(0.0f);
	//AkF32 fRadius = 0.5f;
	//_pCollider = CreateCollider();
	//_pCollider->CreateBoundingSphere(fRadius, &vCenter);

	//// Create Gravity
	//_pGravity = CreateGravity();

	//// Create Rigidbody
	//_pRigidBody = CreateRigidBody();
	//_pRigidBody->SetFrictionCoef(0.0f);
	//_pRigidBody->SetMaxVeleocity(10.0f);

    return AK_TRUE;
}

void Dancer::Update()
{
	// UpdateMove();
	// UpdateAnimation();
}

void Dancer::FinalUpdate()
{
	//_pRigidBody->Update();

	//_pTransform->Update();

	//_pCollider->Update();

	//_pModel->UpdateWorldRow(_pTransform->GetWorldTransformAddr());
}

void Dancer::Render()
{
	//_pModel->Render();
}

void Dancer::RenderShadow()
{
	//_pModel->RenderShadow();
}

void Dancer::OnCollision(Collider* pOther)
{

}

void Dancer::OnCollisionEnter(Collider* pOther)
{

}

void Dancer::OnCollisionExit(Collider* pOther)
{
}

Dancer* Dancer::Clone()
{
	Spawn::Clone();
	return new Dancer();
}


void Dancer::UpdateMove()
{
}

void Dancer::UpdateAnimation()
{
}
