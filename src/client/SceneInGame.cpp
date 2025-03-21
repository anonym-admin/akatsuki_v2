#include "pch.h"
#include "SceneInGame.h"
#include "Application.h"
#include "Camera.h"
#include "LandScape.h"
#include "KDTree.h"
#include "WorldMap.h"
#include "Swat.h"
#include "Dancer.h"
#include "BRS_74.h"
#include "ModelObject.h"
#include "TreeBillboards.h"
#include "Terrain.h"

/*
=============
Scene InGame
=============
*/

SceneInGame::~SceneInGame()
{
	EndScene();
}

AkBool SceneInGame::BeginScene()
{
	// Set IBL Strength
	GRenderer->SetIBLStrength(0.25f);

	// Terrain
	{
		Terrain* pTerrain = new Terrain(L"main.map");
		pTerrain->Name = L"Terrain";
		pTerrain->tLink.pData = pTerrain;
		AddGameObject(GAME_OBJECT_GROUP_TYPE::TERRAIN, pTerrain);
	}

	// Player.A
	{
		Swat* pSwat = new Swat;
		pSwat->Name = L"Swat";
		pSwat->tLink.pData = pSwat;
		pSwat->GetTransform()->SetRotation(DirectX::XM_PI, 0.0f, 0.0f);
		pSwat->GetTransform()->SetPosition(-18.0f, 0.5f, 5.0f);
		AddGameObject(GAME_OBJECT_GROUP_TYPE::PLAYER, pSwat);
	}

	// Dancer.
	{
		Dancer* pDancer = new Dancer;
		pDancer->Name = L"Dancer_01";
		pDancer->tLink.pData = pDancer;
		AddGameObject(GAME_OBJECT_GROUP_TYPE::DANCER, pDancer);
	}

	// Weapon
	{
		BRS_74* pBRS_74 = new BRS_74;
		pBRS_74->Name = L"BRS_74";
		pBRS_74->tLink.pData = pBRS_74;
		pBRS_74->GetTransform()->SetPosition(-20.0f, 0.5f, 5.0f);
		pBRS_74->GetTransform()->SetScale(0.56f, 0.56f, 0.56f);
		AddGameObject(GAME_OBJECT_GROUP_TYPE::WEAPON, pBRS_74);
	}

	// Container
	{
		//ModelObject* pContainer = new ModelObject;
		//pContainer->Name = L"Container";
		//pContainer->tLink.pData = pContainer;
		//pContainer->GetTransform()->SetPosition(-25.0f, 0.5f, 5.0f);
		//AddGameObject(GAME_OBJECT_GROUP_TYPE::CONTAINER, pContainer);
	}

	// Tree Billboard.
	{
		TreeBillboard* pTreeBillboards = new TreeBillboard;
		pTreeBillboards->Name = L"Tree";
		pTreeBillboards->tLink.pData = pTreeBillboards;
		AddGameObject(GAME_OBJECT_GROUP_TYPE::TREE, pTreeBillboards);
	}

	// Create mini map sprite.
	{
		AkI32 iWidth = 128;
		AkI32 iHeight = 128;

		RECT tRect = {};
		GetClientRect(GhWnd, &tRect);
		AkU32 uScreenWidth = tRect.right - tRect.left;
		AkU32 uScreenHeight = tRect.bottom - tRect.top;

		_iMiniMapPosX = uScreenWidth - iWidth - _iOffset;
		_iMiniMapPosY = _iOffset;

		_pMiniMapOutlineSprite = GRenderer->CreateSpriteObjectWidthTex(L"../../assets/colors/white.dds", 0, 0, 136, 136);
		_pMiniMapOutlineSprite->SetDrawBackground(AK_TRUE);

		_pMiniMapSprite = GRenderer->CreateSpriteObjectWidthTex(L"../../assets/landscape/colormap.dds", 0, 0, iWidth, iHeight);
		_pMiniMapSprite->SetDrawBackground(AK_TRUE);
	}

	// Create location point.
	{
		_pLocationPointSprite = GRenderer->CreateSpriteObjectWidthTex(L"../../assets/colors/light_green.dds", 0, 0, 3, 3);
		_pLocationPointSprite->SetDrawBackground(AK_TRUE);
	}

	// Create skybox.
	{
		_pSkyboxObj = GRenderer->CreateSkyboxObject();
	}

	// Bind IBL Texture For PBR.
	{
		AssetTextureContainer_t* pDiffuseHDR = GAssetManager->GetTextureContainer(ASSET_TEXTURE_TYPE::IRRADIANCE);
		AssetTextureContainer_t* pSpecularHDR = GAssetManager->GetTextureContainer(ASSET_TEXTURE_TYPE::SPECULAR);
		AssetTextureContainer_t* pBrdf = GAssetManager->GetTextureContainer(ASSET_TEXTURE_TYPE::BRDF);

		GRenderer->BindIBLTexture(pDiffuseHDR->pTexHandle, pSpecularHDR->pTexHandle, pBrdf->pTexHandle);
	}

	// Lights
	{
		//// Direction light.
		//Vector3 vPos = Vector3(0.0f, 10.0f, 0.0f);
		//Vector3 vDir = Vector3(0.0f, -1.0f, 0.0f);
		//AkF32 fSpotPower = 128.0f;
		//AkF32 fRadius = 0.0f;

		//_pRenderer->AddLight(&vPos, &vDir, fRadius, 0.0f, 1000.0f, LIGHT_SPOT, fSpotPower);

		//_pRenderer->AddLight(&vPos, &vDir, 0.0f, 0.0f, 0.0f, LIGHT_OFF, 0.0f);

		//_pRenderer->AddLight(&vPos, &vDir, 0.0f, 0.0f, 0.0f, LIGHT_OFF, 0.0f);
	}

	// Collision check.
	GCollisionManager->CollisionGroupCheck(GAME_OBJECT_GROUP_TYPE::PLAYER, GAME_OBJECT_GROUP_TYPE::CONTAINER);
	GCollisionManager->CollisionGroupCheck(GAME_OBJECT_GROUP_TYPE::PLAYER, GAME_OBJECT_GROUP_TYPE::WEAPON);
	GCollisionManager->CollisionGroupCheck(GAME_OBJECT_GROUP_TYPE::PLAYER, GAME_OBJECT_GROUP_TYPE::TERRAIN);

	return AK_TRUE;
}

