#include "pch.h"
#include "ConstantBufferManager.h"
#include "ConstantBufferPool.h"

ConstantBufferProperty_t g_pConstantbufferProperties[] =
{
	CONSTANT_BUFFER_TYPE::CONSTANT_BUFFER_TYPE_GLOBAL, sizeof(GlobalConstantBuffer_t),
	CONSTANT_BUFFER_TYPE::CONSTANT_BUFFER_TYPE_MESH, sizeof(MeshConstantBuffer_t),
	CONSTANT_BUFFER_TYPE::CONSTANT_BUFFER_TYPE_SKINNED_MESH, sizeof(SkinnedMeshConstantBuffer_t),
	CONSTANT_BUFFER_TYPE::CONSTANT_BUFFER_TYPE_MATERIAL, sizeof(MaterialConstantBuffer_t),
	CONSTANT_BUFFER_TYPE::CONSTANT_BUFFER_TYPE_SPRITE, sizeof(SpriteConstantBuffer_t)
};

/*
=========================
FConstantBufferManager
=========================
*/

FConstantBufferManager::FConstantBufferManager()
{
}

FConstantBufferManager::~FConstantBufferManager()
{
	CleanUp();
}

AkBool FConstantBufferManager::Initialize(ID3D12Device* pDevice, AkU32 uMaxCBVNum)
{
	for (AkU32 i = 0; i < (AkU32)CONSTANT_BUFFER_TYPE::CONSTANT_BUFFER_TYPE_COUNT; i++)
	{
		_ppConstantBufferPool[i] = new FConstantBufferPool;
		_ppConstantBufferPool[i]->Initialize(pDevice, g_pConstantbufferProperties + i, uMaxCBVNum);
	}

	return AK_TRUE;
}

void FConstantBufferManager::Reset()
{
	for (AkU32 i = 0; i < (AkU32)CONSTANT_BUFFER_TYPE::CONSTANT_BUFFER_TYPE_COUNT; i++)
	{
		_ppConstantBufferPool[i]->Reset();
	}
}

FConstantBufferPool* FConstantBufferManager::GetConstantBufferPool(CONSTANT_BUFFER_TYPE eCBType)
{
	if(eCBType >= CONSTANT_BUFFER_TYPE::CONSTANT_BUFFER_TYPE_COUNT)
	{
		__debugbreak();
		return nullptr;
	}

	return _ppConstantBufferPool[(AkU32)eCBType];
}

void FConstantBufferManager::CleanUp()
{
	for (AkU32 i = 0; i < (AkU32)CONSTANT_BUFFER_TYPE::CONSTANT_BUFFER_TYPE_COUNT; i++)
	{
		if (_ppConstantBufferPool[i])
		{
			delete _ppConstantBufferPool[i];
			_ppConstantBufferPool[i] = nullptr;
		}
	}
}
