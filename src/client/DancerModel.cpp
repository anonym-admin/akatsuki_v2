#include "pch.h"
#include "DancerModel.h"
#include "Application.h"
#include "Animation.h"
#include "AssetManager.h"

UDancerModel::UDancerModel()
{
}

UDancerModel::~UDancerModel()
{
    CleanUp();
}

AkBool UDancerModel::Initialize(UApplication* pApp)
{
    if (!UModel::Initialize(pApp))
    {
        __debugbreak();
        return AK_FALSE;
    }

    _pApp = pApp;
    _pRenderer = pApp->GetRenderer();

    UAssetManager* pAssetManager = _pApp->GetAssetManager();
    UAnimator* pAnimator = _pApp->GetAnimator();

    if (CreateSkinnedMeshObject())
    {
        // Mesh Data.
        AssetMeshDataContainer_t* pMeshDataContainer = pAssetManager->GetMeshDataContainer(ASSET_MESH_DATA_TYPE::ASSET_MESH_DATA_TYPE_DANCER);
        MeshData_t* pDancerMeshData = pMeshDataContainer->pMeshData;
        AkU32 uDancerMeshDataNum = pMeshDataContainer->uMeshDataNum;
        CreateMeshBuffersForDraw(pDancerMeshData, uDancerMeshDataNum);

        // Material.
        IMeshObject* pMeshObj = GetRenderObject();
        Vector3 vAlbedoFactor = Vector3(1.0f);
        AkF32 fMetallicFactor = 0.0f;
        AkF32 fRoughnessFactor = 1.0f;
        Vector3 vEmissiveFactor = Vector3(0.0f);
        pMeshObj->UpdateMaterialBuffers(&vAlbedoFactor, fMetallicFactor, fRoughnessFactor, &vEmissiveFactor);

        // Animation.
        UAnimation* pDancerAnim0 = pAnimator->GetAnimation(GAME_ANIMATION_TYPE::GAME_ANIM_TYPE_DANCER);

        Matrix mWorldRow = Matrix();
        // Ref 0
        CreateContextTable(&mWorldRow, pDancerAnim0);

        // Ref 1
        CreateContextTable(&mWorldRow, pDancerAnim0);

        // Ref 2
        CreateContextTable(&mWorldRow, pDancerAnim0);

        // Ref 3
        CreateContextTable(&mWorldRow, pDancerAnim0);

        // Delete mesh data.
        pAssetManager->DeleteMeshData(ASSET_MESH_DATA_TYPE::ASSET_MESH_DATA_TYPE_DANCER);
    }
    else
    {
        __debugbreak();
        return AK_FALSE;
    }

    return AK_TRUE;
}

void UDancerModel::RenderShadow()
{
    IMeshObject* pMeshObj = GetRenderObject();
    const Matrix* pWorldMat = GetWorldMatrix();
    const Matrix* pBoneTransform = GetCurrentModelContext()->pAnimation->GetFinalTransforms();

    _pRenderer->RenderShadowOfSkinnedMeshObject(pMeshObj, pWorldMat, pBoneTransform);
}

void UDancerModel::Render()
{
    IMeshObject* pMeshObj = GetRenderObject();
    const Matrix* pWorldMax = GetWorldMatrix();
    const Matrix* pBoneTransform = GetCurrentModelContext()->pAnimation->GetFinalTransforms();
    
    _pRenderer->RenderSkinnedMeshObject(pMeshObj, pWorldMax, pBoneTransform);
}

void UDancerModel::RenderNormal()
{
    IMeshObject* pMeshObj = GetRenderObject();
    const Matrix* pWorldMat = GetWorldMatrix();
    const Matrix* pBoneTransform = GetCurrentModelContext()->pAnimation->GetFinalTransforms();

    _pRenderer->RenderNormalOfSkinnedMeshObject(pMeshObj, pWorldMat, pBoneTransform);
}

void UDancerModel::Release()
{
    AkU32 uRefCount = --_uRefCount;
    if (!uRefCount)
    {
        delete this;
    }
}

void UDancerModel::CleanUp()
{
    ModelContext_t* pCurModelCtx = GetCurrentModelContext();
    UAnimation* pAnimator = pCurModelCtx->pAnimation;

    pAnimator->DestroyAnimationClip(L"Dancing.anim");
}
