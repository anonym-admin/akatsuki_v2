#include "pch.h"
#include "ResourceManager.h"

/*
====================
FResourceManager
====================
*/

FResourceManager::FResourceManager()
{
}

FResourceManager::~FResourceManager()
{
	CleanUp();
}

AkBool FResourceManager::Initialize(ID3D12Device* pDevice)
{
	_pDevice = pDevice;

	if (!CreateCmdQueue())
	{
		__debugbreak();
		return AK_FALSE;
	}
	if (!CreateCmdList())
	{
		__debugbreak();
		return AK_FALSE;
	}
	if (!CreateFence())
	{
		__debugbreak();
		return AK_FALSE;
	}

	return AK_TRUE;
}

AkBool FResourceManager::CreateVertexBuffer(AkU32 uSizePerVertex, AkU32 uVertexNum, D3D12_VERTEX_BUFFER_VIEW* pOutVertexBufferView, ID3D12Resource** ppOutBuffer, void* pInitData)
{
	D3D12_VERTEX_BUFFER_VIEW tVertexBufferView = {};
	ID3D12Resource* pVertexBuffer = nullptr;
	ID3D12Resource* pUploadBuffer = nullptr;
	AkU32 uVertexBufferSize = uSizePerVertex * uVertexNum;

	// create vertexbuffer for rendering
	if (_pDevice->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(uVertexBufferSize),
		D3D12_RESOURCE_STATE_COMMON,
		nullptr,
		IID_PPV_ARGS(&pVertexBuffer)))
	{
		__debugbreak();
		return AK_FALSE;
	}

	if (pInitData)
	{
		if (FAILED(_pCmdAllocator->Reset()))
		{
			__debugbreak();
			return AK_FALSE;
		}

		if (FAILED(_pCmdList->Reset(_pCmdAllocator, nullptr)))
		{
			__debugbreak();
			return AK_FALSE;
		}

		if (_pDevice->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Buffer(uVertexBufferSize),
			D3D12_RESOURCE_STATE_COMMON,
			nullptr,
			IID_PPV_ARGS(&pUploadBuffer)))
		{
			__debugbreak();
			return AK_FALSE;
		}

		// Copy the triangle data to the vertex buffer.
		AkU8* pVertexDataBegin = nullptr;
		CD3DX12_RANGE tReadRange(0, 0);        // We do not intend to read from this resource on the CPU.

		if (pUploadBuffer->Map(0, &tReadRange, reinterpret_cast<void**>(&pVertexDataBegin)))
		{
			__debugbreak();
			return AK_FALSE;
		}

		memcpy(pVertexDataBegin, pInitData, uVertexBufferSize);
		pUploadBuffer->Unmap(0, nullptr);

		_pCmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(pVertexBuffer, D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_COPY_DEST));
		_pCmdList->CopyBufferRegion(pVertexBuffer, 0, pUploadBuffer, 0, uVertexBufferSize);
		_pCmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(pVertexBuffer, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER));

		_pCmdList->Close();

		ID3D12CommandList* ppCmdLists[] = { _pCmdList };
		_pCmdQueue->ExecuteCommandLists(_countof(ppCmdLists), ppCmdLists);

		Fence();
		WaitForFenceValue();
	}

	// Initialize the vertex buffer view.
	tVertexBufferView.BufferLocation = pVertexBuffer->GetGPUVirtualAddress();
	tVertexBufferView.StrideInBytes = uSizePerVertex;
	tVertexBufferView.SizeInBytes = uVertexBufferSize;

	*pOutVertexBufferView = tVertexBufferView;
	*ppOutBuffer = pVertexBuffer;

	if (pUploadBuffer)
	{
		pUploadBuffer->Release();
		pUploadBuffer = nullptr;
	}

	return AK_TRUE;
}

