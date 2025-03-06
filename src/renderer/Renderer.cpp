#include "pch.h"
#include "Renderer.h"
#include "CommandListPool.h"
#include "DescriptorAllocator.h"
#include "DescriptorPool.h"
#include "ConstantBufferManager.h"
#include "BasicMeshObject.h"
#include "ResourceManager.h"
#include "TextureManager.h"
#include "FontManager.h"
#include "SpriteObject.h"
#include "D3DUtils.h"
#include "RenderQueue.h"
#include "RenderThread.h"
#include "SkinnedMeshObject.h"
#include "SkyboxObject.h"
#include "LineObject.h"
#include "PostProcess.h"

// For ImGui;
extern ImGuiContext* GImGui;

/*
=======================
Render core class
=======================
*/

FRenderer::FRenderer()
{
}

FRenderer::~FRenderer()
{
	CleanUp();
}

AkBool FRenderer::Initialize(HWND hWnd, AkBool bEnableDebugLayer, AkBool bEnableGBV)
{
	ID3D12Debug* pDebugController = nullptr;
	IDXGIFactory4* pFactory = nullptr;
	IDXGIAdapter1* pAdapter = nullptr;

	// Debug layer.
	AkU32 uDxgiFactoryFlags = 0;
	if (bEnableDebugLayer)
	{
		if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&pDebugController))))
		{
			pDebugController->EnableDebugLayer();

			// Enable additional debug layers.
			uDxgiFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;

			if (bEnableGBV)
			{
				ID3D12Debug5* pDebugController5 = nullptr;
				if (SUCCEEDED(pDebugController->QueryInterface(IID_PPV_ARGS(&pDebugController5))))
				{
					pDebugController5->SetEnableGPUBasedValidation(AK_TRUE);
					pDebugController5->SetEnableAutoName(AK_TRUE);
					pDebugController5->Release();
				}
			}
		}
	}

	if (FAILED(CreateDXGIFactory2(uDxgiFactoryFlags, IID_PPV_ARGS(&pFactory))))
	{
		__debugbreak();
		return AK_FALSE;
	}

	DXGI_ADAPTER_DESC1 tAdapterDesc = {};
	D3D_FEATURE_LEVEL arrFeatureLevels[] =
	{
		D3D_FEATURE_LEVEL_12_2,
		D3D_FEATURE_LEVEL_12_1,
		D3D_FEATURE_LEVEL_12_0,
		D3D_FEATURE_LEVEL_11_1,
		D3D_FEATURE_LEVEL_11_0
	};

	if (!CreateDevice(pFactory, &pAdapter))
	{
		__debugbreak();
		return AK_FALSE;
	}

	_hWnd = hWnd;
	_fDpi = static_cast<AkF32>(::GetDpiForWindow(_hWnd));

	if (!CreateCmdQueue())
	{
		__debugbreak();
		return AK_FALSE;
	}

	if (!CreateDescriptorForRTV())
	{
		__debugbreak();
		return AK_FALSE;
	}

	if (!CreateDescriptorForDSV())
	{
		__debugbreak();
		return AK_FALSE;
	}

	RECT tRect = {};
	::GetClientRect(_hWnd, &tRect);
	AkU32 uScreenWidth = tRect.right - tRect.left;
	AkU32 uScreenHeight = tRect.bottom - tRect.top;

	if (!CreateSwapChain(pFactory, uScreenWidth, uScreenHeight))
	{
		__debugbreak();
		return AK_FALSE;
	}

	_uScreenWidth = uScreenWidth;
	_uScreenHeight = uScreenHeight;

	InitViewports((AkF32)_uScreenWidth, (AkF32)_uScreenHeight);
	InitScissorRect(_uScreenWidth, _uScreenHeight);
	InitCamera();

	AkU32 uPhysicalCoreCount = 0;
	AkU32 uLogicalCoreCount = 0;
	GetPhysicalCoreCount(&uPhysicalCoreCount, &uLogicalCoreCount);
	_uRenderThreadCount = uPhysicalCoreCount;
	if (_uRenderThreadCount > MAX_RENDER_THREAD_COUNT)
	{
		_uRenderThreadCount = MAX_RENDER_THREAD_COUNT;
	}

#ifdef MULTI_THREAD_RENDERING
	CreateRenderThreadPool(_uRenderThreadCount);
#endif

	for (AkU32 i = 0; i < PENDING_FRAME_COUNT; i++)
	{
		for (AkU32 j = 0; j < _uRenderThreadCount; j++)
		{
			_ppCommandListPool[i][j] = new FCommandListPool;
			_ppCommandListPool[i][j]->Initialize(_pDevice, D3D12_COMMAND_LIST_TYPE_DIRECT, 256);

			_ppDescriptorPool[i][j] = new FDescriptorPool;
			_ppDescriptorPool[i][j]->Initialize(_pDevice, MAX_DRAW_COUNT_PER_FRAME * FBasicMeshObject::MAX_DESCRIPTOR_COUNT_FOR_DRAW);

			_ppConstantBufferManger[i][j] = new FConstantBufferManager;
			_ppConstantBufferManger[i][j]->Initialize(_pDevice, MAX_DRAW_COUNT_PER_FRAME);
		}
	}

	_pResourceManager = new FResourceManager;
	_pResourceManager->Initialize(_pDevice);

	_pDescriptorAllocator = new FDescriptorAllocator;
	_pDescriptorAllocator->Initialize(_pDevice, MAX_DESCRIPTOR_COUNT);

	_pTextureManager = new FTextureManager;
	_pTextureManager->Initialize(this, 256, 1024);

	_pFontManager = new FFontManager;
	_pFontManager->Initialize(this, _pCmdQueue, 1025, 256, bEnableDebugLayer);

	for (AkU32 i = 0; i < _uRenderThreadCount; i++)
	{
		_ppRenderQueue[i] = new FRenderQueue;
		_ppRenderQueue[i]->Initialize(this, 8192);
	}

	if (!CreateRTVs())
	{
		__debugbreak();
		return AK_FALSE;
	}

	if (!CreateRTVsAndSRVsForPBR())
	{
		__debugbreak();
		return AK_FALSE;
	}

	if (!CreateDSVs(uScreenWidth, uScreenHeight))
	{
		__debugbreak();
		return AK_FALSE;
	}

	if (!CreateShadowDSVs(_uShadowWidth, _uShadowHeight))
	{
		__debugbreak();
		return AK_FALSE;
	}

	if (!CreateFence())
	{
		__debugbreak();
		return AK_FALSE;
	}

	if (!CreatePostProcess())
	{
		__debugbreak();
		return AK_FALSE;
	}

	// TODO!!
	Vector3 vRadiance = Vector3(0.5f);
	Vector3 vLightDir = Vector3(-20.0f, 20.0f, 20.0f);
	AddGlobalLight(&vRadiance, &vLightDir, AK_TRUE);

	if (pAdapter)
	{
		pAdapter->Release();
		pAdapter = nullptr;
	}
	if (pFactory)
	{
		pFactory->Release();
		pFactory = nullptr;
	}
	if (pDebugController)
	{
		pDebugController->Release();
		pDebugController = nullptr;
	}

	return AK_TRUE;
}

// Multi thread rendering 적용.
void FRenderer::BeginCasterRenderPreparation()
{
	FCommandListPool* pCmdListPool = _ppCommandListPool[_uCurContextIndex][0];
	ID3D12GraphicsCommandList* pCmdList = pCmdListPool->GetCurrentCmdList();

	pCmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(_pShadowDS[_uCascadeIndex - 1], D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_DEPTH_WRITE));

	CD3DX12_CPU_DESCRIPTOR_HANDLE hDSVHeap(_pDSVHeap->GetCPUDescriptorHandleForHeapStart(), _uCascadeIndex, _uDSVDescriptorSize);
	pCmdList->ClearDepthStencilView(hDSVHeap, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

	pCmdList->RSSetViewports(1, &_tShadowViewport);
	pCmdList->RSSetScissorRects(1, &_tShadowScissorRect);

	pCmdList->OMSetRenderTargets(0, nullptr, AK_FALSE, &hDSVHeap);
}

void FRenderer::EndCasterRenderPreparation()
{
	FCommandListPool* pCmdListPool = _ppCommandListPool[_uCurContextIndex][0];
	ID3D12GraphicsCommandList* pCmdList = pCmdListPool->GetCurrentCmdList();

	pCmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(_pShadowDS[_uCascadeIndex - 1], D3D12_RESOURCE_STATE_DEPTH_WRITE, D3D12_RESOURCE_STATE_GENERIC_READ));

	pCmdListPool->CloseAndExecute(_pCmdQueue);

	Fence();

	_uCascadeIndex = (_uCascadeIndex + 1) % (CASCADE_SHADOW_MAP_LEVEL + 1);
	if (_uCascadeIndex == 0)
	{
		_uCascadeIndex = 1;
	}

	AkU32 uNextContextIndex = (_uCurContextIndex + 1) % PENDING_FRAME_COUNT;
	WaitForFenceValue(_u64FenceValue[uNextContextIndex]);

	for (AkU32 i = 0; i < _uRenderThreadCount; i++)
	{
		_ppConstantBufferManger[uNextContextIndex][i]->Reset();
		_ppDescriptorPool[uNextContextIndex][i]->Reset();
		_ppCommandListPool[uNextContextIndex][i]->Reset();
	}

	_uCurContextIndex = uNextContextIndex;
}

