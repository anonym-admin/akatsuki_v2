#pragma once

/*
======================
TextureManager
======================
*/

class FRenderer;
class FResourceManager;

class FTextureManager
{
public:
	FTextureManager();
	~FTextureManager();

	AkBool Initialize(FRenderer* pRenderer, AkU32 uMaxBucketNum, AkU32 uMaxFileNum);
	TextureHandle_t* CreateTextureFromFile(const wchar_t* wcFilename, AkBool bUseSRGB);
	TextureHandle_t* CreateCubeMapTexture(const wchar_t* wcFilename);
	TextureHandle_t* CreateDynamicTexture(AkU32 uTexWidth, AkU32 uTexHeight);
	TextureHandle_t* CreateNullTexture();
	void DestroyTexture(TextureHandle_t* pTexHandle);

private:
	void CleanUp();

	TextureHandle_t* AllocTextureHandle();
	void FreeTextureHandle(TextureHandle_t* pTexHandle);

private:
	FRenderer* _pRenderer = nullptr;
	FResourceManager* _pResourceManager = nullptr;
	HashTable_t* _pHashTable = nullptr;
	List_t* _pTexListHead = nullptr;
	List_t* _pTexListTail = nullptr;
};

