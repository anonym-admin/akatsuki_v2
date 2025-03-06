#pragma once

/*
=========================
FConstantBufferManager
=========================
*/

class FConstantBufferPool;

class FConstantBufferManager
{
public:
	FConstantBufferManager();
	~FConstantBufferManager();

	AkBool Initialize(ID3D12Device* pDevice, AkU32 uMaxCBVNum);
	void Reset();
	FConstantBufferPool* GetConstantBufferPool(CONSTANT_BUFFER_TYPE eCBType);

private:
	void CleanUp();

private:
	FConstantBufferPool* _ppConstantBufferPool[(AkU32)CONSTANT_BUFFER_TYPE::CONSTANT_BUFFER_TYPE_COUNT] = {};
};