void FRenderer::BeginRender()
{
	FCommandListPool* pCmdListPool = _ppCommandListPool[_uCurContextIndex][0];
	ID3D12GraphicsCommandList* pCmdList = pCmdListPool->GetCurrentCmdList();

	CD3DX12_CPU_DESCRIPTOR_HANDLE hRTVHeap(_pRTVHeap->GetCPUDescriptorHandleForHeapStart(), SWAP_CHAIN_FRAME_COUNT, _uRTVDesciptorSize);
	CD3DX12_CPU_DESCRIPTOR_HANDLE hDSVHeap(_pDSVHeap->GetCPUDescriptorHandleForHeapStart());

	// Render taget 클리어
	const AkF32 fClearColor[] = { 0.0f, 0.0f, 1.0f, 1.0f };
	pCmdList->ClearRenderTargetView(hRTVHeap, fClearColor, 0, nullptr);
	pCmdList->ClearDepthStencilView(hDSVHeap, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

	pCmdListPool->CloseAndExecute(_pCmdQueue);

	Fence();
}

void FRenderer::EndRender()
{
	FCommandListPool* pCmdListPool = _ppCommandListPool[_uCurContextIndex][0];

	CD3DX12_CPU_DESCRIPTOR_HANDLE hResolvedRTVHeap(_pRTVHeap->GetCPUDescriptorHandleForHeapStart(), SWAP_CHAIN_FRAME_COUNT, _uRTVDesciptorSize);
	CD3DX12_CPU_DESCRIPTOR_HANDLE hDSVHeap(_pDSVHeap->GetCPUDescriptorHandleForHeapStart());

#ifdef MULTI_THREAD_RENDERING
	_uActiveThreadCount = _uRenderThreadCount;
	for (AkU32 i = 0; i < _uRenderThreadCount; i++)
	{
		::SetEvent(_pRenderThreadDescList[i].hEventList[(AkU32)RENDER_THREAD_EVENT_TYPE::RENDER_THREAD_EVENT_TYPE_PROCESS]);
	}
	::WaitForSingleObject(_hCompleteEvent, INFINITE);
#else
	for (AkU32 i = 0; i < _uRenderThreadCount; i++)
	{
		_ppRenderQueue[i]->Process(i, pCmdListPool, _pCmdQueue, 400, hResolvedRTVHeap, hDSVHeap, &_tViewport, &_tScissorRect);
	}
#endif

	ID3D12GraphicsCommandList* pCmdList = pCmdListPool->GetCurrentCmdList();
	CD3DX12_CPU_DESCRIPTOR_HANDLE hBackBufferRTVHeap(_pRTVHeap->GetCPUDescriptorHandleForHeapStart(), _uRTIndex, _uRTVDesciptorSize);

	D3D12_RESOURCE_BARRIER pBarriers0[] =
	{
		CD3DX12_RESOURCE_BARRIER::Transition(_ppBackBufferRT[_uRTIndex], D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET),
		CD3DX12_RESOURCE_BARRIER::Transition(_pResolvedBufferRT, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE),
	};

	pCmdList->ResourceBarrier(_countof(pBarriers0), pBarriers0);

	// Post Process
	_pPostProcess->ApplyPostProcess(0, pCmdList, hBackBufferRTVHeap, &_tViewport, &_tScissorRect);

	if (_bUseImgui)
	{
		pCmdList->SetDescriptorHeaps(1, &_pImGuiHeap);
		ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), pCmdList);
	}

	D3D12_RESOURCE_BARRIER pBarriers1[] =
	{
		CD3DX12_RESOURCE_BARRIER::Transition(_ppBackBufferRT[_uRTIndex], D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT),
		CD3DX12_RESOURCE_BARRIER::Transition(_pResolvedBufferRT, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET),
	};

	pCmdList->ResourceBarrier(_countof(pBarriers1), pBarriers1);

	pCmdListPool->CloseAndExecute(_pCmdQueue);

	for (AkU32 i = 0; i < _uRenderThreadCount; i++)
	{
		_ppRenderQueue[i]->Reset();
	}
}

void FRenderer::Present()
{
	Fence();

	AkU32 uPresentFlags = 0;

	if (!_uVSyncInterval)
	{
		uPresentFlags = DXGI_PRESENT_ALLOW_TEARING;
	}

	HRESULT hr = _pSwapChain->Present(_uVSyncInterval, uPresentFlags);

	if (DXGI_ERROR_DEVICE_REMOVED == hr)
	{
		__debugbreak();
	}

	_uRTIndex = _pSwapChain->GetCurrentBackBufferIndex();

	AkU32 uNextContextIndex = (_uCurContextIndex + 1) % PENDING_FRAME_COUNT;
	WaitForFenceValue(_u64FenceValue[uNextContextIndex]);

	for (AkU32 i = 0; i < _uRenderThreadCount; i++)
	{
		_ppConstantBufferManger[uNextContextIndex][i]->Reset();
		_ppDescriptorPool[uNextContextIndex][i]->Reset();
		_ppCommandListPool[uNextContextIndex][i]->Reset();
	}

	_uCurContextIndex = uNextContextIndex;
}

AkBool FRenderer::UpdateWindowSize(AkU32 uScreenWidth, AkU32 uScreenHeight)
{
	if (!(uScreenWidth * uScreenHeight))
	{
		return AK_FALSE;
	}
	if (_uScreenWidth == uScreenWidth && _uScreenHeight == uScreenHeight)
	{
		return AK_FALSE;
	}

	Fence();

	for (AkU32 i = 0; i < PENDING_FRAME_COUNT; i++)
	{
		WaitForFenceValue(_u64FenceValue[i]);
	}

	DXGI_SWAP_CHAIN_DESC1	tSwapChainDesc;
	if (FAILED(_pSwapChain->GetDesc1(&tSwapChainDesc)))
	{
		__debugbreak();
		return AK_FALSE;
	}

	for (AkU32 i = 0; i < SWAP_CHAIN_FRAME_COUNT; i++)
	{
		_ppBackBufferRT[i]->Release();
		_ppBackBufferRT[i] = nullptr;
	}
	if (_pResolvedBufferRT)
	{
		_pResolvedBufferRT->Release();
		_pResolvedBufferRT = nullptr;
	}
	if (_pMainDS)
	{
		_pMainDS->Release();
		_pMainDS = nullptr;
	}
	if (FAILED(_pSwapChain->ResizeBuffers(SWAP_CHAIN_FRAME_COUNT, uScreenWidth, uScreenHeight, DXGI_FORMAT_R8G8B8A8_UNORM, _uSwapChainFlag)))
	{
		__debugbreak();
		return AK_FALSE;
	}

	_uRTIndex = _pSwapChain->GetCurrentBackBufferIndex();

	CreateRTVs();
	CreateRTVsAndSRVsForPBR();
	CreateDSVs(uScreenWidth, uScreenHeight);

	InitViewports((AkF32)uScreenWidth, (AkF32)uScreenHeight);
	InitScissorRect(uScreenWidth, uScreenHeight);

	InitCamera();

	_uScreenWidth = uScreenWidth;
	_uScreenHeight = uScreenHeight;

	return AK_TRUE;
}

IMeshObject* FRenderer::CreateBasicMeshObject()
{
	FBasicMeshObject* pBasicMeshObj = new FBasicMeshObject;
	pBasicMeshObj->Initialize(this);

	return pBasicMeshObj;
}

IMeshObject* FRenderer::CreateSkinnedMeshObject()
{
	FSkinnedMeshObject* pSkinnedMeshObj = new FSkinnedMeshObject;
	pSkinnedMeshObj->Initialize(this);

	return pSkinnedMeshObj;
}

ISpriteObject* FRenderer::CreateSpriteObject()
{
	FSpriteObject* pSpriteObject = new FSpriteObject;
	pSpriteObject->Initialize(this);

	return pSpriteObject;
}

ISpriteObject* FRenderer::CreateSpriteObjectWidthTex(const wchar_t* wcTexFilename, AkI32 iPosX, AkI32 iPosY, AkI32 iWidth, AkI32 iHeight)
{
	FSpriteObject* pSpriteObject = new FSpriteObject;

	RECT tRect = {};
	tRect.left = iPosX;
	tRect.top = iPosY;
	tRect.right = iWidth;
	tRect.bottom = iHeight;

	pSpriteObject->Initialize(this, wcTexFilename, &tRect);

	return pSpriteObject;
}

ISkyboxObject* FRenderer::CreateSkyboxObject()
{
	FSkyboxObject* pSkyboxObj = new FSkyboxObject;
	pSkyboxObj->Initialize(this);

	return pSkyboxObj;
}

ILineObject* FRenderer::CreateLineObject()
{
	FLineObject* pLineObj = new FLineObject;
	pLineObj->Initialize(this);

	return pLineObj;
}

void* FRenderer::CreateTextureFromFile(const wchar_t* wcFilename, AkBool bUseSRGB)
{
	TextureHandle_t* pTexHandle = _pTextureManager->CreateTextureFromFile(wcFilename, bUseSRGB);
	if (!pTexHandle)
	{
		__debugbreak();
		return pTexHandle;
	}

	return pTexHandle;
}

void* FRenderer::CreateCubeMapTexture(const wchar_t* wcFilename)
{
	TextureHandle_t* pTexHandle = _pTextureManager->CreateCubeMapTexture(wcFilename);
	if (!pTexHandle)
	{
		__debugbreak();
		return pTexHandle;
	}

	return pTexHandle;
}

