#include "pch.h"
#include "SceneInGame.h"
#include "Application.h"
#include "Camera.h"
#include "LandScape.h"
#include "KDTree.h"
#include "WorldMap.h"
#include "Soldier.h"
#include "Dancer.h"
#include "BRS_74.h"
#include "ModelObject.h"
#include "TreeBillboards.h"
#include "Terrain.h"
#include "Ocean.h"
#include "Cloud.h"

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
	Collider::DRAW_COLLIDER = AK_FALSE;

	// Set IBL Strength
	GRenderer->SetIBLStrength(0.25f);

	// Load Scene file.
	Load(L"../../assets/data/scene/main.scene");

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
		AssetTextureContainer_t* pDiffuseHDR = GAssetManager->GetTexture(L"PureSkyDiffuseHDR.dds");
		AssetTextureContainer_t* pSpecularHDR = GAssetManager->GetTexture(L"PureSkySpecularHDR.dds");
		AssetTextureContainer_t* pBrdf = GAssetManager->GetTexture(L"PureSkyBrdf.dds");

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

	//// Collision check.
	//GCollisionManager->CollisionGroupCheck(GAME_OBJECT_GROUP_TYPE::PLAYER, GAME_OBJECT_GROUP_TYPE::CONTAINER);
	//GCollisionManager->CollisionGroupCheck(GAME_OBJECT_GROUP_TYPE::PLAYER, GAME_OBJECT_GROUP_TYPE::WEAPON);
	//GCollisionManager->CollisionGroupCheck(GAME_OBJECT_GROUP_TYPE::PLAYER, GAME_OBJECT_GROUP_TYPE::TERRAIN);

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

void SceneInGame::RenderDepthMap()
{
	Scene::RenderDepthMap();
}

void SceneInGame::RenderShadowMaps()
{
	Scene::RenderShadowMaps();
}

void SceneInGame::Render()
{
	Scene::Render();

	AssetTextureContainer_t* pEnv = GAssetManager->GetTexture(L"PureSkyEnvHDR.dds");
	AssetTextureContainer_t* pDiffuseHDR = GAssetManager->GetTexture(L"PureSkyDiffuseHDR.dds");
	AssetTextureContainer_t* pSpecularHDR = GAssetManager->GetTexture(L"PureSkySpecularHDR.dds");

	// Render skybox.
	GRenderer->RenderSkybox(_pSkyboxObj, &_mSkyboxTransform, pEnv->pTexHandle, pDiffuseHDR->pTexHandle, pSpecularHDR->pTexHandle);
	GRenderer->RenderReflectionOfSkybox(_pSkyboxObj, &_mSkyboxTransform, pEnv->pTexHandle, pDiffuseHDR->pTexHandle, pSpecularHDR->pTexHandle);
}

void SceneInGame::Load(const wchar_t* wcSceneFile)
{
	DeleteAllGameObject();

	FILE* fp = nullptr;
	_wfopen_s(&fp, wcSceneFile, L"rt");
	if (!fp)
	{
		__debugbreak();
	}

	// 01. terrain.
	Terrain* pTerrain = nullptr;
	wchar_t wcName[_MAX_PATH] = {};
	fwscanf_s(fp, L"%s", wcName, (unsigned)_MAX_PATH);
	pTerrain = new Terrain(wcName);
	pTerrain->tLink.pData = pTerrain;
	AddGameObject(GAME_OBJECT_GROUP_TYPE::TERRAIN, pTerrain);

	// 02. obj
	Actor* pActor = nullptr;
	AkI32 iNumGameObj = 0;
	fwscanf_s(fp, L"%d", &iNumGameObj);
	for (AkI32 i = 0; i < iNumGameObj; i++)
	{
		fwscanf_s(fp, L"%s", wcName, (unsigned)_MAX_PATH);

		AkBool bInst = AK_FALSE;
		GameObjContainer_t** ppGameObjectContainer = GetAllGameObject();
		for (AkU32 i = 0; i < (AkU32)GAME_OBJECT_GROUP_TYPE::COUNT; i++)
		{
			if (ppGameObjectContainer[i])
			{
				List_t* pCur = ppGameObjectContainer[i]->pGameObjHead;
				while (pCur != nullptr)
				{
					if (!wcscmp(wcName, ((Actor*)pCur->pData)->Name))
					{
						pActor = ((ModelObject*)pCur->pData)->Clone();
						bInst = AK_TRUE;
					}

					pCur = pCur->pNext;
				}
			}
		}

		std::wstring wcTempName = wcName;

		if (GetFileNmaeExcludeExt(GetFileName(wcTempName)) == L"soldier")
		{
			pActor = new Soldier(wcTempName.c_str());
		}
		else if (wcTempName.find(L"Ocean") != std::wstring::npos)
		{
			pActor = new Ocean;
		}
		else if (wcTempName.find(L"Cloud") != std::wstring::npos)
		{
			pActor = new Cloud;
		}
		else
		{
			if (!bInst)
			{
				pActor = new ModelObject(wcTempName.c_str());
			}
		}

		Vector3 vScale = Vector3(1.0f);
		Vector3 vYawPitchRoll = Vector3(0.0f);
		Vector3 vPos = Vector3(0.0f);

		fwscanf_s(fp, L"%f %f %f", &vScale.x, &vScale.y, &vScale.z);
		fwscanf_s(fp, L"%f %f %f", &vYawPitchRoll.x, &vYawPitchRoll.y, &vYawPitchRoll.z);
		fwscanf_s(fp, L"%f %f %f", &vPos.x, &vPos.y, &vPos.z);

		pActor->GetTransform()->SetScale(&vScale);
		pActor->GetTransform()->SetRotation(&vYawPitchRoll);
		pActor->GetTransform()->SetPosition(&vPos);

		pActor->GetTransform()->Update();

		pActor->tLink.pData = pActor;
		AddGameObject(GAME_OBJECT_GROUP_TYPE::PLAYER, pActor);
	}

	// 03. billboard
	AkI32 iNumBillboard = 0;
	fwscanf_s(fp, L"%d", &iNumBillboard);
	for (AkI32 i = 0; i < iNumBillboard; i++)
	{
		fwscanf_s(fp, L"%s", wcName, (unsigned)_MAX_PATH);

		AkI32 iVertexNum = 0;
		fwscanf_s(fp, L"%d", &iVertexNum);

		VertexSize_t* pVertices = new VertexSize_t[iVertexNum];
		for (AkI32 i = 0; i < iVertexNum; i++)
		{
			fwscanf_s(fp, L"%f %f %f", &pVertices[i].vPosition.x, &pVertices[i].vPosition.y, &pVertices[i].vPosition.z);
			fwscanf_s(fp, L"%f %f", &pVertices[i].vSize.x, &pVertices[i].vSize.y);
		}

		Billboard* pBillboard = new Billboard(wcName, pVertices, iVertexNum);
		pBillboard->tLink.pData = pBillboard;
		AddGameObject(GAME_OBJECT_GROUP_TYPE::BILLBOARD, pBillboard);

		if(pVertices)
		{
			delete[] pVertices;
			pVertices = nullptr;
		}
	}

	if (fp)
	{
		fclose(fp);
		fp = nullptr;
	}
}
