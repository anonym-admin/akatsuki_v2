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

	void AddMeshData(const wchar_t* wcBasePath, const wchar_t* wcModelFilename, AkF32 fScaleLength, AkBool bForAnim);
	void AddCubeMapTexture(const wchar_t* wcBasePath, const wchar_t* wcEnvFilename, const wchar_t* wcIrradianceFilename, const wchar_t* wcSpecularFilename, const wchar_t* wcBrdfFilaename);
	void AddDynamicTexture();

	void DeleteMeshData(const wchar_t* wcKey);
	void DeleteAllMeshData();

	void DeleteTexture(const wchar_t* wcKey);
	void DeleteAllTexture();

	void DeleteAnimation(const wchar_t* wcKey);
	void DeleteAllAnimation();

	AssetMeshDataContainer_t* GetMeshData(const wchar_t* wcKey);
	AssetTextureContainer_t* GetTexture(const wchar_t* wcKey);
	AssetAnimationContainer_t* GetAnimation(const wchar_t* wcKey);

	MeshData_t* ReadFromFile(AssetMeshDataContainer_t* pAassetMeshDataContainer, AkU32* pMeshDataNum, const wchar_t* wcBasePath, const wchar_t* wcModelFilename, AkF32 fScaleLength, AkBool bForAnim);
	void ReadClip(const wchar_t* wcModel, const wchar_t* wcAnim);

private:
	void CleanUp();

	AssetTextureContainer_t* AllocTextureContainer();
	AssetMeshDataContainer_t* AllocMeshDataContainer();
	AssetAnimationContainer_t* AllocAnimationContainer();
	void FreeTextureContainer(AssetTextureContainer_t* pTexContainer);
	void FreeMeshDataContainer(AssetMeshDataContainer_t* pMeshDataContainer);
	void FreeAnimationContainer(AssetAnimationContainer_t* pAnimContainer);

private:
	std::unordered_map<std::wstring, AssetMeshDataContainer_t*> _mapMeshDataList = {};
	std::unordered_map<std::wstring, AssetTextureContainer_t*> _mapTextures = {};
	std::unordered_map<std::wstring, AssetAnimationContainer_t*> _mapAnimation = {};
};