void* FRenderer::CreateDynamicTexture(AkU32 uTexWidth, AkU32 uTexHeight)
{
	TextureHandle_t* pTexHandle = _pTextureManager->CreateDynamicTexture(uTexWidth, uTexHeight);
	if (!pTexHandle)
	{
		__debugbreak();
		return pTexHandle;
	}

	return pTexHandle;
}

void* FRenderer::CreateFontObject(const wchar_t* wcFontFamilyName, AkF32 fFontSize)
{
	FontHandle_t* pFontHandle = _pFontManager->CreateFontObject(wcFontFamilyName, fFontSize);

	return pFontHandle;
}

AkBool FRenderer::WriteTextToBitmap(AkU8* pDestImage, AkU32 uDestWidth, AkU32 uDestHeight, AkU32 uDestPitch, AkI32* pWidth, AkI32* pHeight, void* pFontHandle, const wchar_t* wcText, AkU32 uTextLength, FONT_COLOR_TYPE eFontColor)
{
	AkBool bResult = _pFontManager->WriteTextToBitmap(pDestImage, uDestWidth, uDestHeight, uDestPitch, pWidth, pHeight, reinterpret_cast<FontHandle_t*>(pFontHandle), wcText, uTextLength, eFontColor);

	return bResult;
}

void FRenderer::UpdateTextureWidthImage(void* pTexHandle, const AkU8* pSrcImage, AkU32 uSrcWidth, AkU32 uSrcHeight)
{
	TextureHandle_t* pTextureHandle = (TextureHandle_t*)pTexHandle;
	ID3D12Resource* pDestTexResource = pTextureHandle->pTextureResource;
	ID3D12Resource* pUploadBuffer = pTextureHandle->pUploadBuffer;

	D3D12_RESOURCE_DESC tDesc = pDestTexResource->GetDesc();
	if (uSrcWidth > tDesc.Width)
	{
		__debugbreak();
	}
	if (uSrcHeight > tDesc.Height)
	{
		__debugbreak();
	}

	D3D12_PLACED_SUBRESOURCE_FOOTPRINT tFootprint;
	AkU32	uRows = 0;
	AkU64	u64RowSize = 0;
	AkU64	u64TotalBytes = 0;

	_pDevice->GetCopyableFootprints(&tDesc, 0, 1, 0, &tFootprint, &uRows, &u64RowSize, &u64TotalBytes);

	BYTE* pMappedPtr = nullptr;
	CD3DX12_RANGE writeRange(0, 0);

	HRESULT hr = pUploadBuffer->Map(0, &writeRange, reinterpret_cast<void**>(&pMappedPtr));
	if (FAILED(hr))
		__debugbreak();

	const AkU8* pSrc = pSrcImage;
	AkU8* pDest = pMappedPtr;
	for (AkU32 y = 0; y < uSrcHeight; y++)
	{
		memcpy(pDest, pSrc, uSrcWidth * 4);
		pSrc += (uSrcWidth * 4);
		pDest += tFootprint.Footprint.RowPitch;
	}
	// Unmap
	pUploadBuffer->Unmap(0, nullptr);

	pTextureHandle->bUpdated = TRUE;
}

void FRenderer::DestroyTexture(void* pTexHandle)
{
	for (AkU32 i = 0; i < PENDING_FRAME_COUNT; i++)
	{
		WaitForFenceValue(_u64FenceValue[i]);
	}

	_pTextureManager->DestroyTexture(reinterpret_cast<TextureHandle_t*>(pTexHandle));
}

void FRenderer::DestroyFontObject(void* pFontHandle)
{
	_pFontManager->DeleteFontObject(reinterpret_cast<FontHandle_t*>(pFontHandle));
}

void FRenderer::RenderBasicMeshObject(IMeshObject* pMeshObj, const Matrix* pWorldMat)
{
	RenderItem_t tItem = {};
	tItem.eItemType = RENDER_ITEM_TYPE::RENDER_ITEM_TYPE_MESH_OBJ;
	tItem.pObjHandle = pMeshObj;
	tItem.tMeshObjParam.pWorld = pWorldMat;

	if (!_ppRenderQueue[_uCurThreadIndex]->Add(&tItem))
	{
		__debugbreak();
	}

	_uCurThreadIndex++;
	_uCurThreadIndex = _uCurThreadIndex % _uRenderThreadCount;
}

void FRenderer::RenderNormalOfBasicMeshObject(IMeshObject* pMeshObj, const Matrix* pWorldMat)
{
	RenderItem_t tItem = {};
	tItem.eItemType = RENDER_ITEM_TYPE::RENDER_ITEM_TYPE_MESH_OBJ;
	tItem.pObjHandle = pMeshObj;
	tItem.tMeshObjParam.pWorld = pWorldMat;
	tItem.tMeshObjParam.bDrawNormal = AK_TRUE;

	if (!_ppRenderQueue[_uCurThreadIndex]->Add(&tItem))
	{
		__debugbreak();
	}

	_uCurThreadIndex++;
	_uCurThreadIndex = _uCurThreadIndex % _uRenderThreadCount;
}

void FRenderer::RenderShadowOfBasicMeshObject(IMeshObject* pMeshObj, const Matrix* pWorldMat)
{
	FCommandListPool* pCmdListPool = _ppCommandListPool[_uCurContextIndex][0];
	ID3D12GraphicsCommandList* pCmdList = pCmdListPool->GetCurrentCmdList();

	FBasicMeshObject* pBasicMeshObj = reinterpret_cast<FBasicMeshObject*>(pMeshObj);
	pBasicMeshObj->DrawShadow(pCmdList, pWorldMat);
}

void FRenderer::RenderSkinnedMeshObject(IMeshObject* pMeshObj, const Matrix* pWorldMat, const Matrix* pBonesTransform)
{
	RenderItem_t tItem = {};
	tItem.eItemType = RENDER_ITEM_TYPE::RENDER_ITEM_TYPE_SKINNED_MESH_OBJ;
	tItem.pObjHandle = pMeshObj;
	tItem.tSkinnedMeshObjParam.pWorld = pWorldMat;
	tItem.tSkinnedMeshObjParam.pBonesTransform = pBonesTransform;

	if (!_ppRenderQueue[_uCurThreadIndex]->Add(&tItem))
	{
		__debugbreak();
	}

	_uCurThreadIndex++;
	_uCurThreadIndex = _uCurThreadIndex % _uRenderThreadCount;
}

void FRenderer::RenderNormalOfSkinnedMeshObject(IMeshObject* pMeshObj, const Matrix* pWorldMat, const Matrix* pBonesTransform)
{
	RenderItem_t tItem = {};
	tItem.eItemType = RENDER_ITEM_TYPE::RENDER_ITEM_TYPE_SKINNED_MESH_OBJ;
	tItem.pObjHandle = pMeshObj;
	tItem.tSkinnedMeshObjParam.pWorld = pWorldMat;
	tItem.tSkinnedMeshObjParam.pBonesTransform = pBonesTransform;
	tItem.tSkinnedMeshObjParam.bDrawNormal = AK_TRUE;

	if (!_ppRenderQueue[_uCurThreadIndex]->Add(&tItem))
	{
		__debugbreak();
	}

	_uCurThreadIndex++;
	_uCurThreadIndex = _uCurThreadIndex % _uRenderThreadCount;
}

void FRenderer::RenderShadowOfSkinnedMeshObject(IMeshObject* pMeshObj, const Matrix* pWorldMat, const Matrix* pBonesTransform)
{
	FCommandListPool* pCmdListPool = _ppCommandListPool[_uCurContextIndex][0];
	ID3D12GraphicsCommandList* pCmdList = pCmdListPool->GetCurrentCmdList();

	FSkinnedMeshObject* pSkinnedMeshObj = reinterpret_cast<FSkinnedMeshObject*>(pMeshObj);
	pSkinnedMeshObj->DrawShadow(pCmdList, pWorldMat, pBonesTransform);
}

void FRenderer::RenderSpriteWithTex(void* pSpriteObjHandle, AkI32 iPosX, AkI32 iPosY, AkF32 fScaleX, AkF32 fScaleY, const RECT* pRect, AkF32 fZ, void* pTexHandle, const Vector3* pFontColor)
{
	RenderItem_t tItem = {};
	tItem.eItemType = RENDER_ITEM_TYPE::RENDER_ITEM_TYPE_SPRITE_OBJ;
	tItem.pObjHandle = pSpriteObjHandle;
	tItem.tSpriteObjParam.iPosX = iPosX;
	tItem.tSpriteObjParam.iPosY = iPosY;
	tItem.tSpriteObjParam.fScaleX = fScaleX;
	tItem.tSpriteObjParam.fScaleY = fScaleY;
	tItem.tSpriteObjParam.pFontColor = pFontColor;

	if (pRect)
	{
		tItem.tSpriteObjParam.bUseRect = AK_TRUE;
		tItem.tSpriteObjParam.tRect = *pRect;
	}
	else
	{
		tItem.tSpriteObjParam.bUseRect = AK_FALSE;
		tItem.tSpriteObjParam.tRect = {};
	}

	tItem.tSpriteObjParam.pTexHandle = pTexHandle;
	tItem.tSpriteObjParam.fZ = fZ;

	if (!_ppRenderQueue[_uCurThreadIndex]->Add(&tItem))
	{
		__debugbreak();
	}

	_uCurThreadIndex++;
	_uCurThreadIndex = _uCurThreadIndex % _uRenderThreadCount;
}

