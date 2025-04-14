#include "pch.h"
#include "Cloud.h"
#include "Renderer.h"
#include "DescriptorAllocator.h"
#include "DescriptorPool.h"
#include "ConstantBufferPool.h"

ID3D12RootSignature* FCloudObject::sm_pRootSignature;
ID3D12PipelineState* FCloudObject::sm_pCloudDensityPSO;
ID3D12PipelineState* FCloudObject::sm_pCloudLightPSO;
AkU32 FCloudObject::sm_uInitRefCount;

FCloudObject::FCloudObject()
{
}

FCloudObject::~FCloudObject()
{
	CleanUp();
}

AkBool FCloudObject::Initialize(FRenderer* pRenderer)
{
	AkBool bResult = AK_TRUE;

	_pRenderer = pRenderer;

	bResult = CreateCommonResources();

	return bResult;
}

void FCloudObject::Draw(AkU32 uThreadIndex, ID3D12GraphicsCommandList* pCmdList)
{
	// Draw Density.
	{
		ID3D12Device* pDevice = _pRenderer->GetDevice();
		FDescriptorPool* pDescriptorPool = _pRenderer->GetDescriptorPool(uThreadIndex);
		ID3D12DescriptorHeap* pDescriptorHeap = pDescriptorPool->GetDescriptorHeap();
		FConstantBufferPool* pVolumeCloudCBPool = _pRenderer->GetConstantBufferPool(uThreadIndex, CONSTANT_BUFFER_TYPE::CONSTANT_BUFFER_TYPE_VOLUME_CLOUD);
		AkU32 uDescriptorSize = pDescriptorPool->GetDescriptorTypeSize();

		CD3DX12_CPU_DESCRIPTOR_HANDLE hCPU = {};
		CD3DX12_GPU_DESCRIPTOR_HANDLE hGPU = {};
		AkU32 uRequiredDescriptorNum = DESCRIPTOR_COUNT_PER_OBJ;

		if (!pDescriptorPool->AllocDescriptorTable(&hCPU, &hGPU, uRequiredDescriptorNum))
		{
			__debugbreak();
			return;
		}

		// Per Obj (u0).
		CD3DX12_CPU_DESCRIPTOR_HANDLE hDest(hCPU, 0, uDescriptorSize);
		pDevice->CopyDescriptorsSimple(1, hDest, _hDensityUAVCpu, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		hDest.Offset(1, uDescriptorSize);

		// Per Obj (b0).
		CBContainer_t* pVolumeCloudCBContainer = pVolumeCloudCBPool->Alloc();
		if (!pVolumeCloudCBContainer)
		{
			__debugbreak();
			return;
		}

		VolumeCloudConstantBuffer_t* pVolumeCloudConstantBuffer = reinterpret_cast<VolumeCloudConstantBuffer_t*>(pVolumeCloudCBContainer->pSystemMemAddr);
		pVolumeCloudConstantBuffer->vUVWoffset = Vector3(0.0f);
		pVolumeCloudConstantBuffer->fLightAbsorptionCoeff = 5.0f;
		pVolumeCloudConstantBuffer->vLightDir = Vector3(0.0f, 1.0f, 0.0f);
		pVolumeCloudConstantBuffer->fDensityAbsorption = 10.0f;
		pVolumeCloudConstantBuffer->vLightColor = Vector3(1.0f, 1.0f, 1.0f) * 40.0f;
		pVolumeCloudConstantBuffer->fAniso = 0.3f;

		pDevice->CopyDescriptorsSimple(1, hDest, pVolumeCloudCBContainer->hCPU, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		hDest.Offset(1, uDescriptorSize);

		pCmdList->SetComputeRootSignature(sm_pRootSignature);
		pCmdList->SetDescriptorHeaps(1, &pDescriptorHeap);
		pCmdList->SetPipelineState(sm_pCloudDensityPSO);
		pCmdList->SetComputeRootDescriptorTable(0, hGPU);
		pCmdList->Dispatch((UINT)ceil(_uVolumeWidth / 16.0f), (UINT)ceil(_uVolumeHeight / 16.0f), (UINT)ceil(_uVolumeDepth / 4.0f));
	}

	//// Draw Light Map.
	//{
	//	pCmdList->SetComputeRootSignature(sm_pRootSignature);
	//	pCmdList->SetPipelineState(sm_pCloudDensityPSO);
	//	pCmdList->Dispatch((UINT)ceil(_uVolumeWidth / 16.0f), (UINT)ceil(_uVolumeHeight) / 16.0f, (UINT)ceil(_uVolumeDepth / 4.0f));
	//}

	// Draw Volume.
	{
		
	}
}

HRESULT __stdcall FCloudObject::QueryInterface(REFIID riid, void** ppvObject)
{
	return E_NOTIMPL;
}

ULONG __stdcall FCloudObject::AddRef(void)
{
	AkU32 uRefCount = ++_uRefCount;
	return uRefCount;
}

ULONG __stdcall FCloudObject::Release(void)
{
	AkU32 uRefCount = --_uRefCount;
	if (!uRefCount)
	{
		delete this;
	}
	return uRefCount;
}

AkBool FCloudObject::CreateCommonResources()
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
	if (!CreateBuffers())
	{
		__debugbreak();
		return AK_FALSE;
	}

	sm_uInitRefCount++;

	return AK_TRUE;
}

AkBool FCloudObject::CreateRootSignature()
{
	ID3D12Device* pDevice = _pRenderer->GetDevice();
	ID3DBlob* pSignature = nullptr;
	ID3DBlob* pError = nullptr;

	CD3DX12_DESCRIPTOR_RANGE tRangesPerObj[2] = {};
	tRangesPerObj[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0);	// u0 : density tex.
	tRangesPerObj[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0);	// b0 : constant buffer.

	CD3DX12_ROOT_PARAMETER tRootParameters[1] = {};
	tRootParameters[0].InitAsDescriptorTable(_countof(tRangesPerObj), tRangesPerObj, D3D12_SHADER_VISIBILITY_ALL);

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

	// Create root signature.
	CD3DX12_ROOT_SIGNATURE_DESC tRootSignatureDesc;
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

AkBool FCloudObject::CreatePipelineState()
{
	ID3D12Device* pDevice = _pRenderer->GetDevice();

	ID3DBlob* pCloudDensityCS = nullptr;
	ID3DBlob* pCloudLightCS = nullptr;
	ID3DBlob* pVolumePS = nullptr;

#if defined(_DEBUG)
	// Enable better shader debugging with the graphics debugging tools.
	AkU32 uCompileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
	AkU32 uCompileFlags = 0;
#endif

	ID3DBlob* pErrorBlob = nullptr;
	if (FAILED(D3DCompileFromFile(L"../../shader/CloudDensity.hlsl", nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "CSMain", "cs_5_0", uCompileFlags, 0, &pCloudDensityCS, &pErrorBlob)))
	{
		if (pErrorBlob != nullptr)
		{
			OutputDebugStringA((char*)pErrorBlob->GetBufferPointer());
			pErrorBlob->Release();
		}
		__debugbreak();
	}
	//if (FAILED(D3DCompileFromFile(L"../../shader/CloudLight.hlsl", nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "CSMain", "ps_5_0", uCompileFlags, 0, &pCloudLightCS, &pErrorBlob)))
	//{
	//	if (pErrorBlob != nullptr)
	//	{
	//		OutputDebugStringA((char*)pErrorBlob->GetBufferPointer());
	//		pErrorBlob->Release();
	//	}
	//	__debugbreak();
	//}


	// Create cloud density pso.
	{
		D3D12_COMPUTE_PIPELINE_STATE_DESC tPsoDesc = {};
		tPsoDesc.pRootSignature = sm_pRootSignature;
		tPsoDesc.CS = CD3DX12_SHADER_BYTECODE(pCloudDensityCS->GetBufferPointer(), pCloudDensityCS->GetBufferSize());
		tPsoDesc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;
		if (FAILED(pDevice->CreateComputePipelineState(&tPsoDesc, IID_PPV_ARGS(&sm_pCloudDensityPSO))))
		{
			__debugbreak();
		}
	}

	//// Create cloud light pso.
	//{
	//	D3D12_COMPUTE_PIPELINE_STATE_DESC tPsoDesc = {};
	//	tPsoDesc.pRootSignature = sm_pRootSignature;
	//	tPsoDesc.CS = CD3DX12_SHADER_BYTECODE(pCloudLightCS->GetBufferPointer(), pCloudLightCS->GetBufferSize());
	//	tPsoDesc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;
	//	if (FAILED(pDevice->CreateComputePipelineState(&tPsoDesc, IID_PPV_ARGS(&sm_pCloudLightPSO))))
	//	{
	//		__debugbreak();
	//	}
	//}

	//// Define the vertex input layout.
	//D3D12_INPUT_ELEMENT_DESC tInputElementDescs[] =
	//{
	//	{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
	//	{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
	//	{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,	0, 24,	D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
	//	{ "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 32, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
	//};

	//// Describe and create the graphics pipeline state object (PSO).
	//D3D12_GRAPHICS_PIPELINE_STATE_DESC tPsoDesc = {};
	//tPsoDesc.InputLayout = { tInputElementDescs, _countof(tInputElementDescs) };
	//tPsoDesc.pRootSignature = sm_pRootSignature;
	//tPsoDesc.VS = CD3DX12_SHADER_BYTECODE(pCloudDensityCS->GetBufferPointer(), pCloudDensityCS->GetBufferSize());
	//tPsoDesc.PS = CD3DX12_SHADER_BYTECODE(pCloudLightCS->GetBufferPointer(), pCloudLightCS->GetBufferSize());
	//tPsoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	//tPsoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	//tPsoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	//tPsoDesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
	//tPsoDesc.DepthStencilState.StencilEnable = FALSE;
	//tPsoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
	////tPsoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_BACK;
	//tPsoDesc.SampleMask = UINT_MAX;
	//tPsoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	//tPsoDesc.NumRenderTargets = 1;
	//tPsoDesc.RTVFormats[0] = _pRenderer->GetFloatRTVFormat();
	//tPsoDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
	//tPsoDesc.SampleDesc.Count = _pRenderer->UseMSAA() ? 4 : 1;
	//tPsoDesc.SampleDesc.Quality = _pRenderer->UseMSAA() ? _pRenderer->GetNumQualityLevel() - 1 : 0;
	//if (FAILED(pDevice->CreateGraphicsPipelineState(&tPsoDesc, IID_PPV_ARGS(&sm_pOceanPSO))))
	//{
	//	__debugbreak();
	//}

	if (pCloudLightCS)
	{
		pCloudLightCS->Release();
		pCloudLightCS = nullptr;
	}
	if (pCloudDensityCS)
	{
		pCloudDensityCS->Release();
		pCloudDensityCS = nullptr;
	}

	return AK_TRUE;
}

AkBool FCloudObject::CreateBuffers()
{
	ID3D12Device* pDevice = _pRenderer->GetDevice();
	FDescriptorAllocator* pDescriptorAllocator = _pRenderer->GetDescriptorAllocator();

	// 육면체를 생성한다.

	// Density Volume Texture 에 대한 SRV, UAV 생성
	{
		D3D12_RESOURCE_DESC tTex3dDesc = CD3DX12_RESOURCE_DESC::Tex3D(_tFormat, _uVolumeWidth, _uVolumeHeight, _uVolumeDepth, 1);
		tTex3dDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;

		HRESULT hResult = pDevice->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
			D3D12_HEAP_FLAG_NONE,
			&tTex3dDesc,
			D3D12_RESOURCE_STATE_COMMON,
			nullptr,
			IID_PPV_ARGS(&_pDensityMap)
		);
		if (FAILED(hResult))
		{
			__debugbreak();
		}
		
		// Create SRV.
		D3D12_CPU_DESCRIPTOR_HANDLE hSRV = {};
		if (pDescriptorAllocator->AllocDescriptorHandle(&hSRV))
		{
			D3D12_SHADER_RESOURCE_VIEW_DESC tSrvDesc = {};
			tSrvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
			tSrvDesc.Format = _tFormat;
			tSrvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE3D;
			tSrvDesc.Texture3D.MipLevels = 1;

			_hDensitySRVCpu = hSRV;
			pDevice->CreateShaderResourceView(_pDensityMap, &tSrvDesc, _hDensitySRVCpu);
		}
		else
		{
			__debugbreak();
		}

		// Create UAV.
		D3D12_CPU_DESCRIPTOR_HANDLE hUAV = {};
		if (pDescriptorAllocator->AllocDescriptorHandle(&hUAV))
		{
			D3D12_UNORDERED_ACCESS_VIEW_DESC tUAVDesc = {};
			tUAVDesc.Format = _tFormat;
			tUAVDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE3D;
			tUAVDesc.Texture3D.MipSlice = 0;
			tUAVDesc.Texture3D.FirstWSlice = 0;
			tUAVDesc.Texture3D.WSize = -1;

			_hDensityUAVCpu = hUAV;
			pDevice->CreateUnorderedAccessView(_pDensityMap, nullptr, &tUAVDesc, _hDensityUAVCpu);
		}
		else
		{
			__debugbreak();
		}
	}



	return AK_TRUE;
}

void FCloudObject::DestroyCommonResources()
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
		DestroyBuffers();
	}
}

