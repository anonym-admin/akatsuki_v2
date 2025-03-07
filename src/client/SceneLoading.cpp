#include "pch.h"
#include "SceneLoading.h"
#include "Application.h"
#include "GeometryGenerator.h"
#include "SceneManager.h"
#include "Animator.h"
#include "LandScape.h"
#include "AssetManager.h"
#include "EventManager.h"

/*
===============
Loading Scene
===============
*/

SceneLoading::~SceneLoading()
{
	EndScene();
}

AkBool SceneLoading::BeginScene()
{
	// Create Font.
	CreateCommonFontObject(L"Consolas", 12.0f);

	// Create sprite obj.
	CreateCommonSpriteObject();

	// Create texture for loading screen text.
	RECT tRect = {};
	::GetClientRect(GhWnd, &tRect);
	_uScreenWidth = tRect.right - tRect.left;
	_uScreenHeight = tRect.bottom - tRect.top;
	_pScreenImage = (AkU8*)malloc(_uScreenWidth * _uScreenHeight * 4);
	_pScreenTextureHandle = GRenderer->CreateDynamicTexture(_uScreenWidth, _uScreenHeight);
	memset(_pScreenImage, 0, _uScreenWidth * _uScreenHeight * 4);

	// Load MeshData.
	{
		GAssetManager->AddMeshData(ASSET_MESH_DATA_TYPE::ASSET_MESH_DATA_TYPE_SWATGUY, L"../../assets/model/", L"SwatGuy.md3d", 1.0f, AK_TRUE);
		GAssetManager->AddMeshData(ASSET_MESH_DATA_TYPE::ASSET_MESH_DATA_TYPE_DANCER, L"../../assets/model/", L"Dancer.md3d", 1.0f, AK_TRUE);
		GAssetManager->AddMeshData(ASSET_MESH_DATA_TYPE::ASSET_MESH_DATA_TYPE_BRS_74, L"../../assets/model/", L"BRS-74.md3d", 1.0f, AK_FALSE);
	}

	// Load Animation clip.
	{
		AssetMeshDataContainer_t* pMeshDataContanier = GAssetManager->GetMeshDataContainer(ASSET_MESH_DATA_TYPE::ASSET_MESH_DATA_TYPE_SWATGUY);
		AnimatorHandle_t tAnimatorHandle = {};
		tAnimatorHandle.mDefaultMat = pMeshDataContanier->mDefaultMat;
		tAnimatorHandle.pBoneOffsetMatList = pMeshDataContanier->pBoneOffsetMatList;
		tAnimatorHandle.pBoneHierarchyList = pMeshDataContanier->pBoneHierarchyList;
		tAnimatorHandle.uBoneNum = pMeshDataContanier->uBoneNum;
		GAnimator->AddAnimation(GAME_ANIMATION_TYPE::GAME_ANIM_TYPE_PLAYER, GMAE_ANIM_FILE_BASE_PATH, GAME_ANIM_PLAYER_ANIM_FILE_NAME, _countof(GAME_ANIM_PLAYER_ANIM_FILE_NAME), &tAnimatorHandle);
	
		pMeshDataContanier = GAssetManager->GetMeshDataContainer(ASSET_MESH_DATA_TYPE::ASSET_MESH_DATA_TYPE_DANCER);
		tAnimatorHandle.mDefaultMat = pMeshDataContanier->mDefaultMat;
		tAnimatorHandle.pBoneOffsetMatList = pMeshDataContanier->pBoneOffsetMatList;
		tAnimatorHandle.pBoneHierarchyList = pMeshDataContanier->pBoneHierarchyList;
		tAnimatorHandle.uBoneNum = pMeshDataContanier->uBoneNum;
		GAnimator->AddAnimation(GAME_ANIMATION_TYPE::GAME_ANIM_TYPE_DANCER, GMAE_ANIM_FILE_BASE_PATH, GAME_ANIM_DANCER_ANIM_FILE_NAME, _countof(GAME_ANIM_DANCER_ANIM_FILE_NAME), &tAnimatorHandle);
	}

	// Image based lighting textures.
	{
		GAssetManager->AddCubeMapTexture(L"../../assets/skybox/", L"PureSkyEnvHDR.dds", L"PureSkyDiffuseHDR.dds", L"PureSkySpecularHDR.dds", L"PureSkyBrdf.dds");
	}

	// Add Change Scene Event.
	EventHandle_t tEventHandle = {};
	tEventHandle.eEventType = EVENT_TYPE::SCENE_CHANGE;
	tEventHandle.tSceneChangeParam.eBefore = SCENE_TYPE::SCENE_TYPE_LOADING;
	tEventHandle.tSceneChangeParam.eAfter = SCENE_TYPE::SCENE_TYPE_INGANE;
	GEventManager->AddEvent(&tEventHandle);

	return AK_TRUE;
}

AkBool SceneLoading::EndScene()
{
	// Loading 관련 오브젝트 삭제.
	if (_pScreenImage)
	{
		free(_pScreenImage);
		_pScreenImage = nullptr;
	}
	if (_pScreenTextureHandle)
	{
		GRenderer->DestroyTexture(_pScreenTextureHandle);
		_pScreenTextureHandle = nullptr;
	}

	return AK_TRUE;
}

void SceneLoading::RenderLoadingScreenCallBack(const wchar_t* wcText)
{
	static wchar_t wcChunkText[1024] = {};

	// Update status text
	AkI32 iTextWidth = 0;
	AkI32 iTextHeight = 0;

	wcscat_s(wcChunkText, wcText);
	AkU32 uTxtLen = (AkU32)wcslen(wcChunkText);

	// 텍스트가 변경된 경우
	GRenderer->WriteTextToBitmap(_pScreenImage, _uScreenWidth, _uScreenHeight, _uScreenWidth * 4, &iTextWidth, &iTextHeight, GetCommonFontObject(), wcChunkText, uTxtLen);
	GRenderer->UpdateTextureWidthImage(_pScreenTextureHandle, _pScreenImage, _uScreenWidth, _uScreenHeight);

	GRenderer->BeginRender();

	GRenderer->RenderSpriteWithTex(GetCommonSpriteObject(), 0, 0, 1.0f, 1.0f, nullptr, 0.0f, _pScreenTextureHandle);

	GRenderer->EndRender();
	GRenderer->Present();
}
