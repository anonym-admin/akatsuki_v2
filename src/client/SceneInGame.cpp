#include "pch.h"
#include "SceneInGame.h"
#include "Application.h"
#include "Actor.h"
#include "GeometryGenerator.h"
#include "CollisionManager.h"
#include "Camera.h"
#include "LandScape.h"
#include "AssetManager.h"
#include "ModelManager.h"
#include "Player.h"
#include "Dancer.h"
#include "BRS_74.h"
#include "Building.h"
#include "PlayerModel.h"
#include "DancerModel.h"
#include "BRS_74_Model.h"
#include "Collider.h"
#include "KDTree.h"
#include "GameInput.h"
#include "WorldMap.h"

/*
=============
Scene InGame
=============
*/

USceneInGame::USceneInGame()
{
}

USceneInGame::~USceneInGame()
{
	EndScene();
}

AkBool USceneInGame::Initialize(UApplication* pApp)
{
	if (!UScene::Initialize(pApp))
	{
		__debugbreak();
		return AK_FALSE;
	}

	_pApp = pApp;

	_pRenderer = _pApp->GetRenderer();

	return AK_TRUE;
}

AkBool USceneInGame::BeginScene()
{
	UCollisionManager* pCollisionManager = _pApp->GetCollisionManager();
	UAssetManager* pAssetManager = _pApp->GetAssetManager();
	UModelManager* pModelManager = _pApp->GetModelManager();

	// Init Default Models.
	pModelManager->InitDefaultModels();
	
	// Set IBL Strength
	_pRenderer->SetIBLStrength(0.25f);

	// Player.A
	{
		UActor* pPlayer = new UPlayer;
		pPlayer->Initialize(_pApp);
		pPlayer->SetRotationY(DirectX::XM_PI);
		pPlayer->SetPosition(0.0f, 10.0f, 1025.0f);
		pPlayer->SetName(L"Player");

		pPlayer->tLink.pData = pPlayer;
		AddGameObject(GAME_OBJECT_GROUP_TYPE::GAME_OBJ_GROUP_TYPE_PLAYER, pPlayer);
	}

	// Dancer.
	{
		{
			UActor* pDancer = new UDancer;
			pDancer->Initialize(_pApp);
			pDancer->SetPosition(-2.0f, 0.5f, 1025.0f);
			pDancer->SetName(L"Dancer");
			pDancer->tLink.pData = pDancer;
			AddGameObject(GAME_OBJECT_GROUP_TYPE::GAME_OBJ_GROUP_TYPE_DANCER, pDancer);
		}
		{
			UActor* pDancer = new UDancer;
			pDancer->Initialize(_pApp);
			pDancer->SetPosition(2.0f, 0.5f, 1025.0f);
			pDancer->SetName(L"Dancer");
			pDancer->tLink.pData = pDancer;
			AddGameObject(GAME_OBJECT_GROUP_TYPE::GAME_OBJ_GROUP_TYPE_DANCER, pDancer);
		}
	}

	// World Map Containter.
	{
		_pWorldMap = new UWorldMapContainer;
		if (!_pWorldMap->Initialize(_pApp))
		{
			__debugbreak();
			return AK_FALSE;
		}

		_pWorldMap->SetName(L"Map");
		_pWorldMap->tLink.pData = _pWorldMap;
		AddGameObject(GAME_OBJECT_GROUP_TYPE::GAME_OBJ_GROUP_TYPE_MAP, _pWorldMap);
	}

	// Buildings.
	{
		AkU32 uNum = 0;
		MeshData_t* pBox = nullptr;
		_pBoxList = new AkBox_t[BUILDING_BOX_COUNT]; 
		_pDraw = new AkBool[BUILDING_BOX_COUNT + 1]; // 임시.
		memset(_pBoxList, 0, sizeof(MeshData_t*) * BUILDING_BOX_COUNT);
		memset(_pDraw, 1, BUILDING_BOX_COUNT);

		_pWorldMap->_pDrawTable = _pDraw;

		for (AkU32 i = 0; i < BUILDING_BOX_COUNT; i++)
		{
			AkF32 fScaleX = GenterateRandomFloat(1.0f, 30.0f);
			AkF32 fScaleY = GenterateRandomFloat(1.0f, 30.0f);
			AkF32 fScaleZ = GenterateRandomFloat(1.0f, 30.0f);

			AkF32 fRandX = GenterateRandomFloat(-1000.0f, 1000.0f);
			AkF32 fRandY = GenterateRandomFloat(-1000.0f, 1000.0f);
			AkF32 fRandZ = GenterateRandomFloat(-1000.0f, 1000.0f);

			// Start Box.
			if (0 == i)
			{
				fScaleX = 25.0f;
				fScaleY = 2.0f;
				fScaleZ = 25.0f;

				fRandX = 0.0f;
				fRandY = 0.0f;
				fRandZ = -1025.0f;
			}

			if (BUILDING_BOX_COUNT - 1 == i)
			{
				fScaleX = 25.0f;
				fScaleY = 2.0f;
				fScaleZ = 25.0f;

				fRandX = 0.0f;
				fRandY = 0.0f;
				fRandZ = 1025.0f;
			}

			Vector3 vRandPosition = Vector3(fRandX, fRandY, fRandZ);

			_pRandomTransform[i] = Matrix::CreateScale(fScaleX, fScaleY, fScaleZ) * Matrix::CreateTranslation(vRandPosition);

			pBox = UGeometryGenerator::MakeCube(&uNum, 0.5f);
			for (AkU32 j = 0; j < uNum; j++)
			{
				for (AkU32 k = 0; k < pBox[j].uVerticeNum; k++)
				{
					pBox[j].pVertices[k].vPosition = Vector3::Transform(pBox[j].pVertices[k].vPosition, _pRandomTransform[i]);
				}
			}

			// Bind Map Container.
			_pWorldMap->BindMeshData(pBox, uNum);

			// For Frustum Culling.
			Vector3 vMin = Vector3(AK_MAX_F32);
			Vector3 vMax = Vector3(-AK_MAX_F32);
			for (AkU32 j = 0; j < uNum; j++)
			{
				for (AkU32 k = 0; k < pBox[j].uVerticeNum; k++)
				{
					vMin.x = min(vMin.x, pBox[j].pVertices[k].vPosition.x);
					vMin.y = min(vMin.y, pBox[j].pVertices[k].vPosition.y);
					vMin.z = min(vMin.z, pBox[j].pVertices[k].vPosition.z);
					vMax.x = max(vMax.x, pBox[j].pVertices[k].vPosition.x);
					vMax.y = max(vMax.y, pBox[j].pVertices[k].vPosition.y);
					vMax.z = max(vMax.z, pBox[j].pVertices[k].vPosition.z);
				}
			}

			AkBox_t tBox = {};
			tBox.vMax = vMax;
			tBox.vMin = vMin;
			_pBoxList[i] = tBox;
		}

		pBox = UGeometryGenerator::MakeCube(&uNum, 0.5f);
		for (AkU32 j = 0; j < uNum; j++)
		{
			wcscpy_s(pBox[j].wcAlbedoTextureFilename, L"../../assets/asset_test_01.dds");
		}

		Vector3 vAlbedo = Vector3(1.0f);
		AkF32 fMetallic = 0.0f;
		AkF32 fRoughness = 1.0f;
		Vector3 vEmissive = Vector3(0.0f);
		_pBuildingMeshObj = _pRenderer->CreateBasicMeshObject();
		_pBuildingMeshObj->CreateMeshBuffers(pBox, uNum);
		_pBuildingMeshObj->UpdateMaterialBuffers(&vAlbedo, fMetallic, fRoughness, &vEmissive);
		UGeometryGenerator::DestroyGeometry(pBox, uNum);

		// Bind Map Container.
		_pWorldMap->BindMeshObj(_pBuildingMeshObj, BUILDING_BOX_COUNT, _pRandomTransform);
	}

	// House
	{
		AssetMeshDataContainer_t tAssetMeshDataContainter = {};
		tAssetMeshDataContainter.pMeshData = pAssetManager->ReadFromFile(&tAssetMeshDataContainter, &tAssetMeshDataContainter.uMeshDataNum, L"../../assets/model/", L"cottage_fbx.md3d", 7.5f, AK_FALSE);
		wcscpy_s(tAssetMeshDataContainter.pMeshData->wcAlbedoTextureFilename, L"../../assets/model/cottage_diffuse.dds");
		wcscpy_s(tAssetMeshDataContainter.pMeshData->wcNormalTextureFilename, L"../../assets/model/cottage_normal.dds");

		// Material.
		Vector3 vAlbedo = Vector3(1.0f);
		AkF32 fMetallic = 0.0f;
		AkF32 fRoughness = 1.0f;
		Vector3 vEmissive = Vector3(0.0f);
		Matrix mWorld = Matrix();
		Vector3 vPos = Vector3(0.0f, 2.5f, -1030.0f);

		for (AkU32 i = 0; i < tAssetMeshDataContainter.uMeshDataNum; i++)
		{
			for (AkU32 j = 0; j < tAssetMeshDataContainter.pMeshData[i].uVerticeNum; j++)
			{
				tAssetMeshDataContainter.pMeshData[i].pVertices[j].vPosition = Vector3::Transform(tAssetMeshDataContainter.pMeshData[i].pVertices[j].vPosition, Matrix::CreateTranslation(vPos));

				// For Debugging.
				// printf("House Normal: %lf %lf %lf \n", tAssetMeshDataContainter.pMeshData[i].pVertices[j].vNormalModel.x, tAssetMeshDataContainter.pMeshData[i].pVertices[j].vNormalModel.y, tAssetMeshDataContainter.pMeshData[i].pVertices[j].vNormalModel.z);
			}
		}

		// Bind Map Container.
		_pWorldMap->BindMeshData(tAssetMeshDataContainter.pMeshData, tAssetMeshDataContainter.uMeshDataNum);

		// Create Render Object.
		_pHouseMeshObj = _pRenderer->CreateBasicMeshObject();
		_pHouseMeshObj->CreateMeshBuffers(tAssetMeshDataContainter.pMeshData, tAssetMeshDataContainter.uMeshDataNum);
		_pHouseMeshObj->UpdateMaterialBuffers(&vAlbedo, fMetallic, fRoughness, &vEmissive);

		// Bind Map Container.
		_pWorldMap->BindMeshObj(_pHouseMeshObj, 1, &_mHouseWorld);
	}

	// Weapon
	{
		UBRS_74* pBRS_74 = new UBRS_74;
		Vector3 vExtent = Vector3(0.05f, 0.1f, 0.35f);
		Vector3 vCenter = Vector3(0.0f);
		pBRS_74->Initialize(_pApp, &vExtent, &vCenter);
		pBRS_74->SetName(L"BRS_74");
		pBRS_74->SetPosition(0.0f, 1.0f, 1025.0f);

		pBRS_74->tLink.pData = pBRS_74;
		AddGameObject(GAME_OBJECT_GROUP_TYPE::GAME_OBJ_GROUP_TYPE_WEAPON, pBRS_74);
	}

	// LandScape
	{
		_pLandScape = new ULandScape;
		if (!_pLandScape->Initialize(_pApp, L"../../assets/landscape/setup.txt"))
		{
			__debugbreak();
			return AK_FALSE;
		}
	}

	// Attach Map.
	{
		_pWorldMap->Build(AK_TRUE);
		pCollisionManager->AttachMap(_pWorldMap);
	}

	// Create mini map sprite.
	{
		AkI32 iWidth = 128;
		AkI32 iHeight = 128;

		_iMiniMapPosX = _pApp->GetScreenWidth() - iWidth - _iOffset;
		_iMiniMapPosY = _iOffset;

		_pMiniMapOutlineSprite = _pRenderer->CreateSpriteObjectWidthTex(L"../../assets/colors/white.dds", 0, 0, 136, 136);
		_pMiniMapOutlineSprite->SetDrawBackground(AK_TRUE);

		_pMiniMapSprite = _pRenderer->CreateSpriteObjectWidthTex(L"../../assets/landscape/colormap.dds", 0, 0, iWidth, iHeight);
		_pMiniMapSprite->SetDrawBackground(AK_TRUE);
	}

	// Create location point.
	{
		_pLocationPointSprite = _pRenderer->CreateSpriteObjectWidthTex(L"../../assets/colors/light_green.dds", 0, 0, 3, 3);
		_pLocationPointSprite->SetDrawBackground(AK_TRUE);
	}

	// Create skybox.
	{
		_pSkyboxObj = _pRenderer->CreateSkyboxObject();
	}

	// Bind IBL Texture For PBR.
	{
		AssetTextureContainer_t* pDiffuseHDR = pAssetManager->GetTextureContainer(ASSET_TEXTURE_TYPE::ASSET_TEXTURE_TYPE_IBL_IRRADIANCE);
		AssetTextureContainer_t* pSpecularHDR = pAssetManager->GetTextureContainer(ASSET_TEXTURE_TYPE::ASSET_TEXTURE_TYPE_IBL_SPECULAR);
		AssetTextureContainer_t* pBrdf = pAssetManager->GetTextureContainer(ASSET_TEXTURE_TYPE::ASSET_TEXTURE_TYPE_IBL_BRDF);

		_pRenderer->BindIBLTexture(pDiffuseHDR->pTexHandle, pSpecularHDR->pTexHandle, pBrdf->pTexHandle);
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
	pCollisionManager->CollisionGroupCheck(GAME_OBJECT_GROUP_TYPE::GAME_OBJ_GROUP_TYPE_PLAYER, GAME_OBJECT_GROUP_TYPE::GAME_OBJ_GROUP_TYPE_DANCER);
	pCollisionManager->CollisionGroupCheck(GAME_OBJECT_GROUP_TYPE::GAME_OBJ_GROUP_TYPE_PLAYER, GAME_OBJECT_GROUP_TYPE::GAME_OBJ_GROUP_TYPE_WEAPON);
	pCollisionManager->CollisionGroupCheck(GAME_OBJECT_GROUP_TYPE::GAME_OBJ_GROUP_TYPE_PLAYER, GAME_OBJECT_GROUP_TYPE::GAME_OBJ_GROUP_TYPE_MAP);
	pCollisionManager->CollisionGroupCheck(GAME_OBJECT_GROUP_TYPE::GAME_OBJ_GROUP_TYPE_DANCER, GAME_OBJECT_GROUP_TYPE::GAME_OBJ_GROUP_TYPE_MAP);
	pCollisionManager->CollisionGroupCheck(GAME_OBJECT_GROUP_TYPE::GAME_OBJ_GROUP_TYPE_WEAPON, GAME_OBJECT_GROUP_TYPE::GAME_OBJ_GROUP_TYPE_MAP);

	// Create Frustum.
	_pFrustum = CreateFrustum(_pRenderer);

	return AK_TRUE;
}

AkBool USceneInGame::EndScene()
{
	UModelManager* pModelManager = _pApp->GetModelManager();

	pModelManager->CleanUpDefaultModels();

	if (_pHouseMeshObj)
	{
		_pHouseMeshObj->Release();
		_pHouseMeshObj = nullptr;
	}
	if (_pDraw)
	{
		delete[] _pDraw;
		_pDraw = nullptr;
	}
	if (_pBoxList)
	{
		delete[] _pBoxList;
		_pBoxList = nullptr;
	}
	if (_pFrustum)
	{
		delete _pFrustum;
		_pFrustum = nullptr;
	}
	if (_pLandScape)
	{
		delete _pLandScape;
		_pLandScape = nullptr;
	}

	if (_pBuildingMeshObj)
	{
		_pBuildingMeshObj->Release();
		_pBuildingMeshObj = nullptr;
	}

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

	return AK_TRUE;
}

void USceneInGame::Update(const AkF32 fDeltaTime)
{
	GameObjContainer_t** pGameObjContainerList = GetAllGameObject();
	UGameInput* pGameInput = _pApp->GetGameInput();

	// Update game obj.
	for (AkU32 i = 0; i < (AkU32)GAME_OBJECT_GROUP_TYPE::GAME_OBJ_GROUP_TYPE_COUNT; i++)
	{
		if (pGameObjContainerList[i])
		{
			List_t* pCur = pGameObjContainerList[i]->pGameObjHead;
			while (pCur != nullptr)
			{
				UActor* pActor = reinterpret_cast<UActor*>(pCur->pData);
				pActor->Update(fDeltaTime);

				pCur = pCur->pNext;
			}
		}
	}

	_iMiniMapPosX = _pApp->GetScreenWidth() - 128 - _iOffset;
	_iMiniMapPosY = _iOffset;

	_iPosX = (_iMiniMapPosX + 4) + (AkI32)(0.5f * 128.0f);
	_iPosY = (_iMiniMapPosY + 4) + (AkI32)(0.5f * 128.0f);

	_iPosX -= 1;
	_iPosY -= 1;

	// Update LandScape.
	// _pLandScape->Update(fDeltaTime);

	// Update Frustum Culling.
	// Frustum Culling 구현부와 KDTree Triangle 충돌사이에 연동 데이터를 적용하여, KDTree 충돌처리시에 충돌 Obj 를 미리 컬링하는 구조로 최적화 가능.
	UpdateFrustum(_pRenderer, _pFrustum);

	AkU32 uIndex = 0;

	// 충돌 첫프레임 싱크 맞추기.
	if (_bFirst)
	{
		memset(_pDraw, 1, BUILDING_BOX_COUNT + 1);
		_bFirst = AK_FALSE;
	}
	else
	{
		memset(_pDraw, 0, BUILDING_BOX_COUNT + 1);
		for (AkU32 i = 0; i < BUILDING_BOX_COUNT; i++)
		{
			AkBool bIntersectFrustumToBox = IntersectFrustumToBox(_pFrustum, &_pBoxList[i]);
			if (bIntersectFrustumToBox)
			{
				_pDraw[i] = AK_TRUE;
				_pTempRandomTransform[uIndex] = _pRandomTransform[i];
				uIndex++;
			}
		}

		if (uIndex >= BUILDING_BOX_COUNT)
		{
			__debugbreak();
		}

		_pWorldMap->SetMeshObjNum(0, uIndex);
		_pWorldMap->SetWorldRowList(0, _pTempRandomTransform);
		_uRenderMapObjCount = uIndex;
	}

	// Occulusion Culling.
	// TODO!!
}

void USceneInGame::FinalUpdate(const AkF32 fDeltaTime)
{
	GameObjContainer_t** pGameObjContainerList = GetAllGameObject();

	// Final Update game obj.
	for (AkU32 i = 0; i < (AkU32)GAME_OBJECT_GROUP_TYPE::GAME_OBJ_GROUP_TYPE_COUNT; i++)
	{
		if (pGameObjContainerList[i])
		{
			List_t* pCur = pGameObjContainerList[i]->pGameObjHead;
			while (pCur != nullptr)
			{
				UActor* pActor = reinterpret_cast<UActor*>(pCur->pData);
				pActor->FinalUpdate(fDeltaTime);

				pCur = pCur->pNext;
			}
		}
	}
}

void USceneInGame::RenderShadowPass()
{
	GameObjContainer_t** pGameObjContainerList = GetAllGameObject();

	// Render game obj.
	for (AkU32 i = 0; i < (AkU32)GAME_OBJECT_GROUP_TYPE::GAME_OBJ_GROUP_TYPE_COUNT; i++)
	{
		if (pGameObjContainerList[i])
		{
			List_t* pCur = pGameObjContainerList[i]->pGameObjHead;
			while (pCur != nullptr)
			{
				UActor* pActor = reinterpret_cast<UActor*>(pCur->pData);
				pActor->RenderShadow();

				pCur = pCur->pNext;
			}
		}
	}
}

void USceneInGame::Render()
{
	GameObjContainer_t** pGameObjContainerList = GetAllGameObject();

	// Render game obj.
	for (AkU32 i = 0; i < (AkU32)GAME_OBJECT_GROUP_TYPE::GAME_OBJ_GROUP_TYPE_COUNT; i++)
	{
		if (pGameObjContainerList[i])
		{
			List_t* pCur = pGameObjContainerList[i]->pGameObjHead;
			while (pCur != nullptr)
			{
				UActor* pActor = reinterpret_cast<UActor*>(pCur->pData);
				pActor->Render();

				pCur = pCur->pNext;
			}
		}
	}

	// Render mini map.
	// _pRenderer->RenderSprite(_pMiniMapOutlineSprite, _iMiniMapPosX - 4, _iMiniMapPosY - 4, 1.0f, 1.0f, 0.0001f);
	// _pRenderer->RenderSprite(_pMiniMapSprite, _iMiniMapPosX, _iMiniMapPosY, 1.0f, 1.0f, 0.00009f);

	// Render location point.
	// _pRenderer->RenderSprite(_pLocationPointSprite, _iPosX, _iPosY, 1.0f, 1.0f, 0.0f);

	// Render Land scape.
	// _pLandScape->Render();

	UAssetManager* pAssetManager = _pApp->GetAssetManager();

	AssetTextureContainer_t* pEnv = pAssetManager->GetTextureContainer(ASSET_TEXTURE_TYPE::ASSET_TEXTURE_TYPE_ENV);
	AssetTextureContainer_t* pDiffuseHDR = pAssetManager->GetTextureContainer(ASSET_TEXTURE_TYPE::ASSET_TEXTURE_TYPE_IBL_IRRADIANCE);
	AssetTextureContainer_t* pSpecularHDR = pAssetManager->GetTextureContainer(ASSET_TEXTURE_TYPE::ASSET_TEXTURE_TYPE_IBL_SPECULAR);

	// Render skybox.
	_pRenderer->RenderSkybox(_pSkyboxObj, &_mSkyboxTransform, pEnv->pTexHandle, pDiffuseHDR->pTexHandle, pSpecularHDR->pTexHandle);

	// Render KDTree.
	//if (_bRenderKDTreeFlag)
		//RenderKDTreeNode(_pKDTree, _pRenderer, _pKDTreeBoxObj);

	//for (AkU32 i = 0; i < BUILDING_BOX_COUNT; i++)
		//_pRenderer->RenderBasicMeshObject(_pBuildingMeshObj, &_mRandomTransform[i]);
}

AkU32 USceneInGame::GetTriangleCount()
{
	if (nullptr == _pWorldMap)
	{
		return 0;
	}

	return _pWorldMap->GetColliderNum(); // Collider 가 삼각형.
}