void FRenderer::RenderSprite(void* pSpriteObjHandle, AkI32 iPosX, AkI32 iPosY, AkF32 fScaleX, AkF32 fScaleY, AkF32 fZ)
{
	RenderItem_t tItem = {};
	tItem.eItemType = RENDER_ITEM_TYPE::RENDER_ITEM_TYPE_SPRITE_OBJ;
	tItem.pObjHandle = pSpriteObjHandle;
	tItem.tSpriteObjParam.iPosX = iPosX;
	tItem.tSpriteObjParam.iPosY = iPosY;
	tItem.tSpriteObjParam.fScaleX = fScaleX;
	tItem.tSpriteObjParam.fScaleY = fScaleY;
	tItem.tSpriteObjParam.bUseRect = AK_FALSE;
	tItem.tSpriteObjParam.tRect = {};
	tItem.tSpriteObjParam.pTexHandle = nullptr;
	tItem.tSpriteObjParam.fZ = fZ;

	if (!_ppRenderQueue[_uCurThreadIndex]->Add(&tItem))
	{
		__debugbreak();
	}

	_uCurThreadIndex++;
	_uCurThreadIndex = _uCurThreadIndex % _uRenderThreadCount;
}

void FRenderer::RenderSkybox(ISkyboxObject* pSkyboxObj, const Matrix* pWorldMat, void* pEnvHDR, void* pDiffuseHDR, void* pSpecularHDR)
{
	RenderItem_t tItem = {};
	tItem.eItemType = RENDER_ITEM_TYPE::RENDER_ITEM_TYPE_SKYBOX_OBJ;
	tItem.pObjHandle = pSkyboxObj;
	tItem.tSkyboxObjParam.pWorld = pWorldMat;
	tItem.tSkyboxObjParam.pEnvHDR = pEnvHDR;
	tItem.tSkyboxObjParam.pDiffuseHDR = pDiffuseHDR;
	tItem.tSkyboxObjParam.pSpecularHDR = pSpecularHDR;

	if (!_ppRenderQueue[_uCurThreadIndex]->Add(&tItem))
	{
		__debugbreak();
	}

	_uCurThreadIndex++;
	_uCurThreadIndex = _uCurThreadIndex % _uRenderThreadCount;
}

void FRenderer::RenderLineObject(ILineObject* pLineObj, const Matrix* pWorldMat)
{
	RenderItem_t tItem = {};
	tItem.eItemType = RENDER_ITEM_TYPE::RENDER_ITEM_TYPE_LINE_OBJ;
	tItem.pObjHandle = pLineObj;
	tItem.tLineObjParam._pWorld = pWorldMat;

	if (!_ppRenderQueue[_uCurThreadIndex]->Add(&tItem))
	{
		__debugbreak();
	}

	_uCurThreadIndex++;
	_uCurThreadIndex = _uCurThreadIndex % _uRenderThreadCount;
}

void FRenderer::SetCameraPosition(AkF32 fX, AkF32 fY, AkF32 fZ)
{
	_vCamPos.x = fX;
	_vCamPos.y = fY;
	_vCamPos.z = fZ;
	Vector3 vCamUp = Vector3(0.0f, 1.0f, 0.0f);

	SetCamera(&_vCamPos, &_vCamDir, &vCamUp);
}

void FRenderer::RotateXCamera(AkF32 fRadian)
{
	Vector3 vCamDir = Vector3(0.0f, 0.0f, 1.0f);
	vCamDir = Vector3::Transform(vCamDir, Matrix::CreateRotationX(fRadian));
	_vCamDir = vCamDir;
	Vector3 vCamUp = Vector3(0.0f, 1.0f, 0.0f);

	SetCamera(&_vCamPos, &_vCamDir, &vCamUp);
}

void FRenderer::RotateYCamera(AkF32 fRadian)
{
	Vector3 vCamDir = Vector3(0.0f, 0.0f, 1.0f);
	vCamDir = Vector3::Transform(vCamDir, Matrix::CreateRotationY(fRadian));
	_vCamDir = vCamDir;
	Vector3 vCamUp = Vector3(0.0f, 1.0f, 0.0f);

	SetCamera(&_vCamPos, &_vCamDir, &vCamUp);
}

void FRenderer::RotateYawPitchRollCamera(AkF32 fYaw, AkF32 fPitch, AkF32 fRoll)
{
	Vector3 vCamDir = Vector3(0.0f, 0.0f, 1.0f);
	vCamDir = Vector3::Transform(vCamDir, Matrix::CreateFromYawPitchRoll(fYaw, fPitch, fRoll));
	_vCamDir = vCamDir;
	Vector3 vCamUp = Vector3(0.0f, 1.0f, 0.0f);

	SetCamera(&_vCamPos, &_vCamDir, &vCamUp);
}

void FRenderer::MoveCamera(AkF32 fX, AkF32 fY, AkF32 fZ)
{
	_vCamPos.x += fX;
	_vCamPos.y += fY;
	_vCamPos.z += fZ;
	Vector3 vCamUp = Vector3(0.0f, 1.0f, 0.0f);

	SetCamera(&_vCamPos, &_vCamDir, &vCamUp);
}

void FRenderer::GetCameraPosition(AkF32* pX, AkF32* pY, AkF32* pZ)
{
	*pX = _vCamPos.x;
	*pY = _vCamPos.y;
	*pZ = _vCamPos.z;
}

void FRenderer::SetCamera(const Vector3* pCamPos, const Vector3* pCamDir, Vector3* pCamUp)
{
	// 업벡터와 카메라 방향벡터가 일치할때 예외처리
	if (abs((*pCamUp).Dot(*pCamDir) - 1.0f) <= AK_F32_EPSILON)
	{
		*pCamUp = Vector3(0.0f, 0.0f, -1.0f);
	}
	else if (abs((*pCamUp).Dot(*pCamDir) + 1.0f) <= AK_F32_EPSILON)
	{
		*pCamUp = Vector3(0.0f, 0.0f, 1.0f);
	}

	_mViewMat = XMMatrixLookToLH(*pCamPos, *pCamDir, *pCamUp);

	const AkF32 fAspectRatio = static_cast<AkF32>(_uScreenWidth) / static_cast<AkF32>(_uScreenHeight);
	_fFov = XM_PIDIV4;
	_fNear = 0.1f;
	_fFar = 1000.0f;
	_mProjMat = XMMatrixPerspectiveFovLH(_fFov, fAspectRatio, _fNear, _fFar);
}

void FRenderer::AddGlobalLight(const Vector3* pRadiance, const Vector3* pDir, AkBool bShadow)
{
	_tGlobalLight.vRadiance = *pRadiance;
	_tGlobalLight.vDirection = *pDir;
	_tGlobalLight.vDirection.Normalize();
	_tGlobalLight.uType = LIGHT_DIRECTIONAL;

	if (bShadow)
	{
		_tGlobalLight.uType |= LIGHT_SHADOW;
	}
}

void FRenderer::AddPointLight(const Vector3* pPos, const Vector3* pDir, AkF32 fRadius, AkF32 fFallOffStart, AkF32 fFallOffEnd, AkF32 fSpotPower, AkBool bShadow)
{
	if (_uPointLightsNum >= POINT_LIGHTS_NUM)
	{
		__debugbreak();
	}

	_pPointLights[_uPointLightsNum].vRadiance = Vector3(5.0f);
	_pPointLights[_uPointLightsNum].vPosition = *pPos;
	_pPointLights[_uPointLightsNum].vDirection = *pDir;
	_pPointLights[_uPointLightsNum].fSpotPower = fSpotPower;
	_pPointLights[_uPointLightsNum].fRadius = fRadius;
	_pPointLights[_uPointLightsNum].uType = LIGHT_POINT;
	_pPointLights[_uPointLightsNum].fFallOffStart = fFallOffStart;
	_pPointLights[_uPointLightsNum].fFallOffEnd = fFallOffEnd;

	if (bShadow)
	{
		_pPointLights[_uPointLightsNum].uType |= LIGHT_SHADOW;
	}

	_uPointLightsNum++;
}

void FRenderer::SetIBLStrength(AkF32 fIBLStrength)
{
	_fIBLStrength = fIBLStrength;
}

AkBool FRenderer::MousePickingToPlane(DirectX::SimpleMath::Plane* pPlane, AkF32 fNdcX, AkF32 fNdcY, Vector3* pHitPos, AkF32* pHitDist, AkF32* pRatio)
{
	DirectX::SimpleMath::Ray tRay = {};
	AkF32 fRayLength = 0.0f;
	CalculateMousePickingRayCast(fNdcX, fNdcY, &tRay, &fRayLength);

	AkF32 fHitDist = 0.0f;
	if (tRay.Intersects(*pPlane, fHitDist))
	{
		*pHitPos = tRay.position + fHitDist * tRay.direction;
		*pHitDist = fHitDist;
		*pRatio = fHitDist / fRayLength;

		return AK_TRUE;
	}

	return AK_FALSE;
}

