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
#include "Container.h"

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

	// Player.A
	{
		Swat* pSwat = new Swat();
		pSwat->Name = L"Swat";
		pSwat->tLink.pData = pSwat;
		pSwat->GetTransform()->SetRotation(DirectX::XM_PI, 0.0f, 0.0f);
		pSwat->GetTransform()->SetPosition(0.0f, 1.5f, 1025.0f);
		AddGameObject(GAME_OBJECT_GROUP_TYPE::GAME_OBJ_GROUP_TYPE_PLAYER, pSwat);
	}

	// Dancer.
	{
		Dancer* pDancer = new Dancer;
		pDancer->Name = L"Dancer_01";
		pDancer->tLink.pData = pDancer;
		AddGameObject(GAME_OBJECT_GROUP_TYPE::GAME_OBJ_GROUP_TYPE_DANCER, pDancer);
	}

	// Weapon
	{
		UBRS_74* pBRS_74 = new UBRS_74;
		pBRS_74->Name = L"BRS_74";
		pBRS_74->tLink.pData = pBRS_74;
		pBRS_74->GetTransform()->SetPosition(0.0f, 1.5f, -1025.0f);
		AddGameObject(GAME_OBJECT_GROUP_TYPE::GAME_OBJ_GROUP_TYPE_WEAPON, pBRS_74);
	}

	// Container
	{
		Container* pContainer = new Container;
		pContainer->Name = L"Container";
		pContainer->tLink.pData = pContainer;
		pContainer->GetTransform()->SetPosition(0.0f, 1.5f, 1025.0f);
		AddGameObject(GAME_OBJECT_GROUP_TYPE::GAME_OBJ_GROUP_TYPE_CONTAINER, pContainer);
	}

	// World Map Containter.
	{
		_pWorldMap = new MapObjects;
		_pWorldMap->Name = L"Map Obj";
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

			pBox = GeometryGenerator::MakeCube(&uNum, 0.5f);
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

		pBox = GeometryGenerator::MakeCube(&uNum, 0.5f);
		for (AkU32 j = 0; j < uNum; j++)
		{
			wcscpy_s(pBox[j].wcAlbedoTextureFilename, L"../../assets/asset_test_01.dds");
		}

		Vector3 vAlbedo = Vector3(1.0f);
		AkF32 fMetallic = 0.0f;
		AkF32 fRoughness = 1.0f;
		Vector3 vEmissive = Vector3(0.0f);
		_pBuildingMeshObj = GRenderer->CreateBasicMeshObject();
		_pBuildingMeshObj->CreateMeshBuffers(pBox, uNum);
		_pBuildingMeshObj->UpdateMaterialBuffers(&vAlbedo, fMetallic, fRoughness, &vEmissive);
		GeometryGenerator::DestroyGeometry(pBox, uNum);

		// Bind Map Container.
		_pWorldMap->BindMeshObj(_pBuildingMeshObj, BUILDING_BOX_COUNT, _pRandomTransform);
	}

	// House
	{
		AssetMeshDataContainer_t tAssetMeshDataContainter = {};
		tAssetMeshDataContainter.pMeshData = GAssetManager->ReadFromFile(&tAssetMeshDataContainter, &tAssetMeshDataContainter.uMeshDataNum, L"../../assets/model/", L"cottage_fbx.md3d", 7.5f, AK_FALSE);
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
		_pHouseMeshObj = GRenderer->CreateBasicMeshObject();
		_pHouseMeshObj->CreateMeshBuffers(tAssetMeshDataContainter.pMeshData, tAssetMeshDataContainter.uMeshDataNum);
		_pHouseMeshObj->UpdateMaterialBuffers(&vAlbedo, fMetallic, fRoughness, &vEmissive);

		// Bind Map Container.
		_pWorldMap->BindMeshObj(_pHouseMeshObj, 1, &_mHouseWorld);
	}

	// Attach Map.
	{
		_pWorldMap->Build(AK_TRUE);
		GCollisionManager->AttachMap(_pWorldMap);
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
	GCollisionManager->CollisionGroupCheck(GAME_OBJECT_GROUP_TYPE::GAME_OBJ_GROUP_TYPE_PLAYER, GAME_OBJECT_GROUP_TYPE::GAME_OBJ_GROUP_TYPE_CONTAINER);

	// Create Frustum.
	_pFrustum = CreateFrustum(GRenderer);

	return AK_TRUE;
}

AkBool SceneInGame::EndScene()
{
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

	// Update Frustum Culling.
	// Frustum Culling 구현부와 KDTree Triangle 충돌사이에 연동 데이터를 적용하여, KDTree 충돌처리시에 충돌 Obj 를 미리 컬링하는 구조로 최적화 가능.
	UpdateFrustum(GRenderer, _pFrustum);

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

AkU32 SceneInGame::GetTriangleCount()
{
	if (nullptr == _pWorldMap)
	{
		return 0;
	}

	return _pWorldMap->GetColliderNum(); // Collider 가 삼각형.
}
