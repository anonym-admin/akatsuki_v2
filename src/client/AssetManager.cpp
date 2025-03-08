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

AssetManager::~AssetManager()
{
	CleanUp();
}

void AssetManager::AddMeshData(ASSET_MESH_DATA_TYPE eType, const wchar_t* wcBasePath, const wchar_t* wcModelFilename, AkF32 fScaleLength, AkBool bForAnim)
{
	SceneLoading* pSceneLoading = (SceneLoading*)GSceneManager->GetCurrentScene();
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

void AssetManager::AddCubeMapTexture(const wchar_t* wcBasePath, const wchar_t* wcEnvFilename, const wchar_t* wcIrradianceFilename, const wchar_t* wcSpecularFilename, const wchar_t* wcBrdfFilaename)
{
	SceneLoading* pSceneLoading = (SceneLoading*)GSceneManager->GetCurrentScene();
	AssetTextureContainer_t* pAssetTexContainer = nullptr;
	void* pTexHandle = nullptr;

	wchar_t wcFullPath[MAX_PATH] = {};
	wcscpy_s(wcFullPath, wcBasePath);
	wcscat_s(wcFullPath, wcEnvFilename);

	// Env
	pTexHandle = GRenderer->CreateCubeMapTexture(wcFullPath);
	pAssetTexContainer = AllocTextureContainer();
	pAssetTexContainer->pTexHandle = pTexHandle;

	_ppAssetTextureContainerList[(AkU32)ASSET_TEXTURE_TYPE::ASSET_TEXTURE_TYPE_ENV] = pAssetTexContainer;

	wcscat_s(wcFullPath, L"\n");
	pSceneLoading->RenderLoadingScreenCallBack(wcFullPath);

	memset(wcFullPath, 0, sizeof(wchar_t) * MAX_PATH);
	wcscpy_s(wcFullPath, wcBasePath);
	wcscat_s(wcFullPath, wcIrradianceFilename);

	// Irradiance
	pTexHandle = GRenderer->CreateCubeMapTexture(wcFullPath);
	pAssetTexContainer = AllocTextureContainer();
	pAssetTexContainer->pTexHandle = pTexHandle;

	_ppAssetTextureContainerList[(AkU32)ASSET_TEXTURE_TYPE::ASSET_TEXTURE_TYPE_IBL_IRRADIANCE] = pAssetTexContainer;

	wcscat_s(wcFullPath, L"\n");
	pSceneLoading->RenderLoadingScreenCallBack(wcFullPath);

	memset(wcFullPath, 0, sizeof(wchar_t) * MAX_PATH);
	wcscpy_s(wcFullPath, wcBasePath);
	wcscat_s(wcFullPath, wcSpecularFilename);

	// Specular
	pTexHandle = GRenderer->CreateCubeMapTexture(wcFullPath);
	pAssetTexContainer = AllocTextureContainer();
	pAssetTexContainer->pTexHandle = pTexHandle;

	_ppAssetTextureContainerList[(AkU32)ASSET_TEXTURE_TYPE::ASSET_TEXTURE_TYPE_IBL_SPECULAR] = pAssetTexContainer;

	wcscat_s(wcFullPath, L"\n");
	pSceneLoading->RenderLoadingScreenCallBack(wcFullPath);

	memset(wcFullPath, 0, sizeof(wchar_t) * MAX_PATH);
	wcscpy_s(wcFullPath, wcBasePath);
	wcscat_s(wcFullPath, wcBrdfFilaename);

	// Brdf
	pTexHandle = GRenderer->CreateTextureFromFile(wcFullPath, AK_FALSE);
	pAssetTexContainer = AllocTextureContainer();
	pAssetTexContainer->pTexHandle = pTexHandle;

	_ppAssetTextureContainerList[(AkU32)ASSET_TEXTURE_TYPE::ASSET_TEXTURE_TYPE_IBL_BRDF] = pAssetTexContainer;

	wcscat_s(wcFullPath, L"\n");
	pSceneLoading->RenderLoadingScreenCallBack(wcFullPath);
}

void AssetManager::AddDynamicTexture()
{
}

void AssetManager::DeleteMeshData(ASSET_MESH_DATA_TYPE eType)
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

		if (_ppAssetMeshDataContainerList[uType]->pBoneHierarchyList)
		{
			delete[] _ppAssetMeshDataContainerList[uType]->pBoneHierarchyList;
			_ppAssetMeshDataContainerList[uType]->pBoneHierarchyList = nullptr;
		}

		if (_ppAssetMeshDataContainerList[uType]->pBoneOffsetMatrixList)
		{
			delete[] _ppAssetMeshDataContainerList[uType]->pBoneOffsetMatrixList;
			_ppAssetMeshDataContainerList[uType]->pBoneOffsetMatrixList = nullptr;
		}

		FreeMeshDataContainer(_ppAssetMeshDataContainerList[uType]);
		_ppAssetMeshDataContainerList[uType] = nullptr;
	}
}