AkBool FRenderer::MousePickingToSphere(DirectX::BoundingSphere* pSphere, AkF32 fNdcX, AkF32 fNdcY, Vector3* pHitPos, AkF32* pHitDist, AkF32* pRatio)
{
	DirectX::SimpleMath::Ray tRay = {};
	AkF32 fRayLength = 0.0f;
	CalculateMousePickingRayCast(fNdcX, fNdcY, &tRay, &fRayLength);

	AkF32 fHitDist = 0.0f;
	if (tRay.Intersects(*pSphere, fHitDist))
	{
		*pHitPos = tRay.position + fHitDist * tRay.direction;
		*pHitDist = fHitDist;
		*pRatio = fHitDist / fRayLength;

		return AK_TRUE;
	}

	return AK_FALSE;
}

AkBool FRenderer::MousePickingToBox(DirectX::BoundingBox* pBox, AkF32 fNdcX, AkF32 fNdcY, Vector3* pHitPos, AkF32* pHitDist, AkF32* pRatio)
{
	DirectX::SimpleMath::Ray tRay = {};
	AkF32 fRayLength = 0.0f;
	CalculateMousePickingRayCast(fNdcX, fNdcY, &tRay, &fRayLength);

	AkF32 fHitDist = 0.0f;
	if (tRay.Intersects(*pBox, fHitDist))
	{
		*pHitPos = tRay.position + fHitDist * tRay.direction;
		*pHitDist = fHitDist;
		*pRatio = fHitDist / fRayLength;

		return AK_TRUE;
	}

	return AK_FALSE;
}

void FRenderer::GetFrustum(AkPlane_t* ppOutPlane)
{
	AkPlane_t tPlane[6] = {};

	Matrix mViewProjRow = _mViewMat * _mProjMat;

	AkF32 fX = (AkF32)(mViewProjRow._14 + mViewProjRow._13);
	AkF32 fY = (AkF32)(mViewProjRow._24 + mViewProjRow._23);
	AkF32 fZ = (AkF32)(mViewProjRow._34 + mViewProjRow._33);
	AkF32 fW = (AkF32)(mViewProjRow._44 + mViewProjRow._43);
	tPlane[0].vPlane = DirectX::XMPlaneNormalize(Vector4(fX, fY, fZ, fW));

	fX = (AkF32)(mViewProjRow._14 - mViewProjRow._13);
	fY = (AkF32)(mViewProjRow._24 - mViewProjRow._23);
	fZ = (AkF32)(mViewProjRow._34 - mViewProjRow._33);
	fW = (AkF32)(mViewProjRow._44 - mViewProjRow._43);
	tPlane[1].vPlane = DirectX::XMPlaneNormalize(Vector4(fX, fY, fZ, fW));

	fX = (AkF32)(mViewProjRow._14 + mViewProjRow._11);
	fY = (AkF32)(mViewProjRow._24 + mViewProjRow._21);
	fZ = (AkF32)(mViewProjRow._34 + mViewProjRow._31);
	fW = (AkF32)(mViewProjRow._44 + mViewProjRow._41);
	tPlane[2].vPlane = DirectX::XMPlaneNormalize(Vector4(fX, fY, fZ, fW));

	fX = (AkF32)(mViewProjRow._14 - mViewProjRow._11);
	fY = (AkF32)(mViewProjRow._24 - mViewProjRow._21);
	fZ = (AkF32)(mViewProjRow._34 - mViewProjRow._31);
	fW = (AkF32)(mViewProjRow._44 - mViewProjRow._41);
	tPlane[3].vPlane = DirectX::XMPlaneNormalize(Vector4(fX, fY, fZ, fW));

	fX = (AkF32)(mViewProjRow._14 - mViewProjRow._12);
	fY = (AkF32)(mViewProjRow._24 - mViewProjRow._22);
	fZ = (AkF32)(mViewProjRow._34 - mViewProjRow._32);
	fW = (AkF32)(mViewProjRow._44 - mViewProjRow._42);
	tPlane[4].vPlane = DirectX::XMPlaneNormalize(Vector4(fX, fY, fZ, fW));

	fX = (AkF32)(mViewProjRow._14 + mViewProjRow._12);
	fY = (AkF32)(mViewProjRow._24 + mViewProjRow._22);
	fZ = (AkF32)(mViewProjRow._34 + mViewProjRow._32);
	fW = (AkF32)(mViewProjRow._44 + mViewProjRow._42);
	tPlane[5].vPlane = DirectX::XMPlaneNormalize(Vector4(fX, fY, fZ, fW));

	ppOutPlane[0] = tPlane[0];
	ppOutPlane[1] = tPlane[1];
	ppOutPlane[2] = tPlane[2];
	ppOutPlane[3] = tPlane[3];
	ppOutPlane[4] = tPlane[4];
	ppOutPlane[5] = tPlane[5];
}

void FRenderer::SetVSync(AkBool bUseVSync)
{
	AkU32 uVSyncInterval = 0;

	if (bUseVSync)
	{
		uVSyncInterval = 1;
	}
	else
	{
		uVSyncInterval = 0;
	}

	_uVSyncInterval = uVSyncInterval;
}

HRESULT __stdcall FRenderer::QueryInterface(REFIID riid, void** ppvObject)
{
	return E_NOTIMPL;
}

ULONG __stdcall FRenderer::AddRef(void)
{
	AkU32 uRefCount = ++_uRefCount;
	return uRefCount;
}

ULONG __stdcall FRenderer::Release(void)
{
	AkU32 uRefCount = --_uRefCount;
	if (!uRefCount)
	{
		delete this;
	}
	return uRefCount;
}

FConstantBufferPool* FRenderer::GetConstantBufferPool(AkU32 uThreadIndex, CONSTANT_BUFFER_TYPE eConstBufType)
{
	FConstantBufferManager* pConstantBufferManager = _ppConstantBufferManger[_uCurContextIndex][uThreadIndex];
	FConstantBufferPool* pConstantBufferPool = pConstantBufferManager->GetConstantBufferPool(eConstBufType);

	return pConstantBufferPool;
}

void FRenderer::GetViewPorjMatrix(Matrix* pViewMat, Matrix* pProjMat)
{
	*pViewMat = _mViewMat.Transpose();
	*pProjMat = _mProjMat.Transpose();
}

void FRenderer::GetShadowViewProjMatrix(Matrix* pViewMat, Matrix* pProjMat, AkU32 uCascadeIndex)
{
	*pViewMat = _pShadowView[uCascadeIndex];
	*pProjMat = _pShadowOrthoProj[uCascadeIndex];
}

void FRenderer::GetLightPosition(AkF32* fX, AkF32* fY, AkF32* fZ)
{
	*fX = _vLightPos.x;
	*fY = _vLightPos.y;
	*fZ = _vLightPos.z;
}

void FRenderer::GetIBLTexture(TextureHandle_t** ppOutIrradianceTexHandle, TextureHandle_t** ppOutSpecularTexHandle, TextureHandle_t** ppOutBrdfTexHandle)
{
	*ppOutIrradianceTexHandle = _pIrradianceIBLTexHandle;

	*ppOutSpecularTexHandle = _pSpecularIBLTexHandle;

	*ppOutBrdfTexHandle = _pBrdfTexHandle;
}

Light_t* FRenderer::GetLights(AkU32* pOutLightNum)
{
	*pOutLightNum = _uPointLightsNum;

	return _pPointLights;
}

void FRenderer::GetGlobalLight(Light_t* pOutLight)
{
	*pOutLight = _tGlobalLight;
}

void FRenderer::GetShadowMapSrv(D3D12_CPU_DESCRIPTOR_HANDLE* pOutHandle, AkU32 uCascadeIndex)
{
	*pOutHandle = _pShadowMapSrvCpu[uCascadeIndex];
}

void FRenderer::BindIBLTexture(void* pIrradianceTexHandle, void* pSpecularTexHandle, void* pBrdfTexHandle)
{
	_pIrradianceIBLTexHandle = reinterpret_cast<TextureHandle_t*>(pIrradianceTexHandle);

	_pSpecularIBLTexHandle = reinterpret_cast<TextureHandle_t*>(pSpecularTexHandle);

	_pBrdfTexHandle = reinterpret_cast<TextureHandle_t*>(pBrdfTexHandle);
}

void FRenderer::BindImGui(void** ppImGuiCtx)
{
	_bUseImgui = AK_TRUE;

	CreateImGuiInitResource();

	ImGuiContext** ppImGuiContext = (ImGuiContext**)ppImGuiCtx;
	*ppImGuiContext = GImGui;
}

void FRenderer::UnBindImGui()
{
	_bUseImgui = AK_FALSE;

	DestroyImGuiInitResource();
}

void FRenderer::EnsureCompleted()
{
	for (AkU32 i = 0; i < PENDING_FRAME_COUNT; i++)
	{
		WaitForFenceValue(_u64FenceValue[i]);
	}
}

