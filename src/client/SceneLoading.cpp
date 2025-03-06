#include "pch.h"
#include "SceneLoading.h"
#include "Application.h"
#include "GeometryGenerator.h"
#include "SceneManager.h"
#include "Animation.h"
#include "LandScape.h"
#include "AssetManager.h"
#include "EventManager.h"

/*
===============
Loading Scene
===============
*/

USceneLoading::USceneLoading()
{
}

USceneLoading::~USceneLoading()
{
}

AkBool USceneLoading::Initialize(UApplication* pApp)
{
	if (!UScene::Initialize(pApp))
	{
		__debugbreak();
		return AK_FALSE;
	}

	return AK_TRUE;
}

AkBool USceneLoading::BeginScene()
{
	_pApp = GetApp();

	_pRenderer = GetRenderer();

	HWND hWnd = _pApp->GetHwnd();

	USceneManager* pSceneManager = _pApp->GetSceneManager();
	UAssetManager* pAssetManager = _pApp->GetAssetManager();
	UGameEventManager* pEventManager = _pApp->GetGameEventManager();
	UAnimator* pAnimator = _pApp->GetAnimator();

	// Create Font.
	CreateCommonFontObject(L"Consolas", 12.0f);

	// Create sprite obj.
	CreateCommonSpriteObject();

	// Create texture for loading screen text.
	RECT tRect = {};
	::GetClientRect(hWnd, &tRect);
	_uScreenWidth = tRect.right - tRect.left;
	_uScreenHeight = tRect.bottom - tRect.top;
	_pScreenImage = (AkU8*)malloc(_uScreenWidth * _uScreenHeight * 4);
	_pScreenTextureHandle = _pRenderer->CreateDynamicTexture(_uScreenWidth, _uScreenHeight);
	memset(_pScreenImage, 0, _uScreenWidth * _uScreenHeight * 4);

	// Load MeshData.
	{
		pAssetManager->AddMeshData(ASSET_MESH_DATA_TYPE::ASSET_MESH_DATA_TYPE_SWATGUY, L"../../assets/model/", L"SwatGuy.md3d", 1.0f, AK_TRUE);
		pAssetManager->AddMeshData(ASSET_MESH_DATA_TYPE::ASSET_MESH_DATA_TYPE_DANCER, L"../../assets/model/", L"Dancer.md3d", 1.0f, AK_TRUE);
		pAssetManager->AddMeshData(ASSET_MESH_DATA_TYPE::ASSET_MESH_DATA_TYPE_BRS_74, L"../../assets/model/", L"BRS-74.md3d", 1.0f, AK_FALSE);
	}

	// Load Animation clip.
	{
		AssetMeshDataContainer_t* pMeshDataContanier = pAssetManager->GetMeshDataContainer(ASSET_MESH_DATA_TYPE::ASSET_MESH_DATA_TYPE_SWATGUY);
		AnimatorHandle_t tAnimatorHandle = {};
		tAnimatorHandle.mDefaultMat = pMeshDataContanier->mDefaultMat;
		tAnimatorHandle.pBoneOffsetMatList = pMeshDataContanier->pBoneOffsetMatList;
		tAnimatorHandle.pBoneHierarchyList = pMeshDataContanier->pBoneHierarchyList;
		tAnimatorHandle.uBoneNum = pMeshDataContanier->uBoneNum;
		pAnimator->AddAnimation(GAME_ANIMATION_TYPE::GAME_ANIM_TYPE_PLAYER, GMAE_ANIM_FILE_BASE_PATH, GAME_ANIM_PLAYER_ANIM_FILE_NAME, _countof(GAME_ANIM_PLAYER_ANIM_FILE_NAME), &tAnimatorHandle);
	
		pMeshDataContanier = pAssetManager->GetMeshDataContainer(ASSET_MESH_DATA_TYPE::ASSET_MESH_DATA_TYPE_DANCER);
		tAnimatorHandle.mDefaultMat = pMeshDataContanier->mDefaultMat;
		tAnimatorHandle.pBoneOffsetMatList = pMeshDataContanier->pBoneOffsetMatList;
		tAnimatorHandle.pBoneHierarchyList = pMeshDataContanier->pBoneHierarchyList;
		tAnimatorHandle.uBoneNum = pMeshDataContanier->uBoneNum;
		pAnimator->AddAnimation(GAME_ANIMATION_TYPE::GAME_ANIM_TYPE_DANCER, GMAE_ANIM_FILE_BASE_PATH, GAME_ANIM_DANCER_ANIM_FILE_NAME, _countof(GAME_ANIM_DANCER_ANIM_FILE_NAME), &tAnimatorHandle);
	}

	// Image based lighting textures.
	{
		pAssetManager->AddCubeMapTexture(L"../../assets/skybox/", L"PureSkyEnvHDR.dds", L"PureSkyDiffuseHDR.dds", L"PureSkySpecularHDR.dds", L"PureSkyBrdf.dds");
	}

	//// Create LandScape.
	//{
	//	// 해당 모델 데이터 2개의 매쉬 데이터 존재(1. 실질적 데이터, 2. 옆면) 
	//	_pLandScape = new ULandScape;
	//	_pLandScape->Initialize(_pApp, L"../../assets/landscape/setup.txt");
	//}

	// pSceneManager->ChangeScene(GAME_SCENE_TYPE::SCENE_TYPE_INGANE);

	GameEventHandle_t tEventHandle = {};
	tEventHandle.eEventType = GAME_EVENT_TYPE::GAME_EVENT_TYPE_SCENE_CHANGE;
	tEventHandle.tSceneChangeParam.eBefore = GAME_SCENE_TYPE::SCENE_TYPE_LOADING;
	tEventHandle.tSceneChangeParam.eAfter = GAME_SCENE_TYPE::SCENE_TYPE_INGANE;
	pEventManager->AddEvent(&tEventHandle);

	return AK_TRUE;
}

AkBool USceneLoading::EndScene()
{
	// Loading 관련 오브젝트 삭제.
	if (_pScreenImage)
	{
		free(_pScreenImage);
		_pScreenImage = nullptr;
	}
	if (_pScreenTextureHandle)
	{
		_pRenderer->DestroyTexture(_pScreenTextureHandle);
		_pScreenTextureHandle = nullptr;
	}

	return AK_TRUE;
}

void USceneLoading::RenderLoadingScreenCallBack(const wchar_t* wcText)
{
	static wchar_t wcChunkText[1024] = {};

	// Update status text
	AkI32 iTextWidth = 0;
	AkI32 iTextHeight = 0;

	wcscat_s(wcChunkText, wcText);
	AkU32 uTxtLen = (AkU32)wcslen(wcChunkText);

	// 텍스트가 변경된 경우
	_pRenderer->WriteTextToBitmap(_pScreenImage, _uScreenWidth, _uScreenHeight, _uScreenWidth * 4, &iTextWidth, &iTextHeight, GetCommonFontObject(), wcChunkText, uTxtLen);
	_pRenderer->UpdateTextureWidthImage(_pScreenTextureHandle, _pScreenImage, _uScreenWidth, _uScreenHeight);

	_pRenderer->BeginRender();

	_pRenderer->RenderSpriteWithTex(GetCommonSpriteObject(), 0, 0, 1.0f, 1.0f, nullptr, 0.0f, _pScreenTextureHandle);

	_pRenderer->EndRender();
	_pRenderer->Present();
}
