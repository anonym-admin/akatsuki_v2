#include "pch.h"
#include "AssetManager.h"
#include "SceneLoading.h"
#include "ModelImporter.h"

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

	_ppMeshDataContainerList[(AkU32)eType] = pAssetMeshDataContainer;

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

	_ppTextureContainerList[(AkU32)ASSET_TEXTURE_TYPE::ENV] = pAssetTexContainer;

	wcscat_s(wcFullPath, L"\n");
	pSceneLoading->RenderLoadingScreenCallBack(wcFullPath);

	memset(wcFullPath, 0, sizeof(wchar_t) * MAX_PATH);
	wcscpy_s(wcFullPath, wcBasePath);
	wcscat_s(wcFullPath, wcIrradianceFilename);

	// Irradiance
	pTexHandle = GRenderer->CreateCubeMapTexture(wcFullPath);
	pAssetTexContainer = AllocTextureContainer();
	pAssetTexContainer->pTexHandle = pTexHandle;

	_ppTextureContainerList[(AkU32)ASSET_TEXTURE_TYPE::IRRADIANCE] = pAssetTexContainer;

	wcscat_s(wcFullPath, L"\n");
	pSceneLoading->RenderLoadingScreenCallBack(wcFullPath);

	memset(wcFullPath, 0, sizeof(wchar_t) * MAX_PATH);
	wcscpy_s(wcFullPath, wcBasePath);
	wcscat_s(wcFullPath, wcSpecularFilename);

	// Specular
	pTexHandle = GRenderer->CreateCubeMapTexture(wcFullPath);
	pAssetTexContainer = AllocTextureContainer();
	pAssetTexContainer->pTexHandle = pTexHandle;

	_ppTextureContainerList[(AkU32)ASSET_TEXTURE_TYPE::SPECULAR] = pAssetTexContainer;

	wcscat_s(wcFullPath, L"\n");
	pSceneLoading->RenderLoadingScreenCallBack(wcFullPath);

	memset(wcFullPath, 0, sizeof(wchar_t) * MAX_PATH);
	wcscpy_s(wcFullPath, wcBasePath);
	wcscat_s(wcFullPath, wcBrdfFilaename);

	// Brdf
	pTexHandle = GRenderer->CreateTextureFromFile(wcFullPath, AK_FALSE);
	pAssetTexContainer = AllocTextureContainer();
	pAssetTexContainer->pTexHandle = pTexHandle;

	_ppTextureContainerList[(AkU32)ASSET_TEXTURE_TYPE::BRDF] = pAssetTexContainer;

	wcscat_s(wcFullPath, L"\n");
	pSceneLoading->RenderLoadingScreenCallBack(wcFullPath);
}

void AssetManager::AddDynamicTexture()
{
}

void AssetManager::DeleteMeshData(ASSET_MESH_DATA_TYPE eType)
{
	AkU32 uType = (AkU32)eType;

	if (_ppMeshDataContainerList[uType])
	{
		if (_ppMeshDataContainerList[uType]->pMeshData)
		{
			for (AkU32 i = 0; i < _ppMeshDataContainerList[uType]->uMeshDataNum; i++)
			{
				if (_ppMeshDataContainerList[uType]->pMeshData[i].pVertices)
				{
					free(_ppMeshDataContainerList[uType]->pMeshData[i].pVertices);
					_ppMeshDataContainerList[uType]->pMeshData[i].pVertices = nullptr;
				}
				if (_ppMeshDataContainerList[uType]->pMeshData[i].pSkinnedVertices)
				{
					free(_ppMeshDataContainerList[uType]->pMeshData[i].pSkinnedVertices);
					_ppMeshDataContainerList[uType]->pMeshData[i].pSkinnedVertices = nullptr;
				}
				if (_ppMeshDataContainerList[uType]->pMeshData[i].pIndices)
				{
					free(_ppMeshDataContainerList[uType]->pMeshData[i].pIndices);
					_ppMeshDataContainerList[uType]->pMeshData[i].pIndices = nullptr;
				}
			}

			free(_ppMeshDataContainerList[uType]->pMeshData);
			_ppMeshDataContainerList[uType]->pMeshData = nullptr;
		}

		if (_ppMeshDataContainerList[uType]->pBoneHierarchyList)
		{
			delete[] _ppMeshDataContainerList[uType]->pBoneHierarchyList;
			_ppMeshDataContainerList[uType]->pBoneHierarchyList = nullptr;
		}

		if (_ppMeshDataContainerList[uType]->pBoneOffsetMatrixList)
		{
			delete[] _ppMeshDataContainerList[uType]->pBoneOffsetMatrixList;
			_ppMeshDataContainerList[uType]->pBoneOffsetMatrixList = nullptr;
		}

		FreeMeshDataContainer(_ppMeshDataContainerList[uType]);
		_ppMeshDataContainerList[uType] = nullptr;
	}
}

