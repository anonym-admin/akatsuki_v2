#include "pch.h"
#include "PlayerModel.h"
#include "Animation.h"
#include "Application.h"
#include "GameInput.h"
#include "AssetManager.h"

UPlayerModel::UPlayerModel()
{
}

UPlayerModel::~UPlayerModel()
{
	CleanUp();
}

AkBool UPlayerModel::Initialize(UApplication* pApp)
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
		AssetMeshDataContainer_t* pMeshDataContainer = pAssetManager->GetMeshDataContainer(ASSET_MESH_DATA_TYPE::ASSET_MESH_DATA_TYPE_SWATGUY);
		MeshData_t* pPlayerMeshData = pMeshDataContainer->pMeshData;
		AkU32 uPlayerMeshDataNum = pMeshDataContainer->uMeshDataNum;
		CreateMeshBuffersForDraw(pPlayerMeshData, uPlayerMeshDataNum);

		// Material.
		IMeshObject* pMeshObj = GetRenderObject();
		Vector3 vAlbedoFactor = Vector3(1.0f);
		AkF32 fMetallicFactor = 0.0f;
		AkF32 fRoughnessFactor = 1.0f;
		Vector3 vEmissiveFactor = Vector3(0.0f);
		pMeshObj->UpdateMaterialBuffers(&vAlbedoFactor, fMetallicFactor, fRoughnessFactor, &vEmissiveFactor);

		// Animation.
		UAnimation* pPlayerAnim0 = pAnimator->GetAnimation(GAME_ANIMATION_TYPE::GAME_ANIM_TYPE_PLAYER);

		Matrix mWorldRow = Matrix();

		// Ref 0
		CreateContextTable(&mWorldRow, pPlayerAnim0);

		// Ref 1
		CreateContextTable(&mWorldRow, pPlayerAnim0);

		// Ref 2
		CreateContextTable(&mWorldRow, pPlayerAnim0);

		// Ref 3
		CreateContextTable(&mWorldRow, pPlayerAnim0);

		// Delete mesh data.
		pAssetManager->DeleteMeshData(ASSET_MESH_DATA_TYPE::ASSET_MESH_DATA_TYPE_SWATGUY);
	}
	else
	{
		__debugbreak();
		return AK_FALSE;
	}

	return AK_TRUE;
}

void UPlayerModel::Render()
{
	IMeshObject* pMeshObj = GetRenderObject();
	const Matrix* pWorldMat = GetWorldMatrix();
	const Matrix* pBoneTransform = GetCurrentModelContext()->pAnimation->GetFinalTransforms();

	_pRenderer->RenderSkinnedMeshObject(pMeshObj, pWorldMat, pBoneTransform);
}

void UPlayerModel::Release()
{
	AkU32 uRefCount = --_uRefCount;
	if (!uRefCount)
	{
		delete this;
	}
}

void UPlayerModel::RenderNormal()
{
	IMeshObject* pMeshObj = GetRenderObject();
	const Matrix* pWorldMat = GetWorldMatrix();
	const Matrix* pBoneTransform = GetCurrentModelContext()->pAnimation->GetFinalTransforms();

	_pRenderer->RenderNormalOfSkinnedMeshObject(pMeshObj, pWorldMat, pBoneTransform);
}

void UPlayerModel::RenderShadow()
{
	IMeshObject* pMeshObj = GetRenderObject();
	const Matrix* pWorldMat = GetWorldMatrix();
	const Matrix* pBoneTransform = GetCurrentModelContext()->pAnimation->GetFinalTransforms();

	_pRenderer->RenderShadowOfSkinnedMeshObject(pMeshObj, pWorldMat, pBoneTransform);
}

void UPlayerModel::CleanUp()
{
}