AkBool FResourceManager::CreateIndexBuffer(AkU32 uIndexNum, D3D12_INDEX_BUFFER_VIEW* pOutIndexBufferView, ID3D12Resource** ppOutBuffer, void* pInitData)
{
	D3D12_INDEX_BUFFER_VIEW	IndexBufferView = {};
	ID3D12Resource* pIndexBuffer = nullptr;
	ID3D12Resource* pUploadBuffer = nullptr;
	AkU32 uIndexBufferSize = sizeof(AkU32) * uIndexNum;

	// create vertexbuffer for rendering
	if (_pDevice->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(uIndexBufferSize),
		D3D12_RESOURCE_STATE_COMMON,
		nullptr,
		IID_PPV_ARGS(&pIndexBuffer)))
	{
		__debugbreak();
		return AK_FALSE;
	}

	if (pInitData)
	{
		if (FAILED(_pCmdAllocator->Reset()))
		{
			__debugbreak();
			return AK_FALSE;
		}

		if (FAILED(_pCmdList->Reset(_pCmdAllocator, nullptr)))
		{
			__debugbreak();
			return AK_FALSE;
		}

		if (_pDevice->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Buffer(uIndexBufferSize),
			D3D12_RESOURCE_STATE_COMMON,
			nullptr,
			IID_PPV_ARGS(&pUploadBuffer)))
		{
			__debugbreak();
			return AK_FALSE;
		}

		// Copy the triangle data to the vertex buffer.
		UINT8* pIndexDataBegin = nullptr;
		CD3DX12_RANGE tWriteRange(0, 0);        // We do not intend to read from this resource on the CPU.

		if (pUploadBuffer->Map(0, &tWriteRange, reinterpret_cast<void**>(&pIndexDataBegin)))
		{
			__debugbreak();
			return AK_FALSE;
		}

		memcpy(pIndexDataBegin, pInitData, uIndexBufferSize);
		pUploadBuffer->Unmap(0, nullptr);

		_pCmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(pIndexBuffer, D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_COPY_DEST));
		_pCmdList->CopyBufferRegion(pIndexBuffer, 0, pUploadBuffer, 0, uIndexBufferSize);
		_pCmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(pIndexBuffer, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_INDEX_BUFFER));

		_pCmdList->Close();

		ID3D12CommandList* ppCmdLists[] = { _pCmdList };
		_pCmdQueue->ExecuteCommandLists(_countof(ppCmdLists), ppCmdLists);

		Fence();
		WaitForFenceValue();
	}

	// Initialize the vertex buffer view.
	IndexBufferView.BufferLocation = pIndexBuffer->GetGPUVirtualAddress();
	IndexBufferView.Format = DXGI_FORMAT_R32_UINT;
	IndexBufferView.SizeInBytes = uIndexBufferSize;

	*pOutIndexBufferView = IndexBufferView;
	*ppOutBuffer = pIndexBuffer;

	if (pUploadBuffer)
	{
		pUploadBuffer->Release();
		pUploadBuffer = nullptr;
	}

	return AK_TRUE;
}

AkBool FResourceManager::CreateTextureFromFile(ID3D12Resource** ppTexResource, D3D12_RESOURCE_DESC* pResourceDesc, const wchar_t* wcFilename, AkBool bUseSRGB)
{
	ID3D12Resource* pTexResource = nullptr;

	ResourceUploadBatch tResourceUpload(_pDevice);

	tResourceUpload.Begin();

	if (FAILED(CreateDDSTextureFromFile(_pDevice, tResourceUpload, wcFilename, &pTexResource)))
	{
		__debugbreak();
		return AK_FALSE;
	}

	// Upload the resources to the GPU.
	auto uploadResourcesFinished = tResourceUpload.End(_pCmdQueue);

	// Wait for the upload thread to terminate
	uploadResourcesFinished.wait();

	*ppTexResource = pTexResource;
	*pResourceDesc = pTexResource->GetDesc();

	if (bUseSRGB)
	{
		DXGI_FORMAT tFormat = pTexResource->GetDesc().Format;
		if (DXGI_FORMAT_BC1_UNORM == tFormat)
		{
			pResourceDesc->Format = DXGI_FORMAT_BC1_UNORM_SRGB;
		}
		else if (DXGI_FORMAT_BC3_UNORM == tFormat)
		{
			pResourceDesc->Format = DXGI_FORMAT_BC3_UNORM_SRGB;
		}
		else
		{
			AkI32 a = 0;
		}
	}

	return AK_TRUE;
}

AkBool FResourceManager::CreateCubeMapTexture(ID3D12Resource** ppTexResource, D3D12_RESOURCE_DESC* pResourceDesc, const wchar_t* wcFilename)
{
	ID3D12Resource* pTexResource = nullptr;

	ResourceUploadBatch tResourceUpload(_pDevice);

	tResourceUpload.Begin();

	AkBool bIsCubeMap = AK_FALSE;
	if (FAILED(CreateDDSTextureFromFile(_pDevice, tResourceUpload, wcFilename, &pTexResource, false, 0, nullptr, (bool*)&bIsCubeMap)))
	{
		__debugbreak();
		return AK_FALSE;
	}

	// Upload the resources to the GPU.
	auto uploadResourcesFinished = tResourceUpload.End(_pCmdQueue);

	// Wait for the upload thread to terminate
	uploadResourcesFinished.wait();

	*ppTexResource = pTexResource;
	*pResourceDesc = pTexResource->GetDesc();

	return AK_TRUE;
}

