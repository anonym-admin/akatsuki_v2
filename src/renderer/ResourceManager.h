#pragma once

/*
====================
FResourceManager
====================
*/

class FResourceManager
{
public:
	FResourceManager();
	~FResourceManager();

	AkBool Initialize(ID3D12Device* pDevice);
	AkBool CreateVertexBuffer(AkU32 uSizePerVertex, AkU32 uVertexNum, D3D12_VERTEX_BUFFER_VIEW* pOutVertexBufferView, ID3D12Resource** ppOutBuffer, void* pInitData);
	AkBool CreateIndexBuffer(AkU32 uIndexNum, D3D12_INDEX_BUFFER_VIEW* pOutIndexBufferView, ID3D12Resource** ppOutBuffer, void* pInitData);
	AkBool CreateTextureFromFile(ID3D12Resource** ppTexResource, D3D12_RESOURCE_DESC* pResourceDesc, const wchar_t* wcFilename, AkBool bUseSRGB);
	AkBool CreateCubeMapTexture(ID3D12Resource** ppTexResource, D3D12_RESOURCE_DESC* pResourceDesc, const wchar_t* wcFilename);
	AkBool CreateTextureWithUploadBuffer(ID3D12Resource** ppTexResource, ID3D12Resource** ppUploadBuffer, AkU32 uWidth, AkU32 uHeight, DXGI_FORMAT tFormat);

private:
	void CleanUp();

	AkBool CreateCmdQueue();
	AkBool CreateFence();
	AkBool CreateCmdList();
	void DestroyCmdQueue();
	void DestroyFence();
	void DestroyCmdList();

	void Fence();
	void WaitForFenceValue();

private:
	ID3D12Device* _pDevice = nullptr;
	ID3D12CommandQueue* _pCmdQueue = nullptr;
	ID3D12CommandAllocator* _pCmdAllocator = nullptr;
	ID3D12GraphicsCommandList* _pCmdList = nullptr;
	HANDLE	_hFenceEvent = nullptr;
	ID3D12Fence* _pFence = nullptr;
	AkU64 _u64FenceValue = 0;
};

