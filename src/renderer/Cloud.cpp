#include "pch.h"
#include "Cloud.h"
#include "Renderer.h"
#include "DescriptorAllocator.h"
#include "DescriptorPool.h"
#include "ConstantBufferPool.h"
#include "ResourceManager.h"

ID3D12RootSignature* FCloudObject::sm_pCloudDensityRS;
ID3D12RootSignature* FCloudObject::sm_pCloudLightRS;
ID3D12RootSignature* FCloudObject::sm_pVolumeRS;
ID3D12PipelineState* FCloudObject::sm_pCloudDensityPSO;
ID3D12PipelineState* FCloudObject::sm_pCloudLightPSO;
ID3D12PipelineState* FCloudObject::sm_pVolumePSO;
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
	ID3D12Device* pDevice = _pRenderer->GetDevice();

	// Draw Density.
	{
		FDescriptorPool* pDescriptorPool = _pRenderer->GetDescriptorPool(uThreadIndex);
		ID3D12DescriptorHeap* pDescriptorHeap = pDescriptorPool->GetDescriptorHeap();
		FConstantBufferPool* pVolumeCloudCBPool = _pRenderer->GetConstantBufferPool(uThreadIndex, CONSTANT_BUFFER_TYPE::CONSTANT_BUFFER_TYPE_VOLUME_CLOUD);
		AkU32 uDescriptorSize = pDescriptorPool->GetDescriptorTypeSize();

		CD3DX12_CPU_DESCRIPTOR_HANDLE hCPU = {};
		CD3DX12_GPU_DESCRIPTOR_HANDLE hGPU = {};
		AkU32 uRequiredDescriptorNum = DESCRIPTOR_COUNT_DENSITY_MAP;

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

		static AkF32 fOffset = 0.0f;

		fOffset += 0.001f;

		VolumeCloudConstantBuffer_t* pVolumeCloudConstantBuffer = reinterpret_cast<VolumeCloudConstantBuffer_t*>(pVolumeCloudCBContainer->pSystemMemAddr);
		pVolumeCloudConstantBuffer->vUVWoffset.z = fOffset;
		pVolumeCloudConstantBuffer->fLightAbsorptionCoeff = 5.0f;
		pVolumeCloudConstantBuffer->vLightDir = Vector3(0.0f, 1.0f, 0.0f);
		pVolumeCloudConstantBuffer->fDensityAbsorption = 10.0f;
		pVolumeCloudConstantBuffer->vLightColor = Vector3(1.0f, 1.0f, 1.0f) * 40.0f;
		pVolumeCloudConstantBuffer->fAniso = 0.3f;

		pDevice->CopyDescriptorsSimple(1, hDest, pVolumeCloudCBContainer->hCPU, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		hDest.Offset(1, uDescriptorSize);

		pCmdList->SetComputeRootSignature(sm_pCloudDensityRS);
		pCmdList->SetDescriptorHeaps(1, &pDescriptorHeap);
		pCmdList->SetPipelineState(sm_pCloudDensityPSO);
		pCmdList->SetComputeRootDescriptorTable(0, hGPU);
		pCmdList->Dispatch((UINT)ceil(_uVolumeWidth / 16.0f), (UINT)ceil(_uVolumeHeight / 16.0f), (UINT)ceil(_uVolumeDepth / 4.0f));
	}

	// Draw Light Map.
	{
		FDescriptorPool* pDescriptorPool = _pRenderer->GetDescriptorPool(uThreadIndex);
		ID3D12DescriptorHeap* pDescriptorHeap = pDescriptorPool->GetDescriptorHeap();
		FConstantBufferPool* pVolumeCloudCBPool = _pRenderer->GetConstantBufferPool(uThreadIndex, CONSTANT_BUFFER_TYPE::CONSTANT_BUFFER_TYPE_VOLUME_CLOUD);
		AkU32 uDescriptorSize = pDescriptorPool->GetDescriptorTypeSize();

		CD3DX12_CPU_DESCRIPTOR_HANDLE hCPU = {};
		CD3DX12_GPU_DESCRIPTOR_HANDLE hGPU = {};
		AkU32 uRequiredDescriptorNum = DESCRIPTOR_COUNT_LIGHT_MAP;

		if (!pDescriptorPool->AllocDescriptorTable(&hCPU, &hGPU, uRequiredDescriptorNum))
		{
			__debugbreak();
			return;
		}

		// Per Obj (t0).
		CD3DX12_CPU_DESCRIPTOR_HANDLE hDest(hCPU, 0, uDescriptorSize);
		pDevice->CopyDescriptorsSimple(1, hDest, _hDensitySRVCpu, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		hDest.Offset(1, uDescriptorSize);

		// Per Obj (u0).
		pDevice->CopyDescriptorsSimple(1, hDest, _hLightUAVCpu, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
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

		pCmdList->SetComputeRootSignature(sm_pCloudLightRS);
		pCmdList->SetDescriptorHeaps(1, &pDescriptorHeap);
		pCmdList->SetPipelineState(sm_pCloudLightPSO);
		pCmdList->SetComputeRootDescriptorTable(0, hGPU);
		pCmdList->Dispatch((UINT)ceil(_uLightWidth / 16.0f), (UINT)ceil(_uLightHeight / 16.0f), (UINT)ceil(_uLightDepth / 4.0f));
	}

	// Draw Volume.
	{
		FDescriptorPool* pDescriptorPool = _pRenderer->GetDescriptorPool(uThreadIndex);
		ID3D12DescriptorHeap* pDescriptorHeap = pDescriptorPool->GetDescriptorHeap();
		FConstantBufferPool* pGlobalCBPool = _pRenderer->GetConstantBufferPool(uThreadIndex, CONSTANT_BUFFER_TYPE::CONSTANT_BUFFER_TYPE_GLOBAL);
		FConstantBufferPool* pMeshCBPool = _pRenderer->GetConstantBufferPool(uThreadIndex, CONSTANT_BUFFER_TYPE::CONSTANT_BUFFER_TYPE_MESH);
		FConstantBufferPool* pVolumeCloudCBPool = _pRenderer->GetConstantBufferPool(uThreadIndex, CONSTANT_BUFFER_TYPE::CONSTANT_BUFFER_TYPE_VOLUME_CLOUD);
		AkU32 uDescriptorSize = pDescriptorPool->GetDescriptorTypeSize();

		CD3DX12_CPU_DESCRIPTOR_HANDLE hCPU = {};
		CD3DX12_GPU_DESCRIPTOR_HANDLE hGPU = {};
		AkU32 uRequiredDescriptorNum = DESCRIPTOR_COUNT_VOLUME;

		if (!pDescriptorPool->AllocDescriptorTable(&hCPU, &hGPU, uRequiredDescriptorNum))
		{
			__debugbreak();
			return;
		}

		// t0 (density)
		CD3DX12_CPU_DESCRIPTOR_HANDLE hDest(hCPU, 0, uDescriptorSize);
		pDevice->CopyDescriptorsSimple(1, hDest, _hDensitySRVCpu, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		hDest.Offset(1, uDescriptorSize);

		// t1 (light map)
		pDevice->CopyDescriptorsSimple(1, hDest, _hLightSRVCpu, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		hDest.Offset(1, uDescriptorSize);

		// t2 (Null)
		pDevice->CopyDescriptorsSimple(1, hDest, _hNullCpu, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		hDest.Offset(1, uDescriptorSize);

		// b0
		CBContainer_t* pGlobalCBContainer = pGlobalCBPool->Alloc();
		if (!pGlobalCBContainer)
		{
			__debugbreak();
			return;
		}

		GlobalConstantBuffer_t* pGlobalConstantBuffer = reinterpret_cast<GlobalConstantBuffer_t*>(pGlobalCBContainer->pSystemMemAddr);
		_pRenderer->GetViewPorjMatrix(&pGlobalConstantBuffer->mView, &pGlobalConstantBuffer->mProj);
		_pRenderer->GetCameraPosition(&pGlobalConstantBuffer->vEyeWorld.x, &pGlobalConstantBuffer->vEyeWorld.y, &pGlobalConstantBuffer->vEyeWorld.z);
		
		pDevice->CopyDescriptorsSimple(1, hDest, pGlobalCBContainer->hCPU, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		hDest.Offset(1, uDescriptorSize);

		// b1
		CBContainer_t* pMeshCBContainer = pMeshCBPool->Alloc();
		if (!pMeshCBContainer)
		{
			__debugbreak();
			return;
		}

		MeshConstantBuffer_t* pMeshConstantBuffer = reinterpret_cast<MeshConstantBuffer_t*>(pMeshCBContainer->pSystemMemAddr);
		pMeshConstantBuffer->mWorld = (Matrix::CreateScale(1.5f) * Matrix::CreateTranslation(Vector3(0.0f, 5.0f, 0.0f))).Transpose(); // Temp
		pMeshConstantBuffer->mWorldInv = pMeshConstantBuffer->mWorld.Invert(); // Temp
		
		pDevice->CopyDescriptorsSimple(1, hDest, pMeshCBContainer->hCPU, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		hDest.Offset(1, uDescriptorSize);

		// b2
		CBContainer_t* pVolumeCloudCBContainer = pVolumeCloudCBPool->Alloc();
		if (!pVolumeCloudCBContainer)
		{
			__debugbreak();
			return;
		}

		VolumeCloudConstantBuffer_t* pVolumeCloudConstantBuffer = reinterpret_cast<VolumeCloudConstantBuffer_t*>(pVolumeCloudCBContainer->pSystemMemAddr);
		pVolumeCloudConstantBuffer->fLightAbsorptionCoeff = 5.0f;
		pVolumeCloudConstantBuffer->vLightDir = Vector3(0.0f, 1.0f, 0.0f);
		pVolumeCloudConstantBuffer->fDensityAbsorption = 10.0f;
		pVolumeCloudConstantBuffer->vLightColor = Vector3(1.0f, 1.0f, 1.0f) * 40.0f;
		pVolumeCloudConstantBuffer->fAniso = 0.3f;

		pDevice->CopyDescriptorsSimple(1, hDest, pVolumeCloudCBContainer->hCPU, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		hDest.Offset(1, uDescriptorSize);

		pCmdList->SetGraphicsRootSignature(sm_pVolumeRS);
		pCmdList->SetDescriptorHeaps(1, &pDescriptorHeap);
		pCmdList->SetPipelineState(sm_pVolumePSO);
		pCmdList->SetGraphicsRootDescriptorTable(0, hGPU);
		pCmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		pCmdList->IASetVertexBuffers(0, 1, &_pMeshes->tVBView);
		pCmdList->IASetIndexBuffer(&_pMeshes->tIBView);
		pCmdList->DrawIndexedInstanced(_pMeshes->uIndexCountPerInstance, 1, 0, 0, 0);
	}
}

AkBool FCloudObject::CreateMeshBuffers(MeshData_t* pMeshData, AkU32 uMeshDataNum)
{
	FResourceManager* pResourceManager = _pRenderer->GetResourceManager();
	D3D12_VERTEX_BUFFER_VIEW tVBView = {};
	D3D12_INDEX_BUFFER_VIEW tIBView = {};
	ID3D12Resource* pVertexBuffer = nullptr;
	ID3D12Resource* pIndexBuffer = nullptr;

	// 육면체 매쉬 생성.
	_pMeshes = reinterpret_cast<Mesh_t*>(malloc(sizeof(Mesh_t) * uMeshDataNum));
	_uMeshNum = uMeshDataNum;

	if (1 != _uMeshNum)
	{
		__debugbreak();
		return AK_FALSE;
	}

	for (AkU32 i = 0; i < _uMeshNum; i++)
	{
		if (pResourceManager->CreateVertexBuffer(sizeof(Vertex_t), pMeshData[i].uVerticeNum, &tVBView, &pVertexBuffer, pMeshData[i].pVertices))
		{
			_pMeshes[i].pVB = pVertexBuffer;
			_pMeshes[i].tVBView = tVBView;
		}

		if (pResourceManager->CreateIndexBuffer(pMeshData[i].uIndicesNum, &tIBView, &pIndexBuffer, pMeshData[i].pIndices))
		{
			_pMeshes[i].pIB = pIndexBuffer;
			_pMeshes[i].tIBView = tIBView;
			_pMeshes[i].uVertexCountPerInstance = pMeshData[i].uVerticeNum;
			_pMeshes[i].uIndexCountPerInstance = pMeshData[i].uIndicesNum;
		}
	}

	return AK_TRUE;
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
	if (!CreateDensityAndLightMap())
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

	// Create cloud density root signature.
	{
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

		if (FAILED(pDevice->CreateRootSignature(0, pSignature->GetBufferPointer(), pSignature->GetBufferSize(), IID_PPV_ARGS(&sm_pCloudDensityRS))))
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
	}

	// Create cloud light root signature
	{
		CD3DX12_DESCRIPTOR_RANGE tRangesPerObj[3] = {};
		tRangesPerObj[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);	// t0 : density tex
		tRangesPerObj[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0);	// u0 : light tex
		tRangesPerObj[2].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0);	// b0 : constant buffer.

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

		if (FAILED(pDevice->CreateRootSignature(0, pSignature->GetBufferPointer(), pSignature->GetBufferSize(), IID_PPV_ARGS(&sm_pCloudLightRS))))
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
	}

	// Create volume root signature.
	{
		CD3DX12_DESCRIPTOR_RANGE tRangesPerObj[2] = {};
		tRangesPerObj[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 3, 0);	// t0 : density tex, t1 : light tex, t2 : temperature tex
		tRangesPerObj[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 3, 0);	// b0 : gloabl, b1 : mesh, b2 : consts

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

		if (FAILED(pDevice->CreateRootSignature(0, pSignature->GetBufferPointer(), pSignature->GetBufferSize(), IID_PPV_ARGS(&sm_pVolumeRS))))
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
	}

	return AK_TRUE;
}

AkBool FCloudObject::CreatePipelineState()
{
	ID3D12Device* pDevice = _pRenderer->GetDevice();

	ID3DBlob* pCloudDensityCS = nullptr;
	ID3DBlob* pCloudLightCS = nullptr;
	ID3DBlob* pVolumeVS = nullptr;
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
	if (FAILED(D3DCompileFromFile(L"../../shader/CloudLight.hlsl", nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "CSMain", "cs_5_0", uCompileFlags, 0, &pCloudLightCS, &pErrorBlob)))
	{
		if (pErrorBlob != nullptr)
		{
			OutputDebugStringA((char*)pErrorBlob->GetBufferPointer());
			pErrorBlob->Release();
		}
		__debugbreak();
	}
	if (FAILED(D3DCompileFromFile(L"../../shader/VolumeSmoke.hlsl", nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "VSMain", "vs_5_0", uCompileFlags, 0, &pVolumeVS, &pErrorBlob)))
	{
		if (pErrorBlob != nullptr)
		{
			OutputDebugStringA((char*)pErrorBlob->GetBufferPointer());
			pErrorBlob->Release();
		}
		__debugbreak();
	}
	if (FAILED(D3DCompileFromFile(L"../../shader/VolumeSmoke.hlsl", nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "PSMain", "ps_5_0", uCompileFlags, 0, &pVolumePS, &pErrorBlob)))
	{
		if (pErrorBlob != nullptr)
		{
			OutputDebugStringA((char*)pErrorBlob->GetBufferPointer());
			pErrorBlob->Release();
		}
		__debugbreak();
	}

	// Create cloud density pso.
	{
		D3D12_COMPUTE_PIPELINE_STATE_DESC tPsoDesc = {};
		tPsoDesc.pRootSignature = sm_pCloudDensityRS;
		tPsoDesc.CS = CD3DX12_SHADER_BYTECODE(pCloudDensityCS->GetBufferPointer(), pCloudDensityCS->GetBufferSize());
		tPsoDesc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;
		if (FAILED(pDevice->CreateComputePipelineState(&tPsoDesc, IID_PPV_ARGS(&sm_pCloudDensityPSO))))
		{
			__debugbreak();
		}
	}

	// Create cloud light pso.
	{
		D3D12_COMPUTE_PIPELINE_STATE_DESC tPsoDesc = {};
		tPsoDesc.pRootSignature = sm_pCloudLightRS;
		tPsoDesc.CS = CD3DX12_SHADER_BYTECODE(pCloudLightCS->GetBufferPointer(), pCloudLightCS->GetBufferSize());
		tPsoDesc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;
		if (FAILED(pDevice->CreateComputePipelineState(&tPsoDesc, IID_PPV_ARGS(&sm_pCloudLightPSO))))
		{
			__debugbreak();
		}
	}

	// Define the vertex input layout.
	D3D12_INPUT_ELEMENT_DESC tInputElementDescs[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,	0, 24,	D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 32, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
	};

	// Describe and create the graphics pipeline state object (PSO).
	D3D12_GRAPHICS_PIPELINE_STATE_DESC tPsoDesc = {};
	tPsoDesc.InputLayout = { tInputElementDescs, _countof(tInputElementDescs) };
	tPsoDesc.pRootSignature = sm_pVolumeRS;
	tPsoDesc.VS = CD3DX12_SHADER_BYTECODE(pVolumeVS->GetBufferPointer(), pVolumeVS->GetBufferSize());
	tPsoDesc.PS = CD3DX12_SHADER_BYTECODE(pVolumePS->GetBufferPointer(), pVolumePS->GetBufferSize());
	tPsoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	tPsoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	tPsoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	tPsoDesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
	tPsoDesc.DepthStencilState.StencilEnable = FALSE;
	tPsoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_BACK;
	tPsoDesc.SampleMask = UINT_MAX;
	tPsoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	tPsoDesc.NumRenderTargets = 1;
	tPsoDesc.RTVFormats[0] = _pRenderer->GetFloatRTVFormat();
	tPsoDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
	tPsoDesc.SampleDesc.Count = _pRenderer->UseMSAA() ? 4 : 1;
	tPsoDesc.SampleDesc.Quality = _pRenderer->UseMSAA() ? _pRenderer->GetNumQualityLevel() - 1 : 0;
	if (FAILED(pDevice->CreateGraphicsPipelineState(&tPsoDesc, IID_PPV_ARGS(&sm_pVolumePSO))))
	{
		__debugbreak();
	}

	if (pVolumePS)
	{
		pVolumePS->Release();
		pVolumePS = nullptr;
	}
	if (pVolumeVS)
	{
		pVolumeVS->Release();
		pVolumeVS = nullptr;
	}
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

AkBool FCloudObject::CreateDensityAndLightMap()
{
	ID3D12Device* pDevice = _pRenderer->GetDevice();
	FDescriptorAllocator* pDescriptorAllocator = _pRenderer->GetDescriptorAllocator();

	// Density Map 에 대한 SRV, UAV 생성
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

	// Light Map 에 대한 SRV, UAV 생성.
	{
		D3D12_RESOURCE_DESC tTex3dDesc = CD3DX12_RESOURCE_DESC::Tex3D(_tFormat, _uLightWidth, _uLightHeight, _uLightDepth, 1);
		tTex3dDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;

		HRESULT hResult = pDevice->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
			D3D12_HEAP_FLAG_NONE,
			&tTex3dDesc,
			D3D12_RESOURCE_STATE_COMMON,
			nullptr,
			IID_PPV_ARGS(&_pLightMap)
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

			_hLightSRVCpu = hSRV;
			pDevice->CreateShaderResourceView(_pLightMap, &tSrvDesc, _hLightSRVCpu);
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

			_hLightUAVCpu = hUAV;
			pDevice->CreateUnorderedAccessView(_pLightMap, nullptr, &tUAVDesc, _hLightUAVCpu);
		}
		else
		{
			__debugbreak();
		}
	}

	// Null SRV
	{
		// Create SRV.
		D3D12_CPU_DESCRIPTOR_HANDLE hSRV = {};
		if (pDescriptorAllocator->AllocDescriptorHandle(&hSRV))
		{
			D3D12_SHADER_RESOURCE_VIEW_DESC tSrvDesc = {};
			tSrvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
			tSrvDesc.Format = _tFormat;
			tSrvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE3D;
			tSrvDesc.Texture3D.MipLevels = 1;

			_hNullCpu = hSRV;
			pDevice->CreateShaderResourceView(nullptr, &tSrvDesc, _hNullCpu);
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
		DestroyDensityAndLightMap();
	}
}

void FCloudObject::DestroyRootSignature()
{
	if (sm_pVolumeRS)
	{
		sm_pVolumeRS->Release();
		sm_pVolumeRS = nullptr;
	}
	if (sm_pCloudLightRS)
	{
		sm_pCloudLightRS->Release();
		sm_pCloudLightRS = nullptr;
	}
	if (sm_pCloudDensityRS)
	{
		sm_pCloudDensityRS->Release();
		sm_pCloudDensityRS = nullptr;
	}
}

void FCloudObject::DestroyPipelineState()
{
	if (sm_pVolumePSO)
	{
		sm_pVolumePSO->Release();
		sm_pVolumePSO = nullptr;
	}
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

void FCloudObject::DestroyDensityAndLightMap()
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

	if (_hNullCpu.ptr)
	{
		pDescriptorAllocator->FreeDescriptorHandle(_hNullCpu);
		_hNullCpu = {};
	}
	if (_hDensitySRVCpu.ptr)
	{
		pDescriptorAllocator->FreeDescriptorHandle(_hDensitySRVCpu);
		_hDensitySRVCpu = {};
	}
	if (_hDensityUAVCpu.ptr)
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

	if (_pMeshes)
	{
		if (_pMeshes->pVB)
		{
			_pMeshes->pVB->Release();
			_pMeshes->pVB = nullptr;
		}
		if (_pMeshes->pIB)
		{
			_pMeshes->pIB->Release();
			_pMeshes->pIB = nullptr;
		}

		free(_pMeshes);
		_pMeshes = nullptr;
	}
}