void FCloudObject::DestroyRootSignature()
{
	if (sm_pRootSignature)
	{
		sm_pRootSignature->Release();
		sm_pRootSignature = nullptr;
	}
}

void FCloudObject::DestroyPipelineState()
{
	if (sm_pCloudDensityPSO)
	{
		sm_pCloudDensityPSO->Release();
		sm_pCloudDensityPSO = nullptr;
	}
	if (sm_pCloudLightPSO)
	{
		sm_pCloudLightPSO->Release();
		sm_pCloudLightPSO = nullptr;
	}
}

void FCloudObject::DestroyBuffers()
{
	FDescriptorAllocator* pDescriptorAllocator = _pRenderer->GetDescriptorAllocator();

	if (_pDensityMap)
	{
		_pDensityMap->Release();
		_pDensityMap = nullptr;
	}
	if (_pLightMap)
	{
		_pLightMap->Release();
		_pLightMap = nullptr;
	}

	if(_hDensitySRVCpu.ptr)
	{
		pDescriptorAllocator->FreeDescriptorHandle(_hDensitySRVCpu);
		_hDensitySRVCpu = {};
	}
	if(_hDensityUAVCpu.ptr)
	{
		pDescriptorAllocator->FreeDescriptorHandle(_hDensityUAVCpu);
		_hDensityUAVCpu = {};
	}	
	if (_hLightSRVCpu.ptr)
	{
		pDescriptorAllocator->FreeDescriptorHandle(_hLightSRVCpu);
		_hLightSRVCpu = {};
	}
	if (_hLightUAVCpu.ptr)
	{
		pDescriptorAllocator->FreeDescriptorHandle(_hLightUAVCpu);
		_hLightUAVCpu = {};
	}
}

void FCloudObject::CleanUp()
{
	_pRenderer->EnsureCompleted();

	DestroyCommonResources();
}