void FRenderer::CleanUp()
{
	// Full screen mode 해제
	if (_pSwapChain)
	{
		_pSwapChain->SetFullscreenState(AK_FALSE, nullptr);
	}

	Fence();

	for (AkU32 i = 0; i < PENDING_FRAME_COUNT; i++)
	{
		WaitForFenceValue(_u64FenceValue[i]);
	}

	for (AkU32 i = 0; i < _uRenderThreadCount; i++)
	{
		delete _ppRenderQueue[i];
		_ppRenderQueue[i] = nullptr;
	}
	if (_pFontManager)
	{
		delete _pFontManager;
		_pFontManager = nullptr;
	}
	if (_pTextureManager)
	{
		delete _pTextureManager;
		_pTextureManager = nullptr;
	}
	if (_pResourceManager)
	{
		delete _pResourceManager;
		_pResourceManager = nullptr;
	}
	if (_pDescriptorAllocator)
	{
		delete _pDescriptorAllocator;
		_pDescriptorAllocator = nullptr;
	}
	for (AkU32 i = 0; i < PENDING_FRAME_COUNT; i++)
	{
		for (AkU32 j = 0; j < _uRenderThreadCount; j++)
		{
			if (_ppConstantBufferManger[i][j])
			{
				delete _ppConstantBufferManger[i][j];
				_ppConstantBufferManger[i][j] = nullptr;
			}
			if (_ppDescriptorPool)
			{
				delete _ppDescriptorPool[i][j];
				_ppDescriptorPool[i][j] = nullptr;
			}
			if (_ppCommandListPool)
			{
				delete _ppCommandListPool[i][j];
				_ppCommandListPool[i][j] = nullptr;
			}
		}
	}

	DestroyPostProcess();

	DestroyRenderThreadPool(_uRenderThreadCount);

	DestroyFence();

	DestroyShadowDSVs();

	DestroyDSVs();

	DestroyRTVsAndSRVsForPBR();

	DestroyRTVs();

	DestroySwapChain();

	DestroyDescriptorForDSV();

	DestroyDescriptorForRTV();

	DestroyCmdQueue();

	DestroyDevice();
}

void FRenderer::Fence()
{
	_u64CommonFenceVaule++;
	_pCmdQueue->Signal(_pFence, _u64CommonFenceVaule);
	_u64FenceValue[_uCurContextIndex] = _u64CommonFenceVaule;
}

void FRenderer::WaitForFenceValue(AkU64 u64ExpectedFenceValue)
{
	if (_pFence->GetCompletedValue() < u64ExpectedFenceValue)
	{
		_pFence->SetEventOnCompletion(u64ExpectedFenceValue, _hFenceEvent);
		::WaitForSingleObject(_hFenceEvent, INFINITE);
	}
}

Vector3 FRenderer::GetWorldNearPosition(AkF32 fNdcX, AkF32 fNdcY)
{
	Vector3 vNdcNearPos = Vector3(fNdcX, fNdcY, 0.0f);
	Vector3 vNdcFarPos = Vector3(fNdcX, fNdcY, 1.0f);

	Matrix mInvViewProj = (_mViewMat * _mProjMat).Invert();

	Vector3 vWorldNearPos = Vector3::Transform(vNdcNearPos, mInvViewProj);

	return vWorldNearPos;
}

Vector3 FRenderer::GetWorldFarPosition(AkF32 fNdcX, AkF32 fNdcY)
{
	Vector3 vNdcNearPos = Vector3(fNdcX, fNdcY, 0.0f);
	Vector3 vNdcFarPos = Vector3(fNdcX, fNdcY, 1.0f);

	Matrix mInvViewProj = (_mViewMat * _mProjMat).Invert();

	Vector3 vWorldFarPos = Vector3::Transform(vNdcFarPos, mInvViewProj);

	return vWorldFarPos;
}

void FRenderer::CalculateMousePickingRayCast(AkF32 fNdcX, AkF32 fNdcY, DirectX::SimpleMath::Ray* pRay, AkF32* pRayLength)
{
	Vector3 vWorldNearPos = GetWorldNearPosition(fNdcX, fNdcY);
	Vector3 vWorldFarPos = GetWorldFarPosition(fNdcX, fNdcY);

	Vector3 vRayDir = vWorldFarPos - vWorldNearPos;
	AkF32 fRayLength = vRayDir.Length();
	vRayDir.Normalize();

	DirectX::SimpleMath::Ray tRay(vWorldNearPos, vRayDir);

	*pRay = tRay;
	*pRayLength = fRayLength;
}

void FRenderer::ProcessByThread(AkU32 uThreadIndex)
{
	FCommandListPool* pCmdListPool = _ppCommandListPool[_uCurContextIndex][uThreadIndex];

	CD3DX12_CPU_DESCRIPTOR_HANDLE hRTVHeap(_pRTVHeap->GetCPUDescriptorHandleForHeapStart(), SWAP_CHAIN_FRAME_COUNT, _uRTVDesciptorSize); // Float RTV.
	CD3DX12_CPU_DESCRIPTOR_HANDLE hDSVHeap(_pDSVHeap->GetCPUDescriptorHandleForHeapStart());

	_ppRenderQueue[uThreadIndex]->Process(uThreadIndex, pCmdListPool, _pCmdQueue, 400, hRTVHeap, hDSVHeap, &_tViewport, &_tScissorRect);

	LONG lCurCount = _InterlockedDecrement(&_uActiveThreadCount);
	if (0 == lCurCount)
	{
		::SetEvent(_hCompleteEvent);
	}
}

AkBool FRenderer::CreateDevice(IDXGIFactory4* pFactory, IDXGIAdapter1** ppAdapter)
{
	DXGI_ADAPTER_DESC1 tAdapterDesc = {};
	D3D_FEATURE_LEVEL arrFeatureLevels[] =
	{
		D3D_FEATURE_LEVEL_12_2,
		D3D_FEATURE_LEVEL_12_1,
		D3D_FEATURE_LEVEL_12_0,
		D3D_FEATURE_LEVEL_11_1,
		D3D_FEATURE_LEVEL_11_0
	};

	AkBool bComplete = AK_FALSE;
	AkU32 uNumFeatureLevels = _countof(arrFeatureLevels);
	for (AkU32 i = 0; i < uNumFeatureLevels; i++)
	{
		AkU32 uAdapterIdx = 0;
		while (DXGI_ERROR_NOT_FOUND != pFactory->EnumAdapters1(uAdapterIdx, ppAdapter))
		{
			(*ppAdapter)->GetDesc1(&tAdapterDesc);
			if (SUCCEEDED(D3D12CreateDevice(*ppAdapter, arrFeatureLevels[i], IID_PPV_ARGS(&_pDevice))))
			{
				bComplete = AK_TRUE;
				break;
			}
			(*ppAdapter)->Release();
			(*ppAdapter) = nullptr;
			uAdapterIdx++;
		}

		if (bComplete)
		{
			break;
		}
	}

	if (!_pDevice)
	{
		__debugbreak();
		return AK_FALSE;
	}

	_tAdapterDesc = tAdapterDesc;

	return AK_TRUE;
}

AkBool FRenderer::CreateCmdQueue()
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

AkBool FRenderer::CreateDescriptorForRTV()
{
	D3D12_DESCRIPTOR_HEAP_DESC tRtvHeapDesc = {};
	tRtvHeapDesc.NumDescriptors = SWAP_CHAIN_FRAME_COUNT + 1; // Resolved Rtv.
	tRtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	tRtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	if (FAILED(_pDevice->CreateDescriptorHeap(&tRtvHeapDesc, IID_PPV_ARGS(&_pRTVHeap))))
	{
		__debugbreak();
		return AK_FALSE;
	}

	_uRTVDesciptorSize = _pDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

	return AK_TRUE;
}

AkBool FRenderer::CreateDescriptorForDSV()
{
	D3D12_DESCRIPTOR_HEAP_DESC tDsvHeapDesc = {};
	tDsvHeapDesc.NumDescriptors = 1 + CASCADE_SHADOW_MAP_LEVEL; // Main + Shadow Map.
	tDsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	tDsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	if (FAILED(_pDevice->CreateDescriptorHeap(&tDsvHeapDesc, IID_PPV_ARGS(&_pDSVHeap))))
	{
		__debugbreak();
		return AK_FALSE;
	}

	_uDSVDescriptorSize = _pDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);

	return AK_TRUE;
}

AkBool FRenderer::CreateSwapChain(IDXGIFactory4* pFactory, AkU32 uScreenWidth, AkU32 uScreenHeight)
{
	DXGI_SWAP_CHAIN_DESC1 tSwapChainDesc = {};
	tSwapChainDesc.Width = uScreenWidth;
	tSwapChainDesc.Height = uScreenHeight;
	tSwapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	tSwapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	tSwapChainDesc.BufferCount = SWAP_CHAIN_FRAME_COUNT;
	tSwapChainDesc.SampleDesc.Count = 1;
	tSwapChainDesc.SampleDesc.Quality = 0;
	tSwapChainDesc.Scaling = DXGI_SCALING_NONE;
	tSwapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	tSwapChainDesc.AlphaMode = DXGI_ALPHA_MODE_IGNORE;
	tSwapChainDesc.Flags |= DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING;
	_uSwapChainFlag = tSwapChainDesc.Flags;

	DXGI_SWAP_CHAIN_FULLSCREEN_DESC tFullScreenSwapChainDesc = {};
	tFullScreenSwapChainDesc.Windowed = AK_TRUE;

	IDXGISwapChain1* pSwapChain1 = nullptr;
	if (FAILED(pFactory->CreateSwapChainForHwnd(_pCmdQueue, _hWnd, &tSwapChainDesc, &tFullScreenSwapChainDesc, nullptr, &pSwapChain1)))
	{
		__debugbreak();
		return AK_FALSE;
	}

	pSwapChain1->QueryInterface(IID_PPV_ARGS(&_pSwapChain));
	pSwapChain1->Release();
	pSwapChain1 = nullptr;
	_uRTIndex = _pSwapChain->GetCurrentBackBufferIndex();

	return AK_TRUE;
}

