#include "pch.h"
#include "TextureManager.h"
#include "ResourceManager.h"
#include "Renderer.h"
#include "DescriptorAllocator.h"

/*
======================
TextureManager
======================
*/

FTextureManager::FTextureManager()
{
}

FTextureManager::~FTextureManager()
{
	CleanUp();
}

AkBool FTextureManager::Initialize(FRenderer* pRenderer, AkU32 uMaxBucketNum, AkU32 uMaxFileNum)
{
	_pRenderer = pRenderer;
	_pResourceManager = _pRenderer->GetResourceManager();

	_pHashTable = HT_CreateHashTable(uMaxBucketNum, _MAX_PATH, uMaxFileNum);

	return AK_TRUE;
}

TextureHandle_t* FTextureManager::CreateTextureFromFile(const wchar_t* wcFilename, AkBool bUseSRGB)
{
	ID3D12Device* pDevice = _pRenderer->GetDevice();
	FDescriptorAllocator* pDescriptorAllocator = _pRenderer->GetDescriptorAllocator();

	ID3D12Resource* pTexResouuce = nullptr;
	D3D12_CPU_DESCRIPTOR_HANDLE hSRV = {};
	D3D12_RESOURCE_DESC tDesc = {};
	TextureHandle_t* pTexHandle = nullptr;

	AkU32 uFileNameLen = static_cast<AkU32>(wcslen(wcFilename));
	AkU32 uKeyLength = uFileNameLen * sizeof(wchar_t);
	if (HT_Find(_pHashTable, reinterpret_cast<void**>(&pTexHandle), 1, wcFilename, uKeyLength))
	{
		pTexHandle->uRefCount++;
	}
	else
	{
		if (_pResourceManager->CreateTextureFromFile(&pTexResouuce, &tDesc, wcFilename, bUseSRGB))
		{
			D3D12_SHADER_RESOURCE_VIEW_DESC tSRVDesc = {};
			tSRVDesc.Format = tDesc.Format;
			tSRVDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
			tSRVDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
			tSRVDesc.Texture2D.MipLevels = tDesc.MipLevels;

			if (pDescriptorAllocator->AllocDescriptorHandle(&hSRV))
			{
				pDevice->CreateShaderResourceView(pTexResouuce, &tSRVDesc, hSRV);

				pTexHandle = AllocTextureHandle();
				pTexHandle->pTextureResource = pTexResouuce;
				pTexHandle->bFromFile = AK_TRUE;
				pTexHandle->hSRV = hSRV;

				pTexHandle->pSearchHandle = HT_Insert(_pHashTable, pTexHandle, wcFilename, uKeyLength);
				if (!pTexHandle->pSearchHandle)
				{
					__debugbreak();
					pTexResouuce->Release();
					pTexResouuce = nullptr;
					return nullptr;
				}
			}
			else
			{
				pTexResouuce->Release();
				pTexResouuce = nullptr;
				return pTexHandle;
			}
		}
	}

	return pTexHandle;
}

TextureHandle_t* FTextureManager::CreateCubeMapTexture(const wchar_t* wcFilename)
{
	ID3D12Device* pDevice = _pRenderer->GetDevice();
	FDescriptorAllocator* pDescriptorAllocator = _pRenderer->GetDescriptorAllocator();

	ID3D12Resource* pTexResouuce = nullptr;
	D3D12_CPU_DESCRIPTOR_HANDLE hSRV = {};
	D3D12_RESOURCE_DESC tDesc = {};
	TextureHandle_t* pTexHandle = nullptr;

	AkU32 uFileNameLen = static_cast<AkU32>(wcslen(wcFilename));
	AkU32 uKeyLength = uFileNameLen * sizeof(wchar_t);
	if (HT_Find(_pHashTable, reinterpret_cast<void**>(&pTexHandle), 1, wcFilename, uKeyLength))
	{
		pTexHandle->uRefCount++;
	}
	else
	{
		if (_pResourceManager->CreateCubeMapTexture(&pTexResouuce, &tDesc, wcFilename))
		{
			D3D12_SHADER_RESOURCE_VIEW_DESC tSRVDesc = {};
			tSRVDesc.Format = tDesc.Format;
			tSRVDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
			tSRVDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
			tSRVDesc.Texture2D.MipLevels = tDesc.MipLevels;

			if (pDescriptorAllocator->AllocDescriptorHandle(&hSRV))
			{
				pDevice->CreateShaderResourceView(pTexResouuce, &tSRVDesc, hSRV);

				pTexHandle = AllocTextureHandle();
				pTexHandle->pTextureResource = pTexResouuce;
				pTexHandle->bFromFile = AK_TRUE;
				pTexHandle->hSRV = hSRV;

				pTexHandle->pSearchHandle = HT_Insert(_pHashTable, pTexHandle, wcFilename, uKeyLength);
				if (!pTexHandle->pSearchHandle)
				{
					__debugbreak();
					pTexResouuce->Release();
					pTexResouuce = nullptr;
					return nullptr;
				}
			}
			else
			{
				pTexResouuce->Release();
				pTexResouuce = nullptr;
				return pTexHandle;
			}
		}
	}

	return pTexHandle;
}