void AssetManager::DeleteTexture(ASSET_TEXTURE_TYPE eType)
{
	AkU32 uType = (AkU32)eType;

	if (_ppTextureContainerList[uType])
	{
		GRenderer->DestroyTexture(_ppTextureContainerList[uType]->pTexHandle);
		_ppTextureContainerList[uType]->pTexHandle = nullptr;

		FreeTextureContainer(_ppTextureContainerList[uType]);
		_ppTextureContainerList[uType] = nullptr;
	}
}

void AssetManager::DeleteAnimation(ASSET_ANIM_TYPE eType)
{
	AkU32 uType = (AkU32)eType;

	if (_ppAnimContainerList[uType])
	{
		if (_ppAnimContainerList[uType]->pAnim)
		{
			delete _ppAnimContainerList[uType]->pAnim;
			_ppAnimContainerList[uType]->pAnim = nullptr;
		}

		FreeAnimationContainer(_ppAnimContainerList[uType]);
		_ppAnimContainerList[uType] = nullptr;
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

void AssetManager::ReadClip(ASSET_ANIM_TYPE eType, const wchar_t* wcFilePath, const wchar_t* wcFileName)
{
	SceneLoading* pSceneLoading = (SceneLoading*)GSceneManager->GetCurrentScene();

	wchar_t wcFullPath[MAX_PATH] = {};
	wcscpy_s(wcFullPath, wcFilePath);
	wcscat_s(wcFullPath, wcFileName);

	if (!_ppAnimContainerList[(AkU32)eType])
	{
		_ppAnimContainerList[(AkU32)eType] = AllocAnimationContainer();
		_ppAnimContainerList[(AkU32)eType]->pAnim = new Animation(GetMeshDataContainer((ASSET_MESH_DATA_TYPE)eType), wcFileName, _ppAnimContainerList[(AkU32)eType]->MAX_CLIP_NAME_COUNT);
		_ppAnimContainerList[(AkU32)eType]->wcClipName[0] = wcFileName;
	}
	else
	{
		for (AkU32 i = 0; i < _ppAnimContainerList[(AkU32)eType]->MAX_CLIP_NAME_COUNT; i++)
		{
			if (!_ppAnimContainerList[(AkU32)eType]->wcClipName[i])
			{
				_ppAnimContainerList[(AkU32)eType]->wcClipName[i] = wcFileName;
				break;
			}
		}
	}

	_ppAnimContainerList[(AkU32)eType]->pAnim->ReadClip(wcFilePath, wcFileName);

	wcscat_s(wcFullPath, L"\n");
	pSceneLoading->RenderLoadingScreenCallBack(wcFullPath);
}

void AssetManager::CleanUp()
{
	for (AkU32 i = 0; i < (AkU32)ASSET_MESH_DATA_TYPE::COUNT; i++)
	{
		DeleteMeshData((ASSET_MESH_DATA_TYPE)i);
	}
	for (AkU32 i = 0; i < (AkU32)ASSET_TEXTURE_TYPE::COUNT; i++)
	{
		DeleteTexture((ASSET_TEXTURE_TYPE)i);
	}
	for (AkU32 i = 0; i < (AkU32)ASSET_ANIM_TYPE::COUNT; i++)
	{
		DeleteAnimation((ASSET_ANIM_TYPE)i);
	}
}

AssetTextureContainer_t* AssetManager::AllocTextureContainer()
{
	AssetTextureContainer_t* pTexContainer = new AssetTextureContainer_t;
	memset(pTexContainer, 0, sizeof(AssetTextureContainer_t));
	return pTexContainer;
}

AssetMeshDataContainer_t* AssetManager::AllocMeshDataContainer()
{
	AssetMeshDataContainer_t* pMeshDataContainer = new AssetMeshDataContainer_t;
	memset(pMeshDataContainer, 0, sizeof(AssetMeshDataContainer_t));
	return pMeshDataContainer;
}

AssetAnimationContainer_t* AssetManager::AllocAnimationContainer()
{
	AssetAnimationContainer_t* pAnimContainer = new AssetAnimationContainer_t;
	memset(pAnimContainer, 0, sizeof(AssetAnimationContainer_t));
	return pAnimContainer;
}

void AssetManager::FreeTextureContainer(AssetTextureContainer_t* pTexContainer)
{
	if (pTexContainer)
	{
		delete pTexContainer;
	}
}

void AssetManager::FreeMeshDataContainer(AssetMeshDataContainer_t* pMeshDataContainer)
{
	if (pMeshDataContainer)
	{
		delete pMeshDataContainer;
	}
}

void AssetManager::FreeAnimationContainer(AssetAnimationContainer_t* pAnimContainer)
{
	if (pAnimContainer)
	{
		delete pAnimContainer;
	}
}

