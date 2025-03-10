#include "pch.h"
#include "SceneLoading.h"
#include "Application.h"
#include "GeometryGenerator.h"
#include "SceneManager.h"
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
		GAssetManager->AddMeshData(ASSET_MESH_DATA_TYPE::SWATGUY, MODEL_FILE_PATH, L"SwatGuy.md3d", 1.0f, AK_TRUE);
		GAssetManager->AddMeshData(ASSET_MESH_DATA_TYPE::DANCER, MODEL_FILE_PATH, L"Dancer.md3d", 1.0f, AK_TRUE);
		GAssetManager->AddMeshData(ASSET_MESH_DATA_TYPE::BRS_74, MODEL_FILE_PATH, L"BRS-74.md3d", 1.0f, AK_FALSE);

		GAssetManager->AddMeshData(ASSET_MESH_DATA_TYPE::BULLET, MODEL_FILE_PATH, L"Bullet.md3d", 1.0f, AK_FALSE);
		GAssetManager->AddMeshData(ASSET_MESH_DATA_TYPE::CASING, MODEL_FILE_PATH, L"Casing.md3d", 1.0f, AK_FALSE);

	}

	// Load Animation.
	{

		// Idle
		GAssetManager->ReadClip(ASSET_ANIM_TYPE::SWATGUY, ANIM_FILE_PATH, L"SwatGuy_Idle.anim");
		GAssetManager->ReadClip(ASSET_ANIM_TYPE::SWATGUY, ANIM_FILE_PATH, L"SwatGuy_RifleIdle.anim");
		// Walk
		GAssetManager->ReadClip(ASSET_ANIM_TYPE::SWATGUY, ANIM_FILE_PATH, L"SwatGuy_FrontWalk.anim");
		GAssetManager->ReadClip(ASSET_ANIM_TYPE::SWATGUY, ANIM_FILE_PATH, L"SwatGuy_FrontLeftWalk.anim");
		GAssetManager->ReadClip(ASSET_ANIM_TYPE::SWATGUY, ANIM_FILE_PATH, L"SwatGuy_FrontRightWalk.anim");
		GAssetManager->ReadClip(ASSET_ANIM_TYPE::SWATGUY, ANIM_FILE_PATH, L"SwatGuy_LeftWalk.anim");
		GAssetManager->ReadClip(ASSET_ANIM_TYPE::SWATGUY, ANIM_FILE_PATH, L"SwatGuy_RightWalk.anim");
		GAssetManager->ReadClip(ASSET_ANIM_TYPE::SWATGUY, ANIM_FILE_PATH, L"SwatGuy_BackLeftWalk.anim");
		GAssetManager->ReadClip(ASSET_ANIM_TYPE::SWATGUY, ANIM_FILE_PATH, L"SwatGuy_BackRightWalk.anim");
		GAssetManager->ReadClip(ASSET_ANIM_TYPE::SWATGUY, ANIM_FILE_PATH, L"SwatGuy_BackWalk.anim");
		GAssetManager->ReadClip(ASSET_ANIM_TYPE::SWATGUY, ANIM_FILE_PATH, L"SwatGuy_RifleWalking.anim");
		// Run
		GAssetManager->ReadClip(ASSET_ANIM_TYPE::SWATGUY, ANIM_FILE_PATH, L"SwatGuy_Run.anim");
		GAssetManager->ReadClip(ASSET_ANIM_TYPE::SWATGUY, ANIM_FILE_PATH, L"SwatGuy_FrontLeftRun.anim");
		GAssetManager->ReadClip(ASSET_ANIM_TYPE::SWATGUY, ANIM_FILE_PATH, L"SwatGuy_FrontRightRun.anim");
		GAssetManager->ReadClip(ASSET_ANIM_TYPE::SWATGUY, ANIM_FILE_PATH, L"SwatGuy_RifleRun.anim");
		// Attack
		GAssetManager->ReadClip(ASSET_ANIM_TYPE::SWATGUY, ANIM_FILE_PATH, L"SwatGuy_Punching_01.anim");
		GAssetManager->ReadClip(ASSET_ANIM_TYPE::SWATGUY, ANIM_FILE_PATH, L"SwatGuy_Punching_02.anim");
		GAssetManager->ReadClip(ASSET_ANIM_TYPE::SWATGUY, ANIM_FILE_PATH, L"SwatGuy_RifleIdleFire.anim");
		// Jump
		GAssetManager->ReadClip(ASSET_ANIM_TYPE::SWATGUY, ANIM_FILE_PATH, L"SwatGuy_RunJump.anim");
		GAssetManager->ReadClip(ASSET_ANIM_TYPE::SWATGUY, ANIM_FILE_PATH, L"SwatGuy_IdleJump.anim");
	}

	// Image based lighting textures.
	{
		GAssetManager->AddCubeMapTexture(IBL_FILE_PATH, L"PureSkyEnvHDR.dds", L"PureSkyDiffuseHDR.dds", L"PureSkySpecularHDR.dds", L"PureSkyBrdf.dds");
	}

	// Add Change Scene Event.
	EventHandle_t tEventHandle = {};
	tEventHandle.eEventType = EVENT_TYPE::SCENE_CHANGE;
	tEventHandle.tSceneAndEditorChangeParam.eBeforeScene = SCENE_TYPE::LOADING;
	tEventHandle.tSceneAndEditorChangeParam.eAfterScene = SCENE_TYPE::INGANE;
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
	//static std::wstring wcTextChunk = L"";

	//// Update status text
	//AkI32 iTextWidth = 0;
	//AkI32 iTextHeight = 0;

	//wcTextChunk += std::wstring(wcText);
	//AkU32 uTxtLen = (AkU32)wcslen(wcTextChunk.c_str());

	//// 텍스트가 변경된 경우
	//GRenderer->WriteTextToBitmap(_pScreenImage, _uScreenWidth, _uScreenHeight, _uScreenWidth * 4, &iTextWidth, &iTextHeight, GetCommonFontObject(), wcTextChunk.c_str(), uTxtLen);
	//GRenderer->UpdateTextureWidthImage(_pScreenTextureHandle, _pScreenImage, _uScreenWidth, _uScreenHeight);

	//GRenderer->BeginRender();

	//GRenderer->RenderSpriteWithTex(GetCommonSpriteObject(), 0, 0, 1.0f, 1.0f, nullptr, 0.0f, _pScreenTextureHandle);

	//GRenderer->EndRender();
	//GRenderer->Present();
}
