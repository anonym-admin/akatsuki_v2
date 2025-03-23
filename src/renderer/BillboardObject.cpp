#include "pch.h"
#include "BillboardObject.h"
#include "Renderer.h"
#include "DescriptorPool.h"
#include "ConstantBufferManager.h"
#include "ConstantBufferPool.h"
#include "ResourceManager.h"
#include "TextureManager.h"

/*
=================
BillboardObject
=================
*/

ID3D12RootSignature* FBillboardObjects::sm_pRootSignature;
ID3D12PipelineState* FBillboardObjects::sm_pBillboardPSO;
AkU32 FBillboardObjects::sm_uInitRefCount;

FBillboardObjects::FBillboardObjects()
{
}

FBillboardObjects::~FBillboardObjects()
{
	CleanUp();
}

AkBool FBillboardObjects::Initialize(FRenderer* pRenderer)
{
	AkBool bResult = AK_TRUE;

	_pRenderer = pRenderer;

	bResult = CreateCommonResources();

	return bResult;
}

AkBool FBillboardObjects::CreateBillboardBuffer(BillboardData_t* pBillboardData)
{
	FTextureManager* pTextureManager = _pRenderer->GetTextureManager();
	FResourceManager* pResourceManager = _pRenderer->GetResourceManager();
	D3D12_VERTEX_BUFFER_VIEW tVBView = {};
	ID3D12Resource* pVertexBuffer = nullptr;

	if (pResourceManager->CreateVertexBuffer(sizeof(VertexSize_t), pBillboardData->uPointsNum, &tVBView, &pVertexBuffer, pBillboardData->pVertice))
	{
		_pVertexBuffer = pVertexBuffer;
		_tVertexBufferView = tVBView;
	}

	_uPointNum = pBillboardData->uPointsNum;

	_pMaterials = new MaterialConstantBuffer_t;

	// Array
	if (!wcscmp(pBillboardData->wcArrayFilename, L""))
	{
		_pArrayTextureHandle = pTextureManager->CreateNullTexture();
	}
	else
	{
		_pArrayTextureHandle = reinterpret_cast<TextureHandle_t*>(_pRenderer->CreateTextureFromFile(pBillboardData->wcArrayFilename, AK_FALSE, AK_TRUE));
	}
	// Albedo
	if (!wcscmp(pBillboardData->wcAlbedoTextureFilename, L""))
	{
		_pAlbedoTextureHandle = pTextureManager->CreateNullTexture();
	}
	else
	{
		_pAlbedoTextureHandle = reinterpret_cast<TextureHandle_t*>(_pRenderer->CreateTextureFromFile(pBillboardData->wcAlbedoTextureFilename, AK_TRUE));
		_pMaterials->uUseAlbedoMap = AK_TRUE;
	}
	// Normal
	if (!wcscmp(pBillboardData->wcNormalTextureFilename, L""))
	{
		_pNormalTextureHandle = pTextureManager->CreateNullTexture();
	}
	else
	{
		_pNormalTextureHandle = reinterpret_cast<TextureHandle_t*>(_pRenderer->CreateTextureFromFile(pBillboardData->wcNormalTextureFilename, AK_FALSE));
		_pMaterials->uUseNormalMap = AK_TRUE;
	}
	// Emissive
	if (!wcscmp(pBillboardData->wcEmissiveTextureFilename, L""))
	{
		_pEmissiveTextureHandle = pTextureManager->CreateNullTexture();
	}
	else
	{
		_pEmissiveTextureHandle = reinterpret_cast<TextureHandle_t*>(_pRenderer->CreateTextureFromFile(pBillboardData->wcEmissiveTextureFilename, AK_TRUE));
		_pMaterials->uUseEimissiveMap = AK_TRUE;
	}
	// Metallic.
	if (!wcscmp(pBillboardData->wcMetallicTextureFilename, L""))
	{
		_pMetallicTextureHandle = pTextureManager->CreateNullTexture();
	}
	else
	{
		_pMetallicTextureHandle = reinterpret_cast<TextureHandle_t*>(_pRenderer->CreateTextureFromFile(pBillboardData->wcMetallicTextureFilename, AK_FALSE));
		_pMaterials->uUseMetallicMap = AK_TRUE;
	}
	// Roughness
	if (!wcscmp(pBillboardData->wcRoughnessTextureFilename, L""))
	{
		_pRoughnessTextureHandle = pTextureManager->CreateNullTexture();
	}
	else
	{
		_pRoughnessTextureHandle = reinterpret_cast<TextureHandle_t*>(_pRenderer->CreateTextureFromFile(pBillboardData->wcRoughnessTextureFilename, AK_FALSE));
		_pMaterials->uUseRoughnessMap = AK_TRUE;
	}
	// Ambient Occulusion
	if (!wcscmp(pBillboardData->wcAoTextureFilename, L""))
	{
		_pAoTextureHandle = pTextureManager->CreateNullTexture();
	}
	else
	{
		_pAoTextureHandle = reinterpret_cast<TextureHandle_t*>(_pRenderer->CreateTextureFromFile(pBillboardData->wcAoTextureFilename, AK_FALSE));
		_pMaterials->uUseAOMap = AK_TRUE;
	}

	return AK_TRUE;
}