TextureHandle_t* FTextureManager::CreateDynamicTexture(AkU32 uTexWidth, AkU32 uTexHeight)
{
	ID3D12Device* pDevice = _pRenderer->GetDevice();
	FDescriptorAllocator* pDescriptorAllocator = _pRenderer->GetDescriptorAllocator();
	TextureHandle_t* pTexHandle = nullptr;

	ID3D12Resource* pTexResource = nullptr;
	ID3D12Resource* pUploadBuffer = nullptr;
	D3D12_CPU_DESCRIPTOR_HANDLE hSRV = {};


	DXGI_FORMAT tTexFormat = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;

	if (_pResourceManager->CreateTextureWithUploadBuffer(&pTexResource, &pUploadBuffer, uTexWidth, uTexHeight, tTexFormat))
	{
		D3D12_SHADER_RESOURCE_VIEW_DESC SRVDesc = {};
		SRVDesc.Format = tTexFormat;
		SRVDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		SRVDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		SRVDesc.Texture2D.MipLevels = 1;

		if (pDescriptorAllocator->AllocDescriptorHandle(&hSRV))
		{
			pDevice->CreateShaderResourceView(pTexResource, &SRVDesc, hSRV);

			pTexHandle = AllocTextureHandle();
			pTexHandle->pTextureResource = pTexResource;
			pTexHandle->pUploadBuffer = pUploadBuffer;
			pTexHandle->hSRV = hSRV;
		}
		else
		{
			pTexResource->Release();
			pTexResource = nullptr;

			pUploadBuffer->Release();
			pUploadBuffer = nullptr;
		}
	}

	return pTexHandle;
}

TextureHandle_t* FTextureManager::CreateNullTexture()
{
	ID3D12Device* pDevice = _pRenderer->GetDevice();
	FDescriptorAllocator* pDescriptorAllocator = _pRenderer->GetDescriptorAllocator();
	D3D12_CPU_DESCRIPTOR_HANDLE hSRV = {};
	TextureHandle_t* pTexHandle = nullptr;

	if (pDescriptorAllocator->AllocDescriptorHandle(&hSRV))
	{
		D3D12_SHADER_RESOURCE_VIEW_DESC SRVDesc = {};
		SRVDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
		SRVDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		SRVDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		SRVDesc.Texture2D.MipLevels = 1;

		pDevice->CreateShaderResourceView(nullptr, &SRVDesc, hSRV);

		pTexHandle = AllocTextureHandle();
		pTexHandle->pTextureResource = nullptr;
		pTexHandle->pUploadBuffer = nullptr;
		pTexHandle->hSRV = hSRV;
	}

	return pTexHandle;
}

void FTextureManager::DestroyTexture(TextureHandle_t* pTexHandle)
{
	FreeTextureHandle(pTexHandle);
}

void FTextureManager::CleanUp()
{
	if (_pTexListHead)
	{
		printf("%p \n", _pTexListHead->pData);
		__debugbreak();
		return;
	}
	if (_pHashTable)
	{
		HT_DestroyHashTable(_pHashTable);
		_pHashTable = nullptr;
	}
}

TextureHandle_t* FTextureManager::AllocTextureHandle()
{
	TextureHandle_t* pTexHandle = reinterpret_cast<TextureHandle_t*>(malloc(sizeof(TextureHandle_t)));
	memset(pTexHandle, 0, sizeof(TextureHandle_t));

	pTexHandle->tLink.pData = pTexHandle;
	pTexHandle->uRefCount = 1;
	LL_PushBack(&_pTexListHead, &_pTexListTail, &pTexHandle->tLink);

	return pTexHandle;
}

void FTextureManager::FreeTextureHandle(TextureHandle_t* pTexHandle)
{
	ID3D12Device* pDevice = _pRenderer->GetDevice();
	FDescriptorAllocator* pDescriptorAllocator = _pRenderer->GetDescriptorAllocator();

	if (!pTexHandle->uRefCount)
	{
		__debugbreak();
		return;
	}

	AkU32 uRefCount = --pTexHandle->uRefCount;
	if (!uRefCount)
	{
		if (pTexHandle->pTextureResource)
		{
			pTexHandle->pTextureResource->Release();
			pTexHandle->pTextureResource = nullptr;
		}
		if (pTexHandle->pUploadBuffer)
		{
			pTexHandle->pUploadBuffer->Release();
			pTexHandle->pUploadBuffer = nullptr;
		}
		if (pTexHandle->hSRV.ptr)
		{
			pDescriptorAllocator->FreeDescriptorHandle(pTexHandle->hSRV);
			pTexHandle->hSRV = {};
		}
		if (pTexHandle->pSearchHandle)
		{
			HT_Delete(_pHashTable, pTexHandle->pSearchHandle);
			pTexHandle->pSearchHandle = nullptr;
		}

		// For Debugging.
		// printf("Delete Texture [%p]\n", pTexHandle->tLink.pData); 

		LL_Delete(&_pTexListHead, &_pTexListTail, &pTexHandle->tLink);

		free(pTexHandle);
	}
}