AkBool FRenderer::CreateRTVs()
{
	CD3DX12_CPU_DESCRIPTOR_HANDLE hRtvCpu(_pRTVHeap->GetCPUDescriptorHandleForHeapStart());

	for (AkU32 i = 0; i < SWAP_CHAIN_FRAME_COUNT; i++)
	{
		_pSwapChain->GetBuffer(i, IID_PPV_ARGS(&_ppBackBufferRT[i]));
		_pDevice->CreateRenderTargetView(_ppBackBufferRT[i], nullptr, hRtvCpu);
		hRtvCpu.Offset(1, _uRTVDesciptorSize);
	}

	return AK_TRUE;
}

AkBool FRenderer::CreateRTVsAndSRVsForPBR()
{
	// Float RTV & SRV
	D3D12_RESOURCE_DESC tRtvDesc = {};
	tRtvDesc = _ppBackBufferRT[0]->GetDesc();
	tRtvDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;

	D3D12_CLEAR_VALUE tClearValue = {};
	const AkF32 fClearColor[] = { 0.0f, 0.0f, 1.0f, 1.0f };
	memcpy(tClearValue.Color, fClearColor, sizeof(fClearColor));
	tClearValue.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;

	if (FAILED(_pDevice->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT), D3D12_HEAP_FLAG_NONE, &tRtvDesc, D3D12_RESOURCE_STATE_RENDER_TARGET, &tClearValue, IID_PPV_ARGS(&_pResolvedBufferRT))))
	{
		__debugbreak();
		return AK_FALSE;
	}

	CD3DX12_CPU_DESCRIPTOR_HANDLE hRtvCpu(_pRTVHeap->GetCPUDescriptorHandleForHeapStart(), 3, _uRTVDesciptorSize);

	_pDevice->CreateRenderTargetView(_pResolvedBufferRT, nullptr, hRtvCpu);

	CD3DX12_CPU_DESCRIPTOR_HANDLE hSrvCpu = {};
	if (_pDescriptorAllocator->AllocDescriptorHandle(&hSrvCpu))
	{
		_pDevice->CreateShaderResourceView(_pResolvedBufferRT, nullptr, hSrvCpu);
	}

	_hResolvedBufferSrvCpu = hSrvCpu;

	return AK_TRUE;
}

AkBool FRenderer::CreateDSVs(AkU32 uWidth, AkU32 uHeight)
{
	// Create Main Depth Stencil View.
	{
		D3D12_DEPTH_STENCIL_VIEW_DESC tDepthStencilDesc = {};
		tDepthStencilDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		tDepthStencilDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
		tDepthStencilDesc.Flags = D3D12_DSV_FLAG_NONE;

		D3D12_CLEAR_VALUE tDepthOptimizedClearValue = {};
		tDepthOptimizedClearValue.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		tDepthOptimizedClearValue.DepthStencil.Depth = 1.0f;
		tDepthOptimizedClearValue.DepthStencil.Stencil = 0;

		CD3DX12_RESOURCE_DESC tDepthDesc(
			D3D12_RESOURCE_DIMENSION_TEXTURE2D,
			0,
			uWidth,
			uHeight,
			1,
			1,
			DXGI_FORMAT_R24G8_TYPELESS,
			1,
			0,
			D3D12_TEXTURE_LAYOUT_UNKNOWN,
			D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL);

		if (FAILED(_pDevice->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT), D3D12_HEAP_FLAG_NONE, &tDepthDesc, D3D12_RESOURCE_STATE_DEPTH_WRITE, &tDepthOptimizedClearValue, IID_PPV_ARGS(&_pMainDS))))
		{
			__debugbreak();
			return AK_FALSE;
		}

		CD3DX12_CPU_DESCRIPTOR_HANDLE hDsvHandle(_pDSVHeap->GetCPUDescriptorHandleForHeapStart());
		_pDevice->CreateDepthStencilView(_pMainDS, &tDepthStencilDesc, hDsvHandle);
	}

	return AK_TRUE;
}

AkBool FRenderer::CreateShadowDSVs(AkU32 uWidth, AkU32 uHeight)
{
	// Create Shadow Map Depth Stencil View.
	{
		D3D12_DEPTH_STENCIL_VIEW_DESC tDepthStencilDesc = {};
		tDepthStencilDesc.Format = DXGI_FORMAT_D32_FLOAT;
		tDepthStencilDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
		tDepthStencilDesc.Flags = D3D12_DSV_FLAG_NONE;

		D3D12_CLEAR_VALUE tDepthOptimizedClearValue = {};
		tDepthOptimizedClearValue.Format = DXGI_FORMAT_D32_FLOAT;
		tDepthOptimizedClearValue.DepthStencil.Depth = 1.0f;
		tDepthOptimizedClearValue.DepthStencil.Stencil = 0;

		CD3DX12_RESOURCE_DESC tDepthDesc(
			D3D12_RESOURCE_DIMENSION_TEXTURE2D,
			0,
			uWidth,
			uHeight,
			1,
			1,
			DXGI_FORMAT_R32_TYPELESS,
			1,
			0,
			D3D12_TEXTURE_LAYOUT_UNKNOWN,
			D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL);

		for (AkU32 i = 0; i < CASCADE_SHADOW_MAP_LEVEL; i++)
		{
			if (FAILED(_pDevice->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT), D3D12_HEAP_FLAG_NONE, &tDepthDesc, D3D12_RESOURCE_STATE_GENERIC_READ, &tDepthOptimizedClearValue, IID_PPV_ARGS(&_pShadowDS[i]))))
			{
				__debugbreak();
				return AK_FALSE;
			}
		}

		CD3DX12_CPU_DESCRIPTOR_HANDLE hDsvHandle(_pDSVHeap->GetCPUDescriptorHandleForHeapStart(), 1, _uDSVDescriptorSize);

		for (AkU32 i = 0; i < CASCADE_SHADOW_MAP_LEVEL; i++)
		{
			_pDevice->CreateDepthStencilView(_pShadowDS[i], &tDepthStencilDesc, hDsvHandle);

			hDsvHandle.Offset(1, _uDSVDescriptorSize);
		}
	}

	// Create SRV to resource so we can sample the shadow map in a shader program.
	{
		D3D12_SHADER_RESOURCE_VIEW_DESC tSrvDesc = {};
		tSrvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		tSrvDesc.Format = DXGI_FORMAT_R32_FLOAT;
		tSrvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		tSrvDesc.Texture2D.MostDetailedMip = 0;
		tSrvDesc.Texture2D.MipLevels = 1;
		tSrvDesc.Texture2D.ResourceMinLODClamp = 0.0f;
		tSrvDesc.Texture2D.PlaneSlice = 0;

		for (AkU32 i = 0; i < CASCADE_SHADOW_MAP_LEVEL; i++)
		{
			CD3DX12_CPU_DESCRIPTOR_HANDLE hSrvCpu = {};
			if (_pDescriptorAllocator->AllocDescriptorHandle(&hSrvCpu))
			{
				_pDevice->CreateShaderResourceView(_pShadowDS[i], &tSrvDesc, hSrvCpu);

				_pShadowMapSrvCpu[i] = hSrvCpu;
			}
		}
	}

	return AK_TRUE;
}

AkBool FRenderer::CreateFence()
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

AkBool FRenderer::CreateRenderThreadPool(AkU32 uThreadCount)
{
	_pRenderThreadDescList = reinterpret_cast<RENDER_THREAD_DESC*>(malloc(sizeof(RENDER_THREAD_DESC) * uThreadCount));
	memset(_pRenderThreadDescList, 0, sizeof(RENDER_THREAD_DESC) * uThreadCount);

	_hCompleteEvent = ::CreateEvent(nullptr, AK_FALSE, AK_FALSE, nullptr);
	for (AkU32 i = 0; i < uThreadCount; i++)
	{
		for (AkU32 j = 0; j < (AkU32)RENDER_THREAD_EVENT_TYPE::RENDER_THREAD_EVENT_TYPE_COUNT; j++)
		{
			_pRenderThreadDescList[i].hEventList[j] = ::CreateEvent(nullptr, AK_FALSE, AK_FALSE, nullptr);
		}
		_pRenderThreadDescList[i].pRenderer = this;
		_pRenderThreadDescList[i].uThreadIndex = i;
		AkU32 uThreadID = 0;
		_pRenderThreadDescList[i].hThread = reinterpret_cast<HANDLE>(_beginthreadex(nullptr, 0, RenderThreadByProcess, _pRenderThreadDescList + i, 0, &uThreadID));
	}

	return AK_TRUE;
}

AkBool FRenderer::CreatePostProcess()
{
	_pPostProcess = new FPostProcess;
	if (!_pPostProcess->Initialize(this))
	{
		__debugbreak();
		return AK_FALSE;
	}

	return AK_TRUE;
}

AkBool FRenderer::CreateImGuiInitResource()
{
	AkBool bResult = AK_TRUE;

	D3D12_DESCRIPTOR_HEAP_DESC tDesc = {};
	tDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	tDesc.NumDescriptors = 1;
	tDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	if (_pDevice->CreateDescriptorHeap(&tDesc, IID_PPV_ARGS(&_pImGuiHeap)) != S_OK)
	{
		bResult = AK_FALSE;
	}

	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

	// Setup Dear ImGui style
	ImGui::StyleColorsDark();

	// Setup Platform/Renderer backends
	bResult = ImGui_ImplWin32_Init(_hWnd);
	bResult = ImGui_ImplDX12_Init(_pDevice, PENDING_FRAME_COUNT, DXGI_FORMAT_R8G8B8A8_UNORM, _pImGuiHeap, _pImGuiHeap->GetCPUDescriptorHandleForHeapStart(), _pImGuiHeap->GetGPUDescriptorHandleForHeapStart());

	return bResult;
}