void AssetManager::DeleteTexture(ASSET_TEXTURE_TYPE eType)
{
	AkU32 uType = (AkU32)eType;

	if (_ppAssetTextureContainerList[uType])
	{
		GRenderer->DestroyTexture(_ppAssetTextureContainerList[uType]->pTexHandle);
		_ppAssetTextureContainerList[uType]->pTexHandle = nullptr;

		FreeTextureContainer(_ppAssetTextureContainerList[uType]);
		_ppAssetTextureContainerList[uType] = nullptr;
	}
}

MeshData_t* AssetManager::ReadFromFile(AssetMeshDataContainer_t* pAassetMeshDataContainer, AkU32* pMeshDataNum, const wchar_t* wcBasePath, const wchar_t* wcModelFilename, AkF32 fScaleLength, AkBool bForAnim)
{
	UModelImporter tModelImporter = {};
	MeshData_t* pMeshData = nullptr;

	tModelImporter.Load(wcBasePath, wcModelFilename, bForAnim);

	pMeshData = tModelImporter.GetMeshData();
	*pMeshDataNum = tModelImporter.GetMeshDataNum();

	GeometryGenerator::NormalizeMeshData(pMeshData, *pMeshDataNum, fScaleLength, bForAnim, &pAassetMeshDataContainer->mDefaultMat);

	if (bForAnim)
	{
		pAassetMeshDataContainer->pBoneOffsetMatrixList = tModelImporter.GetBoneOffsetTransformList();
		pAassetMeshDataContainer->pBoneHierarchyList = tModelImporter.GetBoneHierarchyList();
		pAassetMeshDataContainer->uBoneNum = tModelImporter.GetBoneNum();
	}

	return pMeshData;
}

void AssetManager::CleanUp()
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

AssetTextureContainer_t* AssetManager::AllocTextureContainer()
{
	AssetTextureContainer_t* pAssetTexContainer = new AssetTextureContainer_t;
	memset(pAssetTexContainer, 0, sizeof(AssetTextureContainer_t));
	return pAssetTexContainer;
}

AssetMeshDataContainer_t* AssetManager::AllocMeshDataContainer()
{
	AssetMeshDataContainer_t* pAssetMeshDataContainer = new AssetMeshDataContainer_t;
	memset(pAssetMeshDataContainer, 0, sizeof(AssetMeshDataContainer_t));
	return pAssetMeshDataContainer;
}

void AssetManager::FreeTextureContainer(AssetTextureContainer_t* pAssetTextureContainer)
{
	if (pAssetTextureContainer)
	{
		delete pAssetTextureContainer;
		pAssetTextureContainer = nullptr;
	}
}

void AssetManager::FreeMeshDataContainer(AssetMeshDataContainer_t* pAssetMeshDataContainer)
{
	if (pAssetMeshDataContainer)
	{
		delete pAssetMeshDataContainer;
		pAssetMeshDataContainer = nullptr;
	}
}
