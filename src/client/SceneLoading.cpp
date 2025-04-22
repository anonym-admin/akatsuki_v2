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
		GAssetManager->AddMeshData(MESH_FILE_PATH, L"soldier.mesh", 1.0f, AK_TRUE);
		GAssetManager->AddMeshData(MESH_FILE_PATH, L"container.mesh", 1.0f, AK_FALSE);
		GAssetManager->AddMeshData(MESH_FILE_PATH, L"brs-74.mesh", 1.0f, AK_FALSE);
		GAssetManager->AddMeshData(MESH_FILE_PATH, L"stone_grey_01.mesh", 1.0f, AK_FALSE);
		GAssetManager->AddMeshData(MESH_FILE_PATH, L"casing.mesh", 1.0f, AK_FALSE);
	}

	// Load Animation.
	{
		GAssetManager->ReadClip(L"soldier", L"soldier_idle.anim");
		GAssetManager->ReadClip(L"soldier", L"soldier_rifle_idle.anim");
		GAssetManager->ReadClip(L"soldier", L"soldier_walk.anim");
		GAssetManager->ReadClip(L"soldier", L"soldier_walk_back.anim");
		GAssetManager->ReadClip(L"soldier", L"soldier_walk_right.anim");
		GAssetManager->ReadClip(L"soldier", L"soldier_walk_right_diag.anim");
		GAssetManager->ReadClip(L"soldier", L"soldier_walk_right_bdiag.anim");
		GAssetManager->ReadClip(L"soldier", L"soldier_walk_left.anim");
		GAssetManager->ReadClip(L"soldier", L"soldier_walk_left_diag.anim");
		GAssetManager->ReadClip(L"soldier", L"soldier_walk_left_bdiag.anim");
		GAssetManager->ReadClip(L"soldier", L"soldier_rifle_walk.anim");
		GAssetManager->ReadClip(L"soldier", L"soldier_rifle_walk_back.anim");
		GAssetManager->ReadClip(L"soldier", L"soldier_rifle_walk_right.anim");
		GAssetManager->ReadClip(L"soldier", L"soldier_rifle_walk_right_diag.anim");
		GAssetManager->ReadClip(L"soldier", L"soldier_rifle_walk_right_bdiag.anim");
		GAssetManager->ReadClip(L"soldier", L"soldier_rifle_walk_left.anim");
		GAssetManager->ReadClip(L"soldier", L"soldier_rifle_walk_left_diag.anim");
		GAssetManager->ReadClip(L"soldier", L"soldier_rifle_walk_left_bdiag.anim");
		GAssetManager->ReadClip(L"soldier", L"soldier_rifle_run.anim");
		GAssetManager->ReadClip(L"soldier", L"soldier_rifle_run_back.anim");
		GAssetManager->ReadClip(L"soldier", L"soldier_rifle_run_right.anim");
		GAssetManager->ReadClip(L"soldier", L"soldier_rifle_run_right_diag.anim");
		GAssetManager->ReadClip(L"soldier", L"soldier_rifle_run_right_bdiag.anim");
		GAssetManager->ReadClip(L"soldier", L"soldier_rifle_run_left.anim");
		GAssetManager->ReadClip(L"soldier", L"soldier_rifle_run_left_diag.anim");
		GAssetManager->ReadClip(L"soldier", L"soldier_rifle_run_left_bdiag.anim");
		GAssetManager->ReadClip(L"soldier", L"soldier_rifle_fire_stop.anim");
		GAssetManager->ReadClip(L"soldier", L"soldier_punch.anim");
		GAssetManager->ReadClip(L"soldier", L"soldier_walk_jump.anim");
		GAssetManager->ReadClip(L"soldier", L"soldier_rifle_jump.anim");
	}

	// Image based lighting textures.
	{
		GAssetManager->AddCubeMapTexture(IBL_FILE_PATH, L"PureSky8KEnvHDR.dds", L"PureSky8KDiffuseHDR.dds", L"PureSky8KSpecularHDR.dds", L"PureSky8KBrdf.dds");
		// GAssetManager->AddCubeMapTexture(IBL_FILE_PATH, L"PureSkyEnvHDR.dds", L"PureSkyDiffuseHDR.dds", L"PureSkySpecularHDR.dds", L"PureSkyBrdf.dds");
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
	static std::wstring wcTextChunk = L"";

	// Update status text
	AkI32 iTextWidth = 0;
	AkI32 iTextHeight = 0;

	wcTextChunk += std::wstring(wcText);
	AkU32 uTxtLen = (AkU32)wcslen(wcTextChunk.c_str());

	// 텍스트가 변경된 경우
	GRenderer->WriteTextToBitmap(_pScreenImage, _uScreenWidth, _uScreenHeight, _uScreenWidth * 4, &iTextWidth, &iTextHeight, GFont, wcTextChunk.c_str(), uTxtLen);
	GRenderer->UpdateTextureWidthImage(_pScreenTextureHandle, _pScreenImage, _uScreenWidth, _uScreenHeight);

	GRenderer->BeginRender();

	GRenderer->RenderSpriteWithTex(GSprite, 0, 0, 1.0f, 1.0f, nullptr, 0.0f, _pScreenTextureHandle, AK_FALSE);

	GRenderer->EndRender();
	GRenderer->Present();
}