void FRenderer::InitViewports(AkF32 fWidth, AkF32 fHeight)
{
	_tViewport.Width = fWidth;
	_tViewport.Height = fHeight;
	_tViewport.MinDepth = 0.0f;
	_tViewport.MaxDepth = 1.0f;

	_tShadowViewport.Width = (AkF32)_uShadowWidth;
	_tShadowViewport.Height = (AkF32)_uShadowHeight;
	_tShadowViewport.MinDepth = 0.0f;
	_tShadowViewport.MaxDepth = 1.0f;
}

void FRenderer::InitScissorRect(AkU32 uWidth, AkU32 uHeight)
{
	_tScissorRect.left = 0;
	_tScissorRect.top = 0;
	_tScissorRect.right = uWidth;
	_tScissorRect.bottom = uHeight;

	_tShadowScissorRect.left = 0;
	_tShadowScissorRect.top = 0;
	_tShadowScissorRect.right = _uShadowWidth;
	_tShadowScissorRect.bottom = _uShadowHeight;
}

void FRenderer::InitCamera()
{
	_vCamPos = Vector3(0.0f, 0.0f, 1.0f);
	_vCamDir = Vector3(0.0f, 0.0f, 1.0f);
	Vector3 vCamUp = Vector3(0.0f, 1.0f, 0.0f);

	SetCamera(&_vCamPos, &_vCamDir, &vCamUp);
}

void FRenderer::UpdateCascadeOrthoProjMatrix()
{
	_pCascadeBoundary[0] = _fNear;
	_pCascadeBoundary[1] = _fFar / 50.0f;
	_pCascadeBoundary[2] = _fFar / 25.0f;
	_pCascadeBoundary[3] = _fFar / 10.0f;
	_pCascadeBoundary[4] = _fFar / 2.0f;
	_pCascadeBoundary[5] = _fFar;

	AkF32 fNumer = _fFar - _fNear;

	const AkF32 fAspectRatio = static_cast<AkF32>(_uScreenWidth) / static_cast<AkF32>(_uScreenHeight);

	for (AkU32 i = 0; i < CASCADE_SHADOW_MAP_LEVEL; i++)
	{
		Matrix mProj = XMMatrixPerspectiveFovLH(_fFov, fAspectRatio, _pCascadeBoundary[i], _pCascadeBoundary[i + 1]); // Row Major.
		Matrix mView = _mViewMat; // Row Major.

		Matrix mInvViewProj = (mView * mProj).Invert();

		AkU32 uConerNum = 0;
		for (AkU32 x = 0; x < 2; x++)
		{
			for (AkU32 y = 0; y < 2; y++)
			{
				for (AkU32 z = 0; z < 2; z++)
				{
					Vector4 vPoint = Vector4::Transform(Vector4(2.0f * x - 1.0f, 2.0f * y - 1.0f, (AkF32)z, 1.0f), mInvViewProj);
					vPoint /= vPoint.w;

					_pFrustumPoints[uConerNum++] = vPoint; // Frustum 의 World Position.
				}
			}
		}

		// Calc Frsutum Center.
		Vector3 vCenter = Vector3(0.0f);
		for (AkU32 j = 0; j < 8; j++)
		{
			vCenter += Vector3(_pFrustumPoints[j].x, _pFrustumPoints[j].y, _pFrustumPoints[j].z);
		}
		vCenter /= 8.0f;

		// View Matrix.
		_tGlobalLight.vDirection.Normalize();
		_pShadowView[i] = XMMatrixLookAtLH(vCenter + _tGlobalLight.vDirection, vCenter, Vector3(0.0f, 1.0f, 0.0f));

		AkF32 fMinX = AK_MAX_F32;
		AkF32 fMaxX = -AK_MAX_F32;
		AkF32 fMinY = AK_MAX_F32;
		AkF32 fMaxY = -AK_MAX_F32;
		AkF32 fMinZ = AK_MAX_F32;
		AkF32 fMaxZ = -AK_MAX_F32;

		for (AkU32 j = 0; j < 8; j++)
		{
			Vector4 vTrf = Vector4::Transform(_pFrustumPoints[j], _pShadowView[i]);

			fMinX = min(fMinX, vTrf.x);
			fMaxX = max(fMaxX, vTrf.x);
			fMinY = min(fMinY, vTrf.y);
			fMaxY = max(fMaxY, vTrf.y);
			fMinZ = min(fMinZ, vTrf.z);
			fMaxZ = max(fMaxZ, vTrf.z);
		}

		// printf("Before Frustum Z : %lf %lf\n", fMinZ, fMaxZ);

		// Projection Matrix.
		_pShadowOrthoProj[i] = XMMatrixOrthographicOffCenterLH(fMinX, fMaxX, fMinY, fMaxY, fMinZ, fMaxZ);
	}
}

void FRenderer::DestroyDevice()
{
	if (_pDevice)
	{
		AkU32 uRefCount = _pDevice->Release();
		if (uRefCount)
		{
			IDXGIDebug1* pDebug = nullptr;
			if (SUCCEEDED(DXGIGetDebugInterface1(0, IID_PPV_ARGS(&pDebug))))
			{
				pDebug->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_SUMMARY);
				pDebug->Release();
			}
			__debugbreak();
		}

		_pDevice = nullptr;
	}
}

void FRenderer::DestroyCmdQueue()
{
	if (_pCmdQueue)
	{
		_pCmdQueue->Release();
		_pCmdQueue = nullptr;
	}
}

void FRenderer::DestroyDescriptorForRTV()
{
	if (_pRTVHeap)
	{
		_pRTVHeap->Release();
		_pRTVHeap = nullptr;
	}
}

void FRenderer::DestroyDescriptorForDSV()
{
	if (_pDSVHeap)
	{
		_pDSVHeap->Release();
		_pDSVHeap = nullptr;
	}
}

void FRenderer::DestroyRTVs()
{
	for (AkU32 i = 0; i < SWAP_CHAIN_FRAME_COUNT; i++)
	{
		if (_ppBackBufferRT[i])
		{
			_ppBackBufferRT[i]->Release();
			_ppBackBufferRT[i] = nullptr;
		}
	}
}

void FRenderer::DestroyRTVsAndSRVsForPBR()
{
	if (_pResolvedBufferRT)
	{
		_pResolvedBufferRT->Release();
		_pResolvedBufferRT = nullptr;
	}
}

void FRenderer::DestroyDSVs()
{
	if (_pMainDS)
	{
		_pMainDS->Release();
		_pMainDS = nullptr;
	}
}

void FRenderer::DestroyShadowDSVs()
{
	for (AkU32 i = 0; i < CASCADE_SHADOW_MAP_LEVEL; i++)
	{
		if (_pShadowDS[i])
		{
			_pShadowDS[i]->Release();
			_pShadowDS[i] = nullptr;
		}
	}
}

void FRenderer::DestroyFence()
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

void FRenderer::DestroyRenderThreadPool(AkU32 uThreadCount)
{
	if (_pRenderThreadDescList)
	{
		for (AkU32 i = 0; i < uThreadCount; i++)
		{
			::SetEvent(_pRenderThreadDescList[i].hEventList[(AkU32)RENDER_THREAD_EVENT_TYPE::RENDER_THREAD_EVENT_TYPE_DESTROY]);

			::WaitForSingleObject(_pRenderThreadDescList[i].hThread, INFINITE);
			::CloseHandle(_pRenderThreadDescList[i].hThread);
			_pRenderThreadDescList[i].hThread = nullptr;

			for (AkU32 j = 0; j < (AkU32)::RENDER_THREAD_EVENT_TYPE::RENDER_THREAD_EVENT_TYPE_COUNT; j++)
			{
				if (_pRenderThreadDescList[i].hEventList[j])
				{
					::CloseHandle(_pRenderThreadDescList[i].hEventList[j]);
					_pRenderThreadDescList[i].hEventList[j] = nullptr;
				}
			}
		}

		free(_pRenderThreadDescList);
		_pRenderThreadDescList = nullptr;
	}
	if (_hCompleteEvent)
	{
		::CloseHandle(_hCompleteEvent);
		_hCompleteEvent = nullptr;
	}
}

void FRenderer::DestroyPostProcess()
{
	if (_pPostProcess)
	{
		delete _pPostProcess;
		_pPostProcess = nullptr;
	}
}

void FRenderer::DestroyImGuiInitResource()
{
	Fence();

	for (AkU32 i = 0; i < PENDING_FRAME_COUNT; i++)
	{
		WaitForFenceValue(_u64FenceValue[i]);
	}

	// Cleanup
	ImGui_ImplDX12_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();

	if (_pImGuiHeap)
	{
		_pImGuiHeap->Release();
		_pImGuiHeap = nullptr;
	}
}

void FRenderer::DestroySwapChain()
{
	if (_pSwapChain)
	{
		_pSwapChain->Release();
		_pSwapChain = nullptr;
	}
}
