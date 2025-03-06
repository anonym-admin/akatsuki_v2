#include "pch.h"
#include "AssetManager.h"
#include "Application.h"
#include "SceneManager.h"
#include "SceneLoading.h"
#include "ModelImporter.h"
#include "GeometryGenerator.h"

/*
================
Asset Manager
================
*/

UAssetManager::UAssetManager()
{
}

UAssetManager::~UAssetManager()
{
	CleanUp();
}

AkBool UAssetManager::Initialize(UApplication* pApp)
{
	_pApp = pApp;
	_pRenderer = pApp->GetRenderer();

	return AK_TRUE;
}

void UAssetManager::AddMeshData(ASSET_MESH_DATA_TYPE eType, const wchar_t* wcBasePath, const wchar_t* wcModelFilename, AkF32 fScaleLength, AkBool bForAnim)
{
	USceneManager* pSceneManager = _pApp->GetSceneManager();
	USceneLoading* pSceneLoading = (USceneLoading*)pSceneManager->GetCurrentScene();
	AssetMeshDataContainer_t* pAssetMeshDataContainer = nullptr;

	pAssetMeshDataContainer = AllocMeshDataContainer();

	pAssetMeshDataContainer->pMeshData = ReadFromFile(pAssetMeshDataContainer, &pAssetMeshDataContainer->uMeshDataNum, wcBasePath, wcModelFilename, fScaleLength, bForAnim);

	_ppAssetMeshDataContainerList[(AkU32)eType] = pAssetMeshDataContainer;

	wchar_t wcFullPath[MAX_PATH] = {};
	wcscpy_s(wcFullPath, wcBasePath);
	wcscat_s(wcFullPath, wcModelFilename);
	wcscat_s(wcFullPath, L"\n");

	pSceneLoading->RenderLoadingScreenCallBack(wcFullPath);
}

void UAssetManager::AddCubeMapTexture(const wchar_t* wcBasePath, const wchar_t* wcEnvFilename, const wchar_t* wcIrradianceFilename, const wchar_t* wcSpecularFilename, const wchar_t* wcBrdfFilaename)
{
	USceneManager* pSceneManager = _pApp->GetSceneManager();
	USceneLoading* pSceneLoading = (USceneLoading*)pSceneManager->GetCurrentScene();
	AssetTextureContainer_t* pAssetTexContainer = nullptr;
	void* pTexHandle = nullptr;

	wchar_t wcFullPath[MAX_PATH] = {};
	wcscpy_s(wcFullPath, wcBasePath);
	wcscat_s(wcFullPath, wcEnvFilename);

	// Env
	pTexHandle = _pRenderer->CreateCubeMapTexture(wcFullPath);
	pAssetTexContainer = AllocTextureContainer();
	pAssetTexContainer->pTexHandle = pTexHandle;

	_ppAssetTextureContainerList[(AkU32)ASSET_TEXTURE_TYPE::ASSET_TEXTURE_TYPE_ENV] = pAssetTexContainer;

	wcscat_s(wcFullPath, L"\n");
	pSceneLoading->RenderLoadingScreenCallBack(wcFullPath);

	memset(wcFullPath, 0, sizeof(wchar_t) * MAX_PATH);
	wcscpy_s(wcFullPath, wcBasePath);
	wcscat_s(wcFullPath, wcIrradianceFilename);

	// Irradiance
	pTexHandle = _pRenderer->CreateCubeMapTexture(wcFullPath);
	pAssetTexContainer = AllocTextureContainer();
	pAssetTexContainer->pTexHandle = pTexHandle;

	_ppAssetTextureContainerList[(AkU32)ASSET_TEXTURE_TYPE::ASSET_TEXTURE_TYPE_IBL_IRRADIANCE] = pAssetTexContainer;

	wcscat_s(wcFullPath, L"\n");
	pSceneLoading->RenderLoadingScreenCallBack(wcFullPath);

	memset(wcFullPath, 0, sizeof(wchar_t) * MAX_PATH);
	wcscpy_s(wcFullPath, wcBasePath);
	wcscat_s(wcFullPath, wcSpecularFilename);

	// Specular
	pTexHandle = _pRenderer->CreateCubeMapTexture(wcFullPath);
	pAssetTexContainer = AllocTextureContainer();
	pAssetTexContainer->pTexHandle = pTexHandle;

	_ppAssetTextureContainerList[(AkU32)ASSET_TEXTURE_TYPE::ASSET_TEXTURE_TYPE_IBL_SPECULAR] = pAssetTexContainer;

	wcscat_s(wcFullPath, L"\n");
	pSceneLoading->RenderLoadingScreenCallBack(wcFullPath);

	memset(wcFullPath, 0, sizeof(wchar_t) * MAX_PATH);
	wcscpy_s(wcFullPath, wcBasePath);
	wcscat_s(wcFullPath, wcBrdfFilaename);

	// Brdf
	pTexHandle = _pRenderer->CreateTextureFromFile(wcFullPath, AK_FALSE);
	pAssetTexContainer = AllocTextureContainer();
	pAssetTexContainer->pTexHandle = pTexHandle;

	_ppAssetTextureContainerList[(AkU32)ASSET_TEXTURE_TYPE::ASSET_TEXTURE_TYPE_IBL_BRDF] = pAssetTexContainer;

	wcscat_s(wcFullPath, L"\n");
	pSceneLoading->RenderLoadingScreenCallBack(wcFullPath);
}

