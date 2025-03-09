#pragma once

/*
================
Asset Manager
================
*/
class AssetManager
{
public:
	~AssetManager();

	void AddMeshData(ASSET_MESH_DATA_TYPE eType, const wchar_t* wcBasePath, const wchar_t* wcModelFilename, AkF32 fScaleLength, AkBool bForAnim);
	void AddCubeMapTexture(const wchar_t* wcBasePath, const wchar_t* wcEnvFilename, const wchar_t* wcIrradianceFilename, const wchar_t* wcSpecularFilename, const wchar_t* wcBrdfFilaename);
	void AddDynamicTexture();
	void DeleteMeshData(ASSET_MESH_DATA_TYPE eType);
	void DeleteTexture(ASSET_TEXTURE_TYPE eType);
	void DeleteAnimation(ASSET_ANIM_TYPE eType);

	AssetMeshDataContainer_t* GetMeshDataContainer(ASSET_MESH_DATA_TYPE eType) { return _ppMeshDataContainerList[(AkU32)eType]; }
	AssetTextureContainer_t* GetTextureContainer(ASSET_TEXTURE_TYPE eType) { return _ppTextureContainerList[(AkU32)eType]; }
	AssetAnimationContainer_t* GetAnimationContainer(ASSET_ANIM_TYPE eType) { return _ppAnimContainerList[(AkU32)eType]; }

	MeshData_t* ReadFromFile(AssetMeshDataContainer_t* pAassetMeshDataContainer, AkU32* pMeshDataNum, const wchar_t* wcBasePath, const wchar_t* wcModelFilename, AkF32 fScaleLength, AkBool bForAnim);
	void ReadClip(ASSET_ANIM_TYPE eType, const wchar_t* wcFilePath, const wchar_t* wcFileName);

private:
	void CleanUp();

	AssetTextureContainer_t* AllocTextureContainer();
	AssetMeshDataContainer_t* AllocMeshDataContainer();
	AssetAnimationContainer_t* AllocAnimationContainer();
	void FreeTextureContainer(AssetTextureContainer_t* pTexContainer);
	void FreeMeshDataContainer(AssetMeshDataContainer_t* pMeshDataContainer);
	void FreeAnimationContainer(AssetAnimationContainer_t* pAnimContainer);

private:
	AssetMeshDataContainer_t* _ppMeshDataContainerList[(AkU32)ASSET_MESH_DATA_TYPE::COUNT] = {};
	AssetTextureContainer_t* _ppTextureContainerList[(AkU32)ASSET_TEXTURE_TYPE::COUNT] = {};
	AssetAnimationContainer_t* _ppAnimContainerList[(AkU32)ASSET_ANIM_TYPE::COUNT] = {};
};