AkBool SceneInGame::EndScene()
{
	GCollisionManager->Reset();

	if (_pSkyboxObj)
	{
		_pSkyboxObj->Release();
		_pSkyboxObj = nullptr;
	}
	if (_pLocationPointSprite)
	{
		_pLocationPointSprite->Release();
		_pLocationPointSprite = nullptr;
	}
	if (_pMiniMapSprite)
	{
		_pMiniMapSprite->Release();
		_pMiniMapSprite = nullptr;
	}
	if (_pMiniMapOutlineSprite)
	{
		_pMiniMapOutlineSprite->Release();
		_pMiniMapOutlineSprite = nullptr;
	}

	DeleteAllGameObject();

	return AK_TRUE;
}

void SceneInGame::Update()
{
	Scene::Update();

	RECT tRect = {};
	GetClientRect(GhWnd, &tRect);
	AkU32 uScreenWidth = tRect.right - tRect.left;
	AkU32 uScreenHeight = tRect.bottom - tRect.top;

	_iMiniMapPosX = uScreenWidth - 128 - _iOffset;
	_iMiniMapPosY = _iOffset;

	_iPosX = (_iMiniMapPosX + 4) + (AkI32)(0.5f * 128.0f);
	_iPosY = (_iMiniMapPosY + 4) + (AkI32)(0.5f * 128.0f);

	_iPosX -= 1;
	_iPosY -= 1;
}

void SceneInGame::FinalUpdate()
{
	Scene::FinalUpdate();
}

void SceneInGame::RenderShadow()
{
	Scene::RenderShadow();
}

void SceneInGame::Render()
{
	Scene::Render();

	// Render mini map.
	// _pRenderer->RenderSprite(_pMiniMapOutlineSprite, _iMiniMapPosX - 4, _iMiniMapPosY - 4, 1.0f, 1.0f, 0.0001f);
	// _pRenderer->RenderSprite(_pMiniMapSprite, _iMiniMapPosX, _iMiniMapPosY, 1.0f, 1.0f, 0.00009f);

	// Render location point.
	// _pRenderer->RenderSprite(_pLocationPointSprite, _iPosX, _iPosY, 1.0f, 1.0f, 0.0f);

	// Render Land scape.
	// _pLandScape->Render();

	AssetTextureContainer_t* pEnv = GAssetManager->GetTextureContainer(ASSET_TEXTURE_TYPE::ENV);
	AssetTextureContainer_t* pDiffuseHDR = GAssetManager->GetTextureContainer(ASSET_TEXTURE_TYPE::IRRADIANCE);
	AssetTextureContainer_t* pSpecularHDR = GAssetManager->GetTextureContainer(ASSET_TEXTURE_TYPE::SPECULAR);

	// Render skybox.
	GRenderer->RenderSkybox(_pSkyboxObj, &_mSkyboxTransform, pEnv->pTexHandle, pDiffuseHDR->pTexHandle, pSpecularHDR->pTexHandle);

	// Render KDTree.
	//if (_bRenderKDTreeFlag)
		//RenderKDTreeNode(_pKDTree, _pRenderer, _pKDTreeBoxObj);

	//for (AkU32 i = 0; i < BUILDING_BOX_COUNT; i++)
		//_pRenderer->RenderBasicMeshObject(_pBuildingMeshObj, &_mRandomTransform[i]);
}