void UAssetManager::AddDynamicTexture()
{
}

void UAssetManager::DeleteMeshData(ASSET_MESH_DATA_TYPE eType)
{
	AkU32 uType = (AkU32)eType;

	if (_ppAssetMeshDataContainerList[uType])
	{
		if (_ppAssetMeshDataContainerList[uType]->pMeshData)
		{
			for (AkU32 i = 0; i < _ppAssetMeshDataContainerList[uType]->uMeshDataNum; i++)
			{
				if (_ppAssetMeshDataContainerList[uType]->pMeshData[i].pVertices)
				{
					free(_ppAssetMeshDataContainerList[uType]->pMeshData[i].pVertices);
					_ppAssetMeshDataContainerList[uType]->pMeshData[i].pVertices = nullptr;
				}
				if (_ppAssetMeshDataContainerList[uType]->pMeshData[i].pSkinnedVertices)
				{
					free(_ppAssetMeshDataContainerList[uType]->pMeshData[i].pSkinnedVertices);
					_ppAssetMeshDataContainerList[uType]->pMeshData[i].pSkinnedVertices = nullptr;
				}
				if (_ppAssetMeshDataContainerList[uType]->pMeshData[i].pIndices)
				{
					free(_ppAssetMeshDataContainerList[uType]->pMeshData[i].pIndices);
					_ppAssetMeshDataContainerList[uType]->pMeshData[i].pIndices = nullptr;
				}
			}

			free(_ppAssetMeshDataContainerList[uType]->pMeshData);
			_ppAssetMeshDataContainerList[uType]->pMeshData = nullptr;
		}

		FreeMeshDataContainer(_ppAssetMeshDataContainerList[uType]);
		_ppAssetMeshDataContainerList[uType] = nullptr;
	}
}

void UAssetManager::DeleteTexture(ASSET_TEXTURE_TYPE eType)
{
	AkU32 uType = (AkU32)eType;

	if (_ppAssetTextureContainerList[uType])
	{
		_pRenderer->DestroyTexture(_ppAssetTextureContainerList[uType]->pTexHandle);
		_ppAssetTextureContainerList[uType]->pTexHandle = nullptr;

		FreeTextureContainer(_ppAssetTextureContainerList[uType]);
		_ppAssetTextureContainerList[uType] = nullptr;
	}
}

MeshData_t* UAssetManager::ReadFromFile(AssetMeshDataContainer_t* pAassetMeshDataContainer, AkU32* pMeshDataNum, const wchar_t* wcBasePath, const wchar_t* wcModelFilename, AkF32 fScaleLength, AkBool bForAnim)
{
	UModelImporter tModelImporter = {};
	MeshData_t* pMeshData = nullptr;

	tModelImporter.Load(_pApp, wcBasePath, wcModelFilename, bForAnim);

	pMeshData = tModelImporter.GetMeshData();
	*pMeshDataNum = tModelImporter.GetMeshDataNum();

	UGeometryGenerator::NormalizeMeshData(pMeshData, *pMeshDataNum, fScaleLength, bForAnim, &pAassetMeshDataContainer->mDefaultMat);

	if (bForAnim)
	{
		pAassetMeshDataContainer->pBoneOffsetMatList = tModelImporter.GetBoneOffsetTransformList();
		pAassetMeshDataContainer->pBoneHierarchyList = tModelImporter.GetBoneHierarchyList();
		pAassetMeshDataContainer->uBoneNum = tModelImporter.GetBoneNum();
	}

	return pMeshData;
}

void UAssetManager::CleanUp()
{
	for (AkU32 i = 0; i < (AkU32)ASSET_MESH_DATA_TYPE::ASSET_MESH_DATA_TYPE_COUNT; i++)
	{
		DeleteMeshData((ASSET_MESH_DATA_TYPE)i);
	}
	for (AkU32 i = 0; i < (AkU32)ASSET_TEXTURE_TYPE::ASSET_TEXTURE_TYPE_COUNT; i++)
	{
		DeleteTexture((ASSET_TEXTURE_TYPE)i);
	}
}

AssetTextureContainer_t* UAssetManager::AllocTextureContainer()
{
	AssetTextureContainer_t* pAssetTexContainer = new AssetTextureContainer_t;
	memset(pAssetTexContainer, 0, sizeof(AssetTextureContainer_t));
	return pAssetTexContainer;
}

AssetMeshDataContainer_t* UAssetManager::AllocMeshDataContainer()
{
	AssetMeshDataContainer_t* pAssetMeshDataContainer = new AssetMeshDataContainer_t;
	memset(pAssetMeshDataContainer, 0, sizeof(AssetMeshDataContainer_t));
	return pAssetMeshDataContainer;
}

void UAssetManager::FreeTextureContainer(AssetTextureContainer_t* pAssetTextureContainer)
{
	if (pAssetTextureContainer)
	{
		delete pAssetTextureContainer;
		pAssetTextureContainer = nullptr;
	}
}

void UAssetManager::FreeMeshDataContainer(AssetMeshDataContainer_t* pAssetMeshDataContainer)
{
	if (pAssetMeshDataContainer)
	{
		delete pAssetMeshDataContainer;
		pAssetMeshDataContainer = nullptr;
	}
}
