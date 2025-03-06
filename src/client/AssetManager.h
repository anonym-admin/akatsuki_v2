#pragma once

enum class ASSET_MESH_DATA_TYPE
{
	ASSET_MESH_DATA_TYPE_SWATGUY,
	ASSET_MESH_DATA_TYPE_DANCER,
	ASSET_MESH_DATA_TYPE_BRS_74,

	ASSET_MESH_DATA_TYPE_COUNT,
};

enum class ASSET_TEXTURE_TYPE
{
	ASSET_TEXTURE_TYPE_ENV,
	ASSET_TEXTURE_TYPE_IBL_IRRADIANCE,
	ASSET_TEXTURE_TYPE_IBL_SPECULAR,
	ASSET_TEXTURE_TYPE_IBL_BRDF,
	ASSET_TEXTURE_TYPE_DYNAMIC,

	ASSET_TEXTURE_TYPE_COUNT,
};

struct AssetMeshDataContainer_t
{
	MeshData_t* pMeshData = nullptr;
	AkU32 uMeshDataNum = 0;
	Matrix mDefaultMat = Matrix();
	const Matrix* pBoneOffsetMatList = nullptr;
	const AkI32* pBoneHierarchyList = nullptr;
	AkU32 uBoneNum = 0;
};

struct AssetTextureContainer_t
{
	void* pTexHandle = nullptr;
};

/*
================
Asset Manager
================
*/

class UApplication;

class UAssetManager
{
public:
	UAssetManager();
	~UAssetManager();

	AkBool Initialize(UApplication* pApp);

	void AddMeshData(ASSET_MESH_DATA_TYPE eType, const wchar_t* wcBasePath, const wchar_t* wcModelFilename, AkF32 fScaleLength, AkBool bForAnim);
	void AddCubeMapTexture(const wchar_t* wcBasePath, const wchar_t* wcEnvFilename, const wchar_t* wcIrradianceFilename, const wchar_t* wcSpecularFilename, const wchar_t* wcBrdfFilaename);
	void AddDynamicTexture();
	void DeleteMeshData(ASSET_MESH_DATA_TYPE eType);
	void DeleteTexture(ASSET_TEXTURE_TYPE eType);

	AssetMeshDataContainer_t* GetMeshDataContainer(ASSET_MESH_DATA_TYPE eType) { return _ppAssetMeshDataContainerList[(AkU32)eType]; }
	AssetTextureContainer_t* GetTextureContainer(ASSET_TEXTURE_TYPE eType) { return _ppAssetTextureContainerList[(AkU32)eType]; }

	MeshData_t* ReadFromFile(AssetMeshDataContainer_t* pAassetMeshDataContainer, AkU32* pMeshDataNum, const wchar_t* wcBasePath, const wchar_t* wcModelFilename, AkF32 fScaleLength, AkBool bForAnim);

private:
	void CleanUp();

	AssetTextureContainer_t* AllocTextureContainer();
	AssetMeshDataContainer_t* AllocMeshDataContainer();
	void FreeTextureContainer(AssetTextureContainer_t* pAssetTextureContainer);
	void FreeMeshDataContainer(AssetMeshDataContainer_t* pAssetMeshDataContainer);

private:
	UApplication* _pApp = nullptr;
	IRenderer* _pRenderer = nullptr;
	AssetMeshDataContainer_t* _ppAssetMeshDataContainerList[(AkU32)ASSET_MESH_DATA_TYPE::ASSET_MESH_DATA_TYPE_COUNT] = {};
	AssetTextureContainer_t* _ppAssetTextureContainerList[(AkU32)ASSET_TEXTURE_TYPE::ASSET_TEXTURE_TYPE_COUNT] = {};
};

