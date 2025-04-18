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

void AssetManager::AddMeshData(const wchar_t* wcBasePath, const wchar_t* wcModelFilename, AkF32 fScaleLength, AkBool bForAnim)
{
	SceneLoading* pSceneLoading = nullptr;
	SCENE_TYPE eType = GSceneManager->GetCurrentSceneType();
	if (eType == SCENE_TYPE::LOADING)
	{
		pSceneLoading = (SceneLoading*)GSceneManager->GetCurrentScene();
	}

	AssetMeshDataContainer_t* pAssetMeshDataContainer = nullptr;

	pAssetMeshDataContainer = AllocMeshDataContainer();

	pAssetMeshDataContainer->pMeshData = ReadFromFile(pAssetMeshDataContainer, &pAssetMeshDataContainer->uMeshDataNum, wcBasePath, wcModelFilename, fScaleLength, bForAnim);

	wchar_t wcFullPath[MAX_PATH] = {};
	wcscpy_s(wcFullPath, wcBasePath);
	wcscat_s(wcFullPath, wcModelFilename);

	if(!_mapMeshDataList.count(wcModelFilename))
	{
		_mapMeshDataList[wcModelFilename] = pAssetMeshDataContainer;
	}

	wcscat_s(wcFullPath, L"\n");

	if(pSceneLoading)
	{
		// pSceneLoading->RenderLoadingScreenCallBack(wcFullPath);
	}
}

void AssetManager::AddCubeMapTexture(const wchar_t* wcBasePath, const wchar_t* wcEnvFilename, const wchar_t* wcIrradianceFilename, const wchar_t* wcSpecularFilename, const wchar_t* wcBrdfFilaename)
{
	SceneLoading* pSceneLoading = nullptr;
	if (GSceneManager->GetCurrentSceneType() == SCENE_TYPE::LOADING)
	{
		pSceneLoading = (SceneLoading*)GSceneManager->GetCurrentScene();
	}

	AssetTextureContainer_t* pAssetTexContainer = nullptr;
	void* pTexHandle = nullptr;

	wchar_t wcFullPath[MAX_PATH] = {};
	wcscpy_s(wcFullPath, wcBasePath);
	wcscat_s(wcFullPath, wcEnvFilename);

	// Env
	if(!_mapTextures.count(wcEnvFilename))
	{
		pTexHandle = GRenderer->CreateCubeMapTexture(wcFullPath);
		pAssetTexContainer = AllocTextureContainer();
		pAssetTexContainer->pTexHandle = pTexHandle;

		_mapTextures[wcEnvFilename] = pAssetTexContainer;
	}

	wcscat_s(wcFullPath, L"\n");
	if (pSceneLoading)
	{
		// pSceneLoading->RenderLoadingScreenCallBack(wcFullPath);
	}

	memset(wcFullPath, 0, sizeof(wchar_t) * MAX_PATH);
	wcscpy_s(wcFullPath, wcBasePath);
	wcscat_s(wcFullPath, wcIrradianceFilename);

	// Irradiance
	if (!_mapTextures.count(wcIrradianceFilename))
	{
		pTexHandle = GRenderer->CreateCubeMapTexture(wcFullPath);
		pAssetTexContainer = AllocTextureContainer();
		pAssetTexContainer->pTexHandle = pTexHandle;

		_mapTextures[wcIrradianceFilename] = pAssetTexContainer;
	}

	wcscat_s(wcFullPath, L"\n");
	if (pSceneLoading)
	{
		// pSceneLoading->RenderLoadingScreenCallBack(wcFullPath);
	}

	memset(wcFullPath, 0, sizeof(wchar_t) * MAX_PATH);
	wcscpy_s(wcFullPath, wcBasePath);
	wcscat_s(wcFullPath, wcSpecularFilename);

	// Specular
	if (!_mapTextures.count(wcSpecularFilename))
	{
		pTexHandle = GRenderer->CreateCubeMapTexture(wcFullPath);
		pAssetTexContainer = AllocTextureContainer();
		pAssetTexContainer->pTexHandle = pTexHandle;

		_mapTextures[wcSpecularFilename] = pAssetTexContainer;
	}

	memset(wcFullPath, 0, sizeof(wchar_t) * MAX_PATH);
	wcscpy_s(wcFullPath, wcBasePath);
	wcscat_s(wcFullPath, wcBrdfFilaename);

	// Brdf
	if (!_mapTextures.count(wcBrdfFilaename))
	{
		pTexHandle = GRenderer->CreateTextureFromFile(wcFullPath, AK_FALSE);
		pAssetTexContainer = AllocTextureContainer();
		pAssetTexContainer->pTexHandle = pTexHandle;

		_mapTextures[wcBrdfFilaename] = pAssetTexContainer;
	}

	wcscat_s(wcFullPath, L"\n");
	if (pSceneLoading)
	{
		// pSceneLoading->RenderLoadingScreenCallBack(wcFullPath);
	}
}