AkBool FResourceManager::CreateTextureWithUploadBuffer(ID3D12Resource** ppTexResource, ID3D12Resource** ppUploadBuffer, AkU32 uWidth, AkU32 uHeight, DXGI_FORMAT tFormat)
{
	ID3D12Resource* pTexResource = nullptr;
	ID3D12Resource* pUploadBuffer = nullptr;

	D3D12_RESOURCE_DESC textureDesc = {};
	textureDesc.MipLevels = 1;
	textureDesc.Format = tFormat;	// ex) DXGI_FORMAT_R8G8B8A8_UNORM, etc...
	textureDesc.Width = uWidth;
	textureDesc.Height = uHeight;
	textureDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
	textureDesc.DepthOrArraySize = 1;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.SampleDesc.Quality = 0;
	textureDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;

	if (FAILED(_pDevice->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
		D3D12_HEAP_FLAG_NONE,
		&textureDesc,
		D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE,
		nullptr,
		IID_PPV_ARGS(&pTexResource))))
	{
		__debugbreak();
	}

	AkU64 uUploadBufferSize = GetRequiredIntermediateSize(pTexResource, 0, 1);

	if (FAILED(_pDevice->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(uUploadBufferSize),
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&pUploadBuffer))))
	{
		__debugbreak();
		return AK_FALSE;
	}

	*ppTexResource = pTexResource;
	*ppUploadBuffer = pUploadBuffer;

	return AK_TRUE;
}

void FResourceManager::CleanUp()
{
	DestroyFence();

	DestroyCmdList();

	DestroyCmdQueue();
}

AkBool FResourceManager::CreateCmdQueue()
{
	D3D12_COMMAND_QUEUE_DESC queueDesc = {};
	queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

	if (FAILED(_pDevice->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&_pCmdQueue))))
	{
		__debugbreak();
		return AK_FALSE;
	}

	return AK_TRUE;
}

AkBool FResourceManager::CreateFence()
{
	// Create synchronization objects and wait until assets have been uploaded to the GPU.
	if (FAILED(_pDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&_pFence))))
	{
		__debugbreak();
		return AK_FALSE;
	}

	// Create an event handle to use for frame synchronization.
	_hFenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);

	return AK_TRUE;
}

AkBool FResourceManager::CreateCmdList()
{
	if (FAILED(_pDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&_pCmdAllocator))))
	{
		__debugbreak();
		return AK_FALSE;
	}

	// Create the command list.
	if (FAILED(_pDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, _pCmdAllocator, nullptr, IID_PPV_ARGS(&_pCmdList))))
	{
		__debugbreak();
		return AK_FALSE;
	}

	// Command lists are created in the recording state, but there is nothing
	// to record yet. The main loop expects it to be closed, so close it now.
	_pCmdList->Close();

	return AK_TRUE;
}

void FResourceManager::DestroyCmdQueue()
{
	if (_pCmdQueue)
	{
		_pCmdQueue->Release();
		_pCmdQueue = nullptr;
	}
}

void FResourceManager::DestroyFence()
{
	if (_hFenceEvent)
	{
		::CloseHandle(_hFenceEvent);
		_hFenceEvent = nullptr;
	}
	if (_pFence)
	{
		_pFence->Release();
		_pFence = nullptr;
	}
}

void FResourceManager::DestroyCmdList()
{
	if (_pCmdList)
	{
		_pCmdList->Release();
		_pCmdList = nullptr;
	}
	if (_pCmdAllocator)
	{
		_pCmdAllocator->Release();
		_pCmdAllocator = nullptr;
	}
}

void FResourceManager::Fence()
{
	_u64FenceValue++;
	_pCmdQueue->Signal(_pFence, _u64FenceValue);
}

void FResourceManager::WaitForFenceValue()
{
	const AkU64 uExpectedFenceValue = _u64FenceValue;

	if (_pFence->GetCompletedValue() < _u64FenceValue)
	{
		_pFence->SetEventOnCompletion(_u64FenceValue, _hFenceEvent);
		::WaitForSingleObject(_hFenceEvent, INFINITE);
	}
}