AkBool FBillboardObjects::UpdateMaterialBuffers(const Vector3* pAlbedoFactor, AkF32 fMetallicFactor, AkF32 fRoughnessFactor, const Vector3* pEmisiionFactor)
{
	_pMaterials->vAlbedoFactor = *pAlbedoFactor;
	_pMaterials->fMetallicFactor = fMetallicFactor;
	_pMaterials->fRoughnessFactor = fRoughnessFactor;
	_pMaterials->vEmissionFactor = *pEmisiionFactor;

	return AK_TRUE;
}

HRESULT __stdcall FBillboardObjects::QueryInterface(REFIID riid, void** ppvObject)
{
	return E_NOTIMPL;
}

ULONG __stdcall FBillboardObjects::AddRef(void)
{
	AkU32 uRefCount = ++_uRefCount;
	return uRefCount;
}

ULONG __stdcall FBillboardObjects::Release(void)
{
	AkU32 uRefCount = --_uRefCount;
	if (!uRefCount)
	{
		delete this;
	}
	return uRefCount;
}

void FBillboardObjects::Draw(AkU32 uThreadIndex, ID3D12GraphicsCommandList* pCmdList, const Matrix* pWorldMat)
{
	ID3D12Device* pDevice = _pRenderer->GetDevice();
	FDescriptorPool* pDescriptorPool = _pRenderer->GetDescriptorPool(uThreadIndex);
	ID3D12DescriptorHeap* pDescriptorHeap = pDescriptorPool->GetDescriptorHeap();
	FConstantBufferPool* pGlobalCBPool = _pRenderer->GetConstantBufferPool(uThreadIndex, CONSTANT_BUFFER_TYPE::CONSTANT_BUFFER_TYPE_GLOBAL);
	FConstantBufferPool* pMeshCBPool = _pRenderer->GetConstantBufferPool(uThreadIndex, CONSTANT_BUFFER_TYPE::CONSTANT_BUFFER_TYPE_MESH);
	FConstantBufferPool* pMaterialCBPool = _pRenderer->GetConstantBufferPool(uThreadIndex, CONSTANT_BUFFER_TYPE::CONSTANT_BUFFER_TYPE_MATERIAL);
	AkU32 uDescriptorSize = pDescriptorPool->GetDescriptorTypeSize();

	CD3DX12_CPU_DESCRIPTOR_HANDLE hCPU = {};
	CD3DX12_GPU_DESCRIPTOR_HANDLE hGPU = {};
	AkU32 uRequiredDescriptorNum = DESCRIPTOR_COUNT_PER_OBJ * (_uPointNum * DESCRIPTOR_COUNT_PER_MESH);

	if (!pDescriptorPool->AllocDescriptorTable(&hCPU, &hGPU, uRequiredDescriptorNum))
	{
		__debugbreak();
		return;
	}

	CBContainer_t* pGlobalCBContainer = pGlobalCBPool->Alloc();
	if (!pGlobalCBContainer)
	{
		__debugbreak();
		return;
	}

	GlobalConstantBuffer_t* pGlobalConstantBuffer = reinterpret_cast<GlobalConstantBuffer_t*>(pGlobalCBContainer->pSystemMemAddr);
	_pRenderer->GetViewPorjMatrix(&pGlobalConstantBuffer->mView, &pGlobalConstantBuffer->mProj);
	_pRenderer->GetCameraPosition(&pGlobalConstantBuffer->vEyeWorld.x, &pGlobalConstantBuffer->vEyeWorld.y, &pGlobalConstantBuffer->vEyeWorld.z);
	pGlobalConstantBuffer->fStrengthIBL = _pRenderer->GetIBLStrength();

	// Per Obj (b0).
	CD3DX12_CPU_DESCRIPTOR_HANDLE hDest(hCPU, 0, uDescriptorSize);
	pDevice->CopyDescriptorsSimple(1, hDest, pGlobalCBContainer->hCPU, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	hDest.Offset(1, uDescriptorSize);

	CBContainer_t* pMeshCBContainer = pMeshCBPool->Alloc();
	if (!pMeshCBContainer)
	{
		__debugbreak();
		return;
	}

	MeshConstantBuffer_t* pMeshConstantBuffer = reinterpret_cast<MeshConstantBuffer_t*>(pMeshCBContainer->pSystemMemAddr);
	Matrix mWorldRow = *pWorldMat;
	pMeshConstantBuffer->mWorld = mWorldRow.Transpose();
	mWorldRow.Translation(Vector3(0.0f));
	mWorldRow.Invert().Transpose();
	pMeshConstantBuffer->mWorldIT = mWorldRow.Transpose();

	// Per Obj (b1).
	pDevice->CopyDescriptorsSimple(1, hDest, pMeshCBContainer->hCPU, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	hDest.Offset(1, uDescriptorSize);

	// Per Billboard 
	TextureHandle_t* pIrradianceTexHandle = nullptr;
	TextureHandle_t* pSpecularTexHandle = nullptr;
	TextureHandle_t* pBrdfTexHandle = nullptr;
	_pRenderer->GetIBLTexture(&pIrradianceTexHandle, &pSpecularTexHandle, &pBrdfTexHandle);

	for (AkU32 i = 0; i < _uPointNum; i++)
	{
		CBContainer_t* pMaterialCBContainer = pMaterialCBPool->Alloc();
		if (!pMaterialCBContainer)
		{
			__debugbreak();
			return;
		}

		MaterialConstantBuffer_t* pMaterialConstantBuffer = reinterpret_cast<MaterialConstantBuffer_t*>(pMaterialCBContainer->pSystemMemAddr);
		memcpy(pMaterialConstantBuffer, &_pMaterials[i], sizeof(MaterialConstantBuffer_t));

		// Material CB(b2)
		pDevice->CopyDescriptorsSimple(1, hDest, pMaterialCBContainer->hCPU, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		hDest.Offset(1, uDescriptorSize);

		// Albedo
		TextureHandle_t* pTexHandle = _pAlbedoTextureHandle;
		if (pTexHandle)
		{
			pDevice->CopyDescriptorsSimple(1, hDest, pTexHandle->hSRV, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		}
		else
		{
			__debugbreak();
		}
		hDest.Offset(1, uDescriptorSize);

		// Normal
		pTexHandle = _pNormalTextureHandle;
		if (pTexHandle)
		{
			pDevice->CopyDescriptorsSimple(1, hDest, pTexHandle->hSRV, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		}
		else
		{
			__debugbreak();
		}
		hDest.Offset(1, uDescriptorSize);

		// Emissive
		pTexHandle = _pEmissiveTextureHandle;
		if (pTexHandle)
		{
			pDevice->CopyDescriptorsSimple(1, hDest, pTexHandle->hSRV, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		}
		else
		{
			__debugbreak();
		}
		hDest.Offset(1, uDescriptorSize);

		// Metallic
		pTexHandle = _pMetallicTextureHandle;
		if (pTexHandle)
		{
			pDevice->CopyDescriptorsSimple(1, hDest, pTexHandle->hSRV, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		}
		else
		{
			__debugbreak();
		}
		hDest.Offset(1, uDescriptorSize);

		// Roughness
		pTexHandle = _pRoughnessTextureHandle;
		if (pTexHandle)
		{
			pDevice->CopyDescriptorsSimple(1, hDest, pTexHandle->hSRV, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		}
		else
		{
			__debugbreak();
		}
		hDest.Offset(1, uDescriptorSize);

		// AO
		pTexHandle = _pAoTextureHandle;
		if (pTexHandle)
		{
			pDevice->CopyDescriptorsSimple(1, hDest, pTexHandle->hSRV, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		}
		else
		{
			__debugbreak();
		}
		hDest.Offset(1, uDescriptorSize);

		// Irradiance IBL.
		if (pIrradianceTexHandle)
		{
			pDevice->CopyDescriptorsSimple(1, hDest, pIrradianceTexHandle->hSRV, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		}
		else
		{
			__debugbreak();
		}
		hDest.Offset(1, uDescriptorSize);

		// Specular IBL
		if (pSpecularTexHandle)
		{
			pDevice->CopyDescriptorsSimple(1, hDest, pSpecularTexHandle->hSRV, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		}
		else
		{
			__debugbreak();
		}
		hDest.Offset(1, uDescriptorSize);

		// Brdf Tex
		if (pBrdfTexHandle)
		{
			pDevice->CopyDescriptorsSimple(1, hDest, pBrdfTexHandle->hSRV, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		}
		else
		{
			__debugbreak();
		}
		hDest.Offset(1, uDescriptorSize);

		// Shadow Map
		for (AkU32 uCascadeIndex = 0; uCascadeIndex < FRenderer::CASCADE_SHADOW_MAP_LEVEL; uCascadeIndex++)
		{
			D3D12_CPU_DESCRIPTOR_HANDLE hSRV = {};
			_pRenderer->GetShadowMapSrv(&hSRV, uCascadeIndex);
			if (hSRV.ptr)
			{
				pDevice->CopyDescriptorsSimple(1, hDest, hSRV, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
			}
			else
			{
				AkI32 a = 3;
			}
			hDest.Offset(1, uDescriptorSize);
		}

		// Array Tex
		pTexHandle = _pArrayTextureHandle;
		if (pTexHandle)
		{
			pDevice->CopyDescriptorsSimple(1, hDest, pTexHandle->hSRV, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		}
		else
		{
			__debugbreak();
		}
		hDest.Offset(1, uDescriptorSize);
	}

	// Set RootSignature.
	pCmdList->SetGraphicsRootSignature(sm_pRootSignature);
	pCmdList->SetDescriptorHeaps(1, &pDescriptorHeap);
	pCmdList->SetPipelineState(sm_pBillboardPSO);

	// Obj (root param 0)
	pCmdList->SetGraphicsRootDescriptorTable(0, hGPU);
	pCmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_POINTLIST);

	CD3DX12_GPU_DESCRIPTOR_HANDLE hGPUforMeshes(hGPU, DESCRIPTOR_COUNT_PER_OBJ, uDescriptorSize);
	pCmdList->SetGraphicsRootDescriptorTable(1, hGPUforMeshes);
	pCmdList->IASetVertexBuffers(0, 1, &_tVertexBufferView);
	pCmdList->DrawInstanced(_uPointNum, 1, 0, 0);
}

void FBillboardObjects::CleanUp()
{
	_pRenderer->EnsureCompleted();

	if (_pVertexBuffer)
	{
		_pVertexBuffer->Release();
		_pVertexBuffer = nullptr;
	}

	if (_pAlbedoTextureHandle)
	{
		_pRenderer->DestroyTexture(_pAlbedoTextureHandle);
		_pAlbedoTextureHandle = nullptr;
	}
	if (_pNormalTextureHandle)
	{
		_pRenderer->DestroyTexture(_pNormalTextureHandle);
		_pNormalTextureHandle = nullptr;
	}
	if (_pEmissiveTextureHandle)
	{
		_pRenderer->DestroyTexture(_pEmissiveTextureHandle);
		_pEmissiveTextureHandle = nullptr;
	}
	if (_pMetallicTextureHandle)
	{
		_pRenderer->DestroyTexture(_pMetallicTextureHandle);
		_pMetallicTextureHandle = nullptr;
	}
	if (_pRoughnessTextureHandle)
	{
		_pRenderer->DestroyTexture(_pRoughnessTextureHandle);
		_pRoughnessTextureHandle = nullptr;
	}
	if (_pAoTextureHandle)
	{
		_pRenderer->DestroyTexture(_pAoTextureHandle);
		_pAoTextureHandle = nullptr;
	}
	if (_pArrayTextureHandle)
	{
		_pRenderer->DestroyTexture(_pArrayTextureHandle);
		_pArrayTextureHandle = nullptr;
	}
	if (_pMaterials)
	{
		free(_pMaterials);
		_pMaterials = nullptr;
	}

	DestroyCommonResources();
}

AkBool FBillboardObjects::CreateCommonResources()
{
	if (sm_uInitRefCount)
	{
		sm_uInitRefCount++;
		return AK_TRUE;
	}

	if (!CreateRootSignature())
	{
		__debugbreak();
		return AK_FALSE;
	}
	if (!CreatePipelineState())
	{
		__debugbreak();
		return AK_FALSE;
	}

	sm_uInitRefCount++;

	return AK_TRUE;
}

AkBool FBillboardObjects::CreateRootSignature()
{
	ID3D12Device* pDevice = _pRenderer->GetDevice();
	ID3DBlob* pSignature = nullptr;
	ID3DBlob* pError = nullptr;

	CD3DX12_DESCRIPTOR_RANGE tRangesPerObj[1] = {};
	tRangesPerObj[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 2, 0);	// b0, b1: Constant Buffer View per Object.

	CD3DX12_DESCRIPTOR_RANGE tRangesPerTriGroup[5] = {};
	tRangesPerTriGroup[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 2);	// b2: Constant Buffer View per Mesh
	tRangesPerTriGroup[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 6, 0);	// t0~t5 : Shader Resource View(Tex) per Mesh.
	tRangesPerTriGroup[2].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 3, 11);	// t10, t11, t12, t13 : Shader Resource View(Tex) per Mesh. (IBL Texture)
	tRangesPerTriGroup[3].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 5, 15);	// t15, t16, t17, t18, t19 : Shadow Map
	tRangesPerTriGroup[4].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 20); // t20 ~ texture array

	CD3DX12_ROOT_PARAMETER tRootParameters[2] = {};
	tRootParameters[0].InitAsDescriptorTable(_countof(tRangesPerObj), tRangesPerObj, D3D12_SHADER_VISIBILITY_ALL);
	tRootParameters[1].InitAsDescriptorTable(_countof(tRangesPerTriGroup), tRangesPerTriGroup, D3D12_SHADER_VISIBILITY_ALL);

	// sampler
	CD3DX12_STATIC_SAMPLER_DESC pSamplerDesc[7] = {};
	FD3DUtils::GetStaticSamplers(pSamplerDesc);

	// Allow input layout and deny uneccessary access to certain pipeline stages.
	D3D12_ROOT_SIGNATURE_FLAGS tRootSignatureFlags =
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_PIXEL_SHADER_ROOT_ACCESS;

	// Create an empty root signature.
	CD3DX12_ROOT_SIGNATURE_DESC tRootSignatureDesc;
	//rootSignatureDesc.Init(0, nullptr, 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);
	tRootSignatureDesc.Init(_countof(tRootParameters), tRootParameters, _countof(pSamplerDesc), pSamplerDesc, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	if (FAILED(D3D12SerializeRootSignature(&tRootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &pSignature, &pError)))
	{
		__debugbreak();
		return AK_FALSE;
	}

	if (FAILED(pDevice->CreateRootSignature(0, pSignature->GetBufferPointer(), pSignature->GetBufferSize(), IID_PPV_ARGS(&sm_pRootSignature))))
	{
		__debugbreak();
		return AK_FALSE;
	}

	if (pSignature)
	{
		pSignature->Release();
		pSignature = nullptr;
	}
	if (pError)
	{
		pError->Release();
		pError = nullptr;
	}

	return AK_TRUE;
}

AkBool FBillboardObjects::CreatePipelineState()
{
	ID3D12Device* pDevice = _pRenderer->GetDevice();

	ID3DBlob* pBillboardVS = nullptr;
	ID3DBlob* pBillboardGS = nullptr;
	ID3DBlob* pBillboardPS = nullptr;

#if defined(_DEBUG)
	// Enable better shader debugging with the graphics debugging tools.
	AkU32 uCompileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
	AkU32 uCompileFlags = 0;
#endif

	ID3DBlob* pErrorBlob = nullptr;
	if (FAILED(D3DCompileFromFile(L"../../shader/Billboard.hlsl", nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "VSMain", "vs_5_0", uCompileFlags, 0, &pBillboardVS, &pErrorBlob)))
	{
		if (pErrorBlob != nullptr)
		{
			OutputDebugStringA((char*)pErrorBlob->GetBufferPointer());
			pErrorBlob->Release();
		}
		__debugbreak();
	}
	if (FAILED(D3DCompileFromFile(L"../../shader/Billboard.hlsl", nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "GSMain", "gs_5_0", uCompileFlags, 0, &pBillboardGS, &pErrorBlob)))
	{
		if (pErrorBlob != nullptr)
		{
			OutputDebugStringA((char*)pErrorBlob->GetBufferPointer());
			pErrorBlob->Release();
		}
		__debugbreak();
	}
	if (FAILED(D3DCompileFromFile(L"../../shader/Billboard.hlsl", nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "PSMain", "ps_5_0", uCompileFlags, 0, &pBillboardPS, &pErrorBlob)))
	{
		if (pErrorBlob != nullptr)
		{
			OutputDebugStringA((char*)pErrorBlob->GetBufferPointer());
			pErrorBlob->Release();
		}
		__debugbreak();
	}

	// Define the vertex input layout.
	D3D12_INPUT_ELEMENT_DESC tGeoInputElementDescs[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "SIZE", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 16, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
		{ "SIZE", 1, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
	};

	// Describe and create the graphics pipeline state object (PSO).
	D3D12_GRAPHICS_PIPELINE_STATE_DESC tPsoDesc = {};
	tPsoDesc.InputLayout = { tGeoInputElementDescs, _countof(tGeoInputElementDescs) };
	tPsoDesc.pRootSignature = sm_pRootSignature;
	tPsoDesc.VS = CD3DX12_SHADER_BYTECODE(pBillboardVS->GetBufferPointer(), pBillboardVS->GetBufferSize());
	tPsoDesc.GS = CD3DX12_SHADER_BYTECODE(pBillboardGS->GetBufferPointer(), pBillboardGS->GetBufferSize());
	tPsoDesc.PS = CD3DX12_SHADER_BYTECODE(pBillboardPS->GetBufferPointer(), pBillboardPS->GetBufferSize());
	tPsoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	tPsoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	tPsoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	tPsoDesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
	tPsoDesc.DepthStencilState.StencilEnable = FALSE;
	tPsoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
	tPsoDesc.SampleMask = UINT_MAX;
	tPsoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT;
	tPsoDesc.NumRenderTargets = 1;
	tPsoDesc.RTVFormats[0] = _pRenderer->GetFloatRTVFormat();
	tPsoDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
	tPsoDesc.SampleDesc.Count = _pRenderer->UseMSAA() ? 4 : 1;
	tPsoDesc.SampleDesc.Quality = _pRenderer->UseMSAA() ? _pRenderer->GetNumQualityLevel() - 1 : 0;
	if (FAILED(pDevice->CreateGraphicsPipelineState(&tPsoDesc, IID_PPV_ARGS(&sm_pBillboardPSO))))
	{
		__debugbreak();
	}

	if (pBillboardPS)
	{
		pBillboardPS->Release();
	}
	if (pBillboardGS)
	{
		pBillboardGS->Release();
	}
	if (pBillboardVS)
	{
		pBillboardVS->Release();
	}

	return AK_TRUE;
}

void FBillboardObjects::DestroyCommonResources()
{
	if (!sm_uInitRefCount)
	{
		return;
	}

	AkU32 uRefCount = --sm_uInitRefCount;
	if (!uRefCount)
	{
		DestroyPipelineState();
		DestroyRootSignature();
	}
}

void FBillboardObjects::DestroyRootSignature()
{
	if (sm_pRootSignature)
	{
		sm_pRootSignature->Release();
		sm_pRootSignature = nullptr;
	}
}

void FBillboardObjects::DestroyPipelineState()
{
	if (sm_pBillboardPSO)
	{
		sm_pBillboardPSO->Release();
		sm_pBillboardPSO = nullptr;
	}
}