void AssetManager::AddDynamicTexture()
{
}

void AssetManager::DeleteMeshData(const wchar_t* wcKey)
{
	AssetMeshDataContainer_t* pMeshDataContainer = _mapMeshDataList[wcKey];

	if (pMeshDataContainer)
	{
		if (pMeshDataContainer->pMeshData)
		{
			for (AkU32 i = 0; i < pMeshDataContainer->uMeshDataNum; i++)
			{
				if (pMeshDataContainer->pMeshData[i].pVertices)
				{
					free(pMeshDataContainer->pMeshData[i].pVertices);
					pMeshDataContainer->pMeshData[i].pVertices = nullptr;
				}
				if (pMeshDataContainer->pMeshData[i].pSkinnedVertices)
				{
					free(pMeshDataContainer->pMeshData[i].pSkinnedVertices);
					pMeshDataContainer->pMeshData[i].pSkinnedVertices = nullptr;
				}
				if (pMeshDataContainer->pMeshData[i].pIndices)
				{
					free(pMeshDataContainer->pMeshData[i].pIndices);
					pMeshDataContainer->pMeshData[i].pIndices = nullptr;
				}
			}

			free(pMeshDataContainer->pMeshData);
			pMeshDataContainer->pMeshData = nullptr;
		}

		if (pMeshDataContainer->pBoneHierarchyList)
		{
			delete[] pMeshDataContainer->pBoneHierarchyList;
			pMeshDataContainer->pBoneHierarchyList = nullptr;
		}

		if (pMeshDataContainer->pBoneOffsetMatrixList)
		{
			delete[] pMeshDataContainer->pBoneOffsetMatrixList;
			pMeshDataContainer->pBoneOffsetMatrixList = nullptr;
		}

		FreeMeshDataContainer(pMeshDataContainer);
		pMeshDataContainer = nullptr;
	}

	_mapMeshDataList.erase(wcKey);
}

void AssetManager::DeleteAllMeshData()
{
	for (auto& e : _mapMeshDataList)
	{
		if (e.second)
		{
			if (e.second->pMeshData)
			{
				for (AkU32 i = 0; i < e.second->uMeshDataNum; i++)
				{
					if (e.second->pMeshData[i].pVertices)
					{
						free(e.second->pMeshData[i].pVertices);
						e.second->pMeshData[i].pVertices = nullptr;
					}
					if (e.second->pMeshData[i].pSkinnedVertices)
					{
						free(e.second->pMeshData[i].pSkinnedVertices);
						e.second->pMeshData[i].pSkinnedVertices = nullptr;
					}
					if (e.second->pMeshData[i].pIndices)
					{
						free(e.second->pMeshData[i].pIndices);
						e.second->pMeshData[i].pIndices = nullptr;
					}
				}

				free(e.second->pMeshData);
				e.second->pMeshData = nullptr;
			}

			//if (e.second->pBoneHierarchyList)
			//{
			//	delete[] e.second->pBoneHierarchyList;
			//	e.second->pBoneHierarchyList = nullptr;
			//}

			//if (e.second->pBoneOffsetMatrixList)
			//{
			//	delete[] e.second->pBoneOffsetMatrixList;
			//	e.second->pBoneOffsetMatrixList = nullptr;
			//}

			//if (e.second->ppBoneName)
			//{
			//	for(AkU32 i = 0; i < e.second->uBoneNum; i++)
			//	{
			//		delete[] e.second->ppBoneName[i];
			//		e.second->ppBoneName[i] = nullptr;
			//	}

			//	delete[] e.second->ppBoneName;
			//	e.second->ppBoneName;
			//}

			FreeMeshDataContainer(e.second);
			e.second = nullptr;
		}
	}

	_mapMeshDataList.clear();
}

void AssetManager::DeleteTexture(const wchar_t* wcKey)
{
	AssetTextureContainer_t* pTextureContainer = _mapTextures[wcKey];
	if (pTextureContainer)
	{
		if (pTextureContainer->pTexHandle)
		{
			GRenderer->DestroyTexture(pTextureContainer->pTexHandle);
			pTextureContainer->pTexHandle = nullptr;
		}

		FreeTextureContainer(pTextureContainer);
	}

	_mapTextures.erase(wcKey);
}

void AssetManager::DeleteAllTexture()
{
	for (auto& e : _mapTextures)
	{
		if (e.second)
		{
			if(e.second->pTexHandle)
			{
				GRenderer->DestroyTexture(e.second->pTexHandle);
				e.second->pTexHandle = nullptr;
			}

			FreeTextureContainer(e.second);
			e.second = nullptr;
		}
	}

	_mapTextures.clear();
}

