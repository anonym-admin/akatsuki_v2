#include "pch.h"
#include "Swat.h"
#include "AssetManager.h"
#include "SkinnedModel.h"
#include "Animator.h"
#include "Transform.h"
#include "Controller.h"
#include "Collider.h"
#include "RigidBody.h"
#include "Camera.h"
#include "GameInput.h"

Swat::Swat()
{
    if (!Initialize())
    {
        __debugbreak();
    }
}

Swat::~Swat()
{
    CleanUp();
}

AkBool Swat::Initialize()
{
    // Create Model.
    AssetMeshDataContainer_t* pMeshDataContainer = GAssetManager->GetMeshDataContainer(ASSET_MESH_DATA_TYPE::ASSET_MESH_DATA_TYPE_SWATGUY);
    Vector3 vAlbedo = Vector3(1.0f);
    AkF32 fMetallic = 0.0f;
    AkF32 fRoughness = 1.0f;
    Vector3 vEmissive = Vector3(0.0f);
    _pModel = CreateModel(pMeshDataContainer, &vAlbedo, fMetallic, fRoughness, &vEmissive, AK_TRUE);
    GAssetManager->DeleteMeshData(ASSET_MESH_DATA_TYPE::ASSET_MESH_DATA_TYPE_SWATGUY);

    // Bind Animation.
    ((SkinnedModel*)_pModel)->BindAnimation(GAnimator->GetAnimation(GAME_ANIMATION_TYPE::GAME_ANIM_TYPE_PLAYER));

    // Create Controller.
    _pController = CreateController();

    // Create Trnasform.
    _pTransform = CreateTransform();

    // Create Collider.
    Vector3 vCenter = Vector3(0.0f);
    AkF32 fRadius = 0.5f;
    _pCollider = CreateCollider();
    _pCollider->CreateBoundingSphere(fRadius, &vCenter);

    // Create Camera.
    Vector3 vCamPos = Vector3(0.0f, 0.5f, -2.0f);
    _pCamera = CreateCamera(&vCamPos);
    _pCamera->Mode = CAMERA_MODE::FOLLOW;
    _pCamera->SetOwner(this);

    return AK_TRUE;
}

void Swat::Update()
{
    _pController->Update();

    UpdateMove();
    UpdateAnimation();
}

void Swat::FinalUpdate()
{
    _pTransform->Update();

    _pCollider->Update();

    _pCamera->Update();

    _pModel->UpdateWorldRow(&_pTransform->WorldTransform);
}

void Swat::Render()
{
    _pModel->Render();
}

void Swat::RenderShadow()
{
    _pModel->RenderShadow();
}

void Swat::OnCollisionEnter(Collider* pOther)
{
}

void Swat::OnCollision(Collider* pOther)
{
}

void Swat::OnCollisionExit(Collider* pOther)
{
}

void Swat::CleanUp()
{
}

void Swat::UpdateMove()
{

}

void Swat::UpdateAnimation()
{

}