void AssetManager::DeleteAnimation(const wchar_t* wcKey)
{
	AssetAnimationContainer_t* pAnimationContainer = _mapAnimation[wcKey];
	if (pAnimationContainer)
	{
		if (pAnimationContainer->pAnim)
		{
			delete pAnimationContainer->pAnim;
			pAnimationContainer->pAnim = nullptr;
		}

		FreeAnimationContainer(pAnimationContainer);
	}

	_mapAnimation.erase(wcKey);
}

void AssetManager::DeleteAllAnimation()
{
	for (auto& e : _mapAnimation)
	{
		if (e.second)
		{
			if (e.second->pAnim)
			{
				delete e.second->pAnim;
				e.second->pAnim = nullptr;
			}

			FreeAnimationContainer(e.second);
			e.second = nullptr;
		}
	}

	_mapAnimation.clear();
}

AssetMeshDataContainer_t* AssetManager::GetMeshData(const wchar_t* wcKey)
{
	if (!_mapMeshDataList.count(wcKey))
	{
		return nullptr;
	}

	AssetMeshDataContainer_t* pResult = _mapMeshDataList[wcKey];
	return pResult;
}

AssetTextureContainer_t* AssetManager::GetTexture(const wchar_t* wcKey)
{
	if (!_mapTextures.count(wcKey))
	{
		return nullptr;
	}

	AssetTextureContainer_t* pResult = _mapTextures[wcKey];
	return pResult;
}

AssetAnimationContainer_t* AssetManager::GetAnimation(const wchar_t* wcKey)
{
	if (!_mapAnimation.count(wcKey))
	{
		return nullptr;
	}

	AssetAnimationContainer_t* pResult = _mapAnimation[wcKey];
	return pResult;
}

MeshData_t* AssetManager::ReadFromFile(AssetMeshDataContainer_t* pAssetMeshDataContainer, AkU32* pMeshDataNum, const wchar_t* wcBasePath, const wchar_t* wcModelFilename, AkF32 fScaleLength, AkBool bForAnim)
{
	ModelImporter tModelImporter = {};
	MeshData_t* pMeshData = nullptr;

	tModelImporter.Load(wcBasePath, wcModelFilename, bForAnim);

	pMeshData = tModelImporter.GetMeshData();
	*pMeshDataNum = tModelImporter.GetMeshDataNum();

	GeometryGenerator::NormalizeMeshData(pMeshData, *pMeshDataNum, fScaleLength, bForAnim, &pAssetMeshDataContainer->mDefaultMat);

	if (bForAnim)
	{
		pAssetMeshDataContainer->pBoneOffsetMatrixList = tModelImporter.GetBoneOffsetTransformList();
		pAssetMeshDataContainer->pBoneHierarchyList = tModelImporter.GetBoneHierarchyList();
		pAssetMeshDataContainer->uBoneNum = tModelImporter.GetBoneNum();
		pAssetMeshDataContainer->ppBoneName = tModelImporter.GetBoneName();
	}

	return pMeshData;
}

void AssetManager::ReadClip(const wchar_t* wcModel, const wchar_t* wcAnim)
{
	SceneLoading* pSceneLoading = nullptr;
	if (GSceneManager->GetCurrentSceneType() == SCENE_TYPE::LOADING)
	{
		pSceneLoading = (SceneLoading*)GSceneManager->GetCurrentScene();
	}

	wchar_t wcFullPath[MAX_PATH] = {};
	wcscpy_s(wcFullPath, CLIP_FILE_PATH);
	wcscat_s(wcFullPath, wcModel);
	wcscat_s(wcFullPath, L"/");

	wchar_t wcModelName[MAX_PATH] = {};
	wcscpy_s(wcModelName, wcModel);
	wcscat_s(wcModelName, L".mesh");

	if (!_mapAnimation[wcModel])
	{
		_mapAnimation[wcModel] = AllocAnimationContainer();
		_mapAnimation[wcModel]->pAnim = new Animation(GetMeshData(wcModelName), wcAnim, _mapAnimation[wcModel]->MAX_CLIP_NAME_COUNT);
		wcscpy_s(_mapAnimation[wcModel]->wcClipName[0], MAX_PATH, wcAnim);
	}
	else
	{
		for (AkU32 i = 0; i < _mapAnimation[wcModel]->MAX_CLIP_NAME_COUNT; i++)
		{
			if (!wcslen(_mapAnimation[wcModel]->wcClipName[i]))
			{
				wcscpy_s(_mapAnimation[wcModel]->wcClipName[i], MAX_PATH, wcAnim);
				break;
			}
		}
	}

	_mapAnimation[wcModel]->pAnim->ReadClip(wcFullPath, wcAnim);

	wcscat_s(wcFullPath, wcAnim);

	wcscat_s(wcFullPath, L"\n");
	if(pSceneLoading)
	{
		// pSceneLoading->RenderLoadingScreenCallBack(wcFullPath);
	}
}

void AssetManager::CleanUp()
{
	DeleteAllMeshData();
	DeleteAllTexture();
	DeleteAllAnimation();
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

