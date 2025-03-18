#include "pch.h"
#include "Particle.h"
#include "Renderer.h"
#include "TextureManager.h"
#include "ResourceManager.h"
#include "DescriptorAllocator.h"
#include "DescriptorPool.h"
#include "ConstantBufferManager.h"
#include "ConstantBufferPool.h"

/*
==============
Particle
==============
*/

ID3D12RootSignature* FParticle::sm_pSparkRS;
ID3D12RootSignature* FParticle::sm_pSpriteRS;
ID3D12PipelineState* FParticle::sm_pSparkPSO;
ID3D12PipelineState* FParticle::sm_pSpritePSO;
AkU32 FParticle::sm_uInitRefCount;

FParticle::FParticle()
{
}

FParticle::~FParticle()
{
	CleanUp();
}

AkBool FParticle::Initialize(FRenderer* pRenderer)
{
	AkBool bResult = AK_TRUE;

	_pRenderer = pRenderer;

	bResult = CreateCommonResources();

	return bResult;
}

void* FParticle::CreateParticleSpark(VertexParticle_t* pVertices, AkU32 uNum)
{
	FResourceManager* pResourceManager = _pRenderer->GetResourceManager();
	FDescriptorAllocator* pDescriptorAllocator = _pRenderer->GetDescriptorAllocator();
	ID3D12Resource* pUploadBuffer = nullptr;
	DynamicDefaultBufferHandle_t* pDynamicDefaultBufferHandle = new DynamicDefaultBufferHandle_t;
	D3D12_CPU_DESCRIPTOR_HANDLE hSRV = {};

	AkU64 u64BytesSize = uNum * sizeof(VertexParticle_t);

	if (pResourceManager->CreateDynamicDefaultBuffer(u64BytesSize, &pUploadBuffer))
	{
		if (pDescriptorAllocator->AllocDescriptorHandle(&hSRV))
		{
			pDynamicDefaultBufferHandle->uDataNum = uNum;
			pDynamicDefaultBufferHandle->pUploadBuffer = pUploadBuffer;
			pDynamicDefaultBufferHandle->hSRV = hSRV;
			pDynamicDefaultBufferHandle->uSizePerType = sizeof(VertexParticle_t);
			pDynamicDefaultBufferHandle->bUpdated = AK_FALSE;
		}
	}

	return pDynamicDefaultBufferHandle;
}

void* FParticle::CreateParticleSprite(VertexSize_t* pVerices)
{
	FResourceManager* pResourceManager = _pRenderer->GetResourceManager();
	FDescriptorAllocator* pDescriptorAllocator = _pRenderer->GetDescriptorAllocator();
	ID3D12Resource* pUploadBuffer = nullptr;
	DynamicDefaultBufferHandle_t* pDynamicDefaultBufferHandle = new DynamicDefaultBufferHandle_t;
	D3D12_CPU_DESCRIPTOR_HANDLE hSRV = {};

	AkU64 u64BytesSize = sizeof(VertexSize_t);

	if (pResourceManager->CreateDynamicDefaultBuffer(u64BytesSize, &pUploadBuffer))
	{
		if (pDescriptorAllocator->AllocDescriptorHandle(&hSRV))
		{
			pDynamicDefaultBufferHandle->uDataNum = 1;
			pDynamicDefaultBufferHandle->pUploadBuffer = pUploadBuffer;
			pDynamicDefaultBufferHandle->hSRV = hSRV;
			pDynamicDefaultBufferHandle->uSizePerType = sizeof(VertexSize_t);
			pDynamicDefaultBufferHandle->bUpdated = AK_FALSE;
		}
	}

	return pDynamicDefaultBufferHandle;
}

void FParticle::SetTexture(void* pTexHandle)
{
	_pTextureHandle = (TextureHandle_t*)pTexHandle;
}

void FParticle::DestroyBasicParticleBuffer(void* pDBHandle)
{
	_pRenderer->EnsureCompleted();

	FDescriptorAllocator* pDescriptorAllocator = _pRenderer->GetDescriptorAllocator();

	DynamicDefaultBufferHandle_t* pDynamicDefaultBufferHandle = (DynamicDefaultBufferHandle_t*)pDBHandle;
	if (pDynamicDefaultBufferHandle)
	{
		if (pDynamicDefaultBufferHandle->pUploadBuffer)
		{
			pDynamicDefaultBufferHandle->pUploadBuffer->Release();
			pDynamicDefaultBufferHandle->pUploadBuffer = nullptr;
		}
		if (pDynamicDefaultBufferHandle->hSRV.ptr)
		{
			pDescriptorAllocator->FreeDescriptorHandle(pDynamicDefaultBufferHandle->hSRV);
			pDynamicDefaultBufferHandle->hSRV = {};
		}

		delete pDynamicDefaultBufferHandle;
		pDynamicDefaultBufferHandle = nullptr;
	}
}

HRESULT __stdcall FParticle::QueryInterface(REFIID riid, void** ppvObject)
{
	return E_NOTIMPL;
}

ULONG __stdcall FParticle::AddRef(void)
{
	AkU32 uRefCount = ++_uRefCount;
	return uRefCount;
}

ULONG __stdcall FParticle::Release(void)
{
	AkU32 uRefCount = --_uRefCount;
	if (!uRefCount)
	{
		delete this;
	}
	return uRefCount;
}

void FParticle::Draw(AkU32 uThreadIndex, ID3D12GraphicsCommandList* pCmdList, const Matrix* pWorldRow, DynamicDefaultBufferHandle_t* pDBHandle, AkU32 uParticleNum, AkF32 fTime, AkF32 fDuration, const Vector2* pStartSize, const Vector3* pStartDirection, AkF32 fSizeOverLifeTime, const Vector3* pRotOverLifeTime, const Vector4* pTotalColor, const Vector4* pColorOverLifeTime)
{
	ID3D12Device* pDevice = _pRenderer->GetDevice();
	FDescriptorPool* pDescriptorPool = _pRenderer->GetDescriptorPool(uThreadIndex);
	ID3D12DescriptorHeap* pDescriptorHeap = pDescriptorPool->GetDescriptorHeap();
	FConstantBufferPool* pGlobalCBPool = _pRenderer->GetConstantBufferPool(uThreadIndex, CONSTANT_BUFFER_TYPE::CONSTANT_BUFFER_TYPE_GLOBAL);
	FConstantBufferPool* pMeshCBPool = _pRenderer->GetConstantBufferPool(uThreadIndex, CONSTANT_BUFFER_TYPE::CONSTANT_BUFFER_TYPE_MESH);
	FConstantBufferPool* pParticleSparkCBPool = _pRenderer->GetConstantBufferPool(uThreadIndex, CONSTANT_BUFFER_TYPE::CONSTANT_BUFFER_TYPE_PARTICLE_SPARK);
	FConstantBufferPool* pParticleColorCBPool = _pRenderer->GetConstantBufferPool(uThreadIndex, CONSTANT_BUFFER_TYPE::CONSTANT_BUFFER_TYPE_PARTICLE_COLOR);
	AkU32 uDescriptorSize = pDescriptorPool->GetDescriptorTypeSize();

	CD3DX12_CPU_DESCRIPTOR_HANDLE hCPU = {};
	CD3DX12_GPU_DESCRIPTOR_HANDLE hGPU = {};
	AkU32 uRequiredDescriptorNum = SPARK_DESCRIPTOR_COUNT_PER_OBJ + uParticleNum;

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

	/*Global Consts*/
	GlobalConstantBuffer_t* pGlobalConstantBuffer = reinterpret_cast<GlobalConstantBuffer_t*>(pGlobalCBContainer->pSystemMemAddr);
	_pRenderer->GetViewPorjMatrix(&pGlobalConstantBuffer->mView, &pGlobalConstantBuffer->mProj);
	pGlobalConstantBuffer->mInvView = pGlobalConstantBuffer->mView.Transpose().Invert().Transpose();
	pGlobalConstantBuffer->mInvProj = pGlobalConstantBuffer->mProj.Transpose().Invert().Transpose();

	_pRenderer->GetCameraPosition(&pGlobalConstantBuffer->vEyeWorld.x, &pGlobalConstantBuffer->vEyeWorld.y, &pGlobalConstantBuffer->vEyeWorld.z);
	pGlobalConstantBuffer->fStrengthIBL = _pRenderer->GetIBLStrength();
	// Per Obj (b0).
	CD3DX12_CPU_DESCRIPTOR_HANDLE hDest(hCPU, 0, uDescriptorSize);
	pDevice->CopyDescriptorsSimple(1, hDest, pGlobalCBContainer->hCPU, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	hDest.Offset(1, uDescriptorSize);

	/*Mesh Consts*/
	CBContainer_t* pMeshCBContainer = pMeshCBPool->Alloc();
	if (!pMeshCBContainer)
	{
		__debugbreak();
		return;
	}

	MeshConstantBuffer_t* pMeshConstantBuffer = reinterpret_cast<MeshConstantBuffer_t*>(pMeshCBContainer->pSystemMemAddr); 
	Matrix mWorldRow = (*pWorldRow);
	pMeshConstantBuffer->mWorld = mWorldRow.Transpose();
	mWorldRow.Translation(Vector3(0.0f));
	mWorldRow.Invert().Transpose();
	pMeshConstantBuffer->mWorldIT = mWorldRow.Transpose();
	// Per Obj (b1).
	pDevice->CopyDescriptorsSimple(1, hDest, pMeshCBContainer->hCPU, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	hDest.Offset(1, uDescriptorSize);

	/*Particle Spark Consts*/
	CBContainer_t* pParticleSparkCBContainer = pParticleSparkCBPool->Alloc();
	if (!pParticleSparkCBContainer)
	{
		__debugbreak();
		return;
	}

	ParticleSparkConstantBuffer_t* pParticleSparkConstantBuffer = reinterpret_cast<ParticleSparkConstantBuffer_t*>(pParticleSparkCBContainer->pSystemMemAddr);
	pParticleSparkConstantBuffer->fTime = fTime;
	pParticleSparkConstantBuffer->fDuration = fDuration;
	pParticleSparkConstantBuffer->vStartSize = *pStartSize;
	pParticleSparkConstantBuffer->vStartDirection = *pStartDirection;
	pParticleSparkConstantBuffer->fSizeOverLifeTime = fSizeOverLifeTime;
	pParticleSparkConstantBuffer->vRotOverLifeTime = *pRotOverLifeTime;
	// Per Obj (b2).
	pDevice->CopyDescriptorsSimple(1, hDest, pParticleSparkCBContainer->hCPU, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	hDest.Offset(1, uDescriptorSize);

	/*Particle Color Consts*/
	CBContainer_t* pParticleColorCBContainer = pParticleColorCBPool->Alloc();
	if (!pParticleColorCBContainer)
	{
		__debugbreak();
		return;
	}

	ParticleColorConstantBuffer_t* pParticleColorConstantBuffer = reinterpret_cast<ParticleColorConstantBuffer_t*>(pParticleColorCBContainer->pSystemMemAddr);
	pParticleColorConstantBuffer->vTotalColor = *pTotalColor;
	pParticleColorConstantBuffer->vColorOverLifeTime = *pColorOverLifeTime;
	// Per Obj (b3).
	pDevice->CopyDescriptorsSimple(1, hDest, pParticleColorCBContainer->hCPU, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	hDest.Offset(1, uDescriptorSize);

	// Per Obj (t1).
	for (AkU32 i = 0; i < uParticleNum; i++)
	{
		pDevice->CopyDescriptorsSimple(1, hDest, _pTextureHandle->hSRV, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		hDest.Offset(1, uDescriptorSize);
	}

	pCmdList->SetGraphicsRootSignature(sm_pSparkRS);
	pCmdList->SetDescriptorHeaps(1, &pDescriptorHeap);
	pCmdList->SetPipelineState(sm_pSparkPSO);

	// Obj (root param 0)
	pCmdList->SetGraphicsRootDescriptorTable(0, hGPU);
	pCmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_POINTLIST);

	CD3DX12_GPU_DESCRIPTOR_HANDLE hGPUforMeshes(hGPU, SPARK_DESCRIPTOR_COUNT_PER_OBJ, uDescriptorSize);
	pCmdList->SetGraphicsRootDescriptorTable(1, hGPUforMeshes);

	// 색을 모두 더하면서 그리는 accumulateBS 사용
	const float blendColor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
	pCmdList->OMSetBlendFactor(blendColor);
	pCmdList->SetGraphicsRootShaderResourceView(2, pDBHandle->pUploadBuffer->GetGPUVirtualAddress());
	pCmdList->DrawInstanced(uParticleNum, 1, 0, 0);
}

void FParticle::Draw(AkU32 uThreadIndex, ID3D12GraphicsCommandList* pCmdList, DynamicDefaultBufferHandle_t* pDBHandle, const Vector2* pMaxFrame, const Vector2* pCurFrame)
{
	ID3D12Device* pDevice = _pRenderer->GetDevice();
	FDescriptorPool* pDescriptorPool = _pRenderer->GetDescriptorPool(uThreadIndex);
	ID3D12DescriptorHeap* pDescriptorHeap = pDescriptorPool->GetDescriptorHeap();
	FConstantBufferPool* pGlobalCBPool = _pRenderer->GetConstantBufferPool(uThreadIndex, CONSTANT_BUFFER_TYPE::CONSTANT_BUFFER_TYPE_GLOBAL);
	FConstantBufferPool* pMeshCBPool = _pRenderer->GetConstantBufferPool(uThreadIndex, CONSTANT_BUFFER_TYPE::CONSTANT_BUFFER_TYPE_MESH);
	FConstantBufferPool* pParticleSpriteCBPool = _pRenderer->GetConstantBufferPool(uThreadIndex, CONSTANT_BUFFER_TYPE::CONSTANT_BUFFER_TYPE_PARTICLE_SPRITE);
	AkU32 uDescriptorSize = pDescriptorPool->GetDescriptorTypeSize();

	CD3DX12_CPU_DESCRIPTOR_HANDLE hCPU = {};
	CD3DX12_GPU_DESCRIPTOR_HANDLE hGPU = {};
	AkU32 uRequiredDescriptorNum = SPRITE_DESCRIPTOR_COUNT_PER_OBJ + 1;

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

	/*Global Consts*/
	GlobalConstantBuffer_t* pGlobalConstantBuffer = reinterpret_cast<GlobalConstantBuffer_t*>(pGlobalCBContainer->pSystemMemAddr);
	_pRenderer->GetViewPorjMatrix(&pGlobalConstantBuffer->mView, &pGlobalConstantBuffer->mProj);
	pGlobalConstantBuffer->mInvView = pGlobalConstantBuffer->mView.Transpose().Invert().Transpose();
	pGlobalConstantBuffer->mInvProj = pGlobalConstantBuffer->mProj.Transpose().Invert().Transpose();

	_pRenderer->GetCameraPosition(&pGlobalConstantBuffer->vEyeWorld.x, &pGlobalConstantBuffer->vEyeWorld.y, &pGlobalConstantBuffer->vEyeWorld.z);
	pGlobalConstantBuffer->fStrengthIBL = _pRenderer->GetIBLStrength();
	// Per Obj (b0).
	CD3DX12_CPU_DESCRIPTOR_HANDLE hDest(hCPU, 0, uDescriptorSize);
	pDevice->CopyDescriptorsSimple(1, hDest, pGlobalCBContainer->hCPU, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	hDest.Offset(1, uDescriptorSize);

	/*Mesh Consts*/
	CBContainer_t* pMeshCBContainer = pMeshCBPool->Alloc();
	if (!pMeshCBContainer)
	{
		__debugbreak();
		return;
	}

	MeshConstantBuffer_t* pMeshConstantBuffer = reinterpret_cast<MeshConstantBuffer_t*>(pMeshCBContainer->pSystemMemAddr);
	Matrix mWorldRow = Matrix();
	pMeshConstantBuffer->mWorld = mWorldRow.Transpose();
	mWorldRow.Translation(Vector3(0.0f));
	mWorldRow.Invert().Transpose();
	pMeshConstantBuffer->mWorldIT = mWorldRow.Transpose();
	// Per Obj (b1).
	pDevice->CopyDescriptorsSimple(1, hDest, pMeshCBContainer->hCPU, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	hDest.Offset(1, uDescriptorSize);

	/*Particle Sprite Consts*/
	CBContainer_t* pParticleSpriteCBContainer = pParticleSpriteCBPool->Alloc();
	if (!pParticleSpriteCBContainer)
	{
		__debugbreak();
		return;
	}

	ParticleSpriteConstantBuffer_t* pParticleSpriteConstantBuffer = (ParticleSpriteConstantBuffer_t*)pParticleSpriteCBContainer->pSystemMemAddr;
	pParticleSpriteConstantBuffer->vMaxFrame = *pMaxFrame;
	pParticleSpriteConstantBuffer->vCurFrame = *pCurFrame;
	// Per Obj (b2).
	pDevice->CopyDescriptorsSimple(1, hDest, pParticleSpriteCBContainer->hCPU, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	hDest.Offset(1, uDescriptorSize);

	// Per Particle (t1).
	pDevice->CopyDescriptorsSimple(1, hDest, _pTextureHandle->hSRV, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	hDest.Offset(1, uDescriptorSize);

	pCmdList->SetGraphicsRootSignature(sm_pSpriteRS);
	pCmdList->SetDescriptorHeaps(1, &pDescriptorHeap);
	pCmdList->SetPipelineState(sm_pSpritePSO);

	// Obj (root param 0)
	pCmdList->SetGraphicsRootDescriptorTable(0, hGPU);
	pCmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_POINTLIST);

	CD3DX12_GPU_DESCRIPTOR_HANDLE hGPUforMeshes(hGPU, SPRITE_DESCRIPTOR_COUNT_PER_OBJ, uDescriptorSize);
	pCmdList->SetGraphicsRootDescriptorTable(1, hGPUforMeshes);

	// 색을 모두 더하면서 그리는 accumulateBS 사용
	const float blendColor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
	pCmdList->OMSetBlendFactor(blendColor);
	pCmdList->SetGraphicsRootShaderResourceView(2, pDBHandle->pUploadBuffer->GetGPUVirtualAddress());
	pCmdList->DrawInstanced(1, 1, 0, 0);
}

void FParticle::CleanUp()
{
	_pRenderer->EnsureCompleted();

	DestroyCommonResources();
}

AkBool FParticle::CreateCommonResources()
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

AkBool FParticle::CreateRootSignature()
{
	ID3D12Device* pDevice = _pRenderer->GetDevice();
	
	// Spark Root Signature.
	{
		ID3DBlob* pSignature = nullptr;
		ID3DBlob* pError = nullptr;

		CD3DX12_DESCRIPTOR_RANGE tRangesPerObj[1] = {};
		tRangesPerObj[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 4, 0);	// b0 ~ b3: Constant Buffer View per Object.

		CD3DX12_DESCRIPTOR_RANGE tRangesPerParticle[1] = {};
		tRangesPerParticle[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 1);	// t1

		CD3DX12_ROOT_PARAMETER tRootParameters[3] = {};
		tRootParameters[0].InitAsDescriptorTable(_countof(tRangesPerObj), tRangesPerObj, D3D12_SHADER_VISIBILITY_ALL);
		tRootParameters[1].InitAsDescriptorTable(_countof(tRangesPerParticle), tRangesPerParticle, D3D12_SHADER_VISIBILITY_ALL);
		tRootParameters[2].InitAsShaderResourceView(0); // Structured buffer (t0)

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
		tRootSignatureDesc.Init(_countof(tRootParameters), tRootParameters, _countof(pSamplerDesc), pSamplerDesc, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

		if (FAILED(D3D12SerializeRootSignature(&tRootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &pSignature, &pError)))
		{
			__debugbreak();
			return AK_FALSE;
		}

		if (FAILED(pDevice->CreateRootSignature(0, pSignature->GetBufferPointer(), pSignature->GetBufferSize(), IID_PPV_ARGS(&sm_pSparkRS))))
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

	// Sprite Root Signature.
	{
		ID3DBlob* pSignature = nullptr;
		ID3DBlob* pError = nullptr;

		CD3DX12_DESCRIPTOR_RANGE tRangesPerObj[1] = {};
		tRangesPerObj[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 3, 0);	// b0 ~ b2

		CD3DX12_DESCRIPTOR_RANGE tRangesPerParticle[1] = {};
		tRangesPerParticle[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 1);	// t1

		CD3DX12_ROOT_PARAMETER tRootParameters[3] = {};
		tRootParameters[0].InitAsDescriptorTable(_countof(tRangesPerObj), tRangesPerObj, D3D12_SHADER_VISIBILITY_ALL);
		tRootParameters[1].InitAsDescriptorTable(_countof(tRangesPerParticle), tRangesPerParticle, D3D12_SHADER_VISIBILITY_ALL);
		tRootParameters[2].InitAsShaderResourceView(0); // Structured buffer (t0)

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
		tRootSignatureDesc.Init(_countof(tRootParameters), tRootParameters, _countof(pSamplerDesc), pSamplerDesc, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

		if (FAILED(D3D12SerializeRootSignature(&tRootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &pSignature, &pError)))
		{
			__debugbreak();
			return AK_FALSE;
		}

		if (FAILED(pDevice->CreateRootSignature(0, pSignature->GetBufferPointer(), pSignature->GetBufferSize(), IID_PPV_ARGS(&sm_pSpriteRS))))
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

AkBool FParticle::CreatePipelineState()
{
	ID3D12Device* pDevice = _pRenderer->GetDevice();

	ID3DBlob* pParticleSparkVS = nullptr;
	ID3DBlob* pParticleSparkGS = nullptr;
	ID3DBlob* pParticleSparkPS = nullptr;
	ID3DBlob* pParticleSpriteVS = nullptr;
	ID3DBlob* pParticleSpriteGS = nullptr;
	ID3DBlob* pParticleSpritePS = nullptr;

#if defined(_DEBUG)
	// Enable better shader debugging with the graphics debugging tools.
	AkU32 uCompileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
	AkU32 uCompileFlags = 0;
#endif

	ID3DBlob* pErrorBlob = nullptr;
	if (FAILED(D3DCompileFromFile(L"../../shader/ParticleSpark.hlsl", nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "VSMain", "vs_5_0", uCompileFlags, 0, &pParticleSparkVS, &pErrorBlob)))
	{
		if (pErrorBlob != nullptr)
		{
			OutputDebugStringA((char*)pErrorBlob->GetBufferPointer());
			pErrorBlob->Release();
		}
		__debugbreak();
	}
	if (FAILED(D3DCompileFromFile(L"../../shader/ParticleSpark.hlsl", nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "GSMain", "gs_5_0", uCompileFlags, 0, &pParticleSparkGS, &pErrorBlob)))
	{
		if (pErrorBlob != nullptr)
		{
			OutputDebugStringA((char*)pErrorBlob->GetBufferPointer());
			pErrorBlob->Release();
		}
		__debugbreak();
	}
	if (FAILED(D3DCompileFromFile(L"../../shader/ParticleSpark.hlsl", nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "PSMain", "ps_5_0", uCompileFlags, 0, &pParticleSparkPS, &pErrorBlob)))
	{
		if (pErrorBlob != nullptr)
		{
			OutputDebugStringA((char*)pErrorBlob->GetBufferPointer());
			pErrorBlob->Release();
		}
		__debugbreak();
	}
	if (FAILED(D3DCompileFromFile(L"../../shader/ParticleSprite.hlsl", nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "VSMain", "vs_5_0", uCompileFlags, 0, &pParticleSpriteVS, &pErrorBlob)))
	{
		if (pErrorBlob != nullptr)
		{
			OutputDebugStringA((char*)pErrorBlob->GetBufferPointer());
			pErrorBlob->Release();
		}
		__debugbreak();
	}
	if (FAILED(D3DCompileFromFile(L"../../shader/ParticleSprite.hlsl", nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "GSMain", "gs_5_0", uCompileFlags, 0, &pParticleSpriteGS, &pErrorBlob)))
	{
		if (pErrorBlob != nullptr)
		{
			OutputDebugStringA((char*)pErrorBlob->GetBufferPointer());
			pErrorBlob->Release();
		}
		__debugbreak();
	}
	if (FAILED(D3DCompileFromFile(L"../../shader/ParticleSprite.hlsl", nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "PSMain", "ps_5_0", uCompileFlags, 0, &pParticleSpritePS, &pErrorBlob)))
	{
		if (pErrorBlob != nullptr)
		{
			OutputDebugStringA((char*)pErrorBlob->GetBufferPointer());
			pErrorBlob->Release();
		}
		__debugbreak();
	}

	// Describe and create the graphics pipeline state object (PSO).
	D3D12_GRAPHICS_PIPELINE_STATE_DESC tOpaquePsoDesc = {};
	tOpaquePsoDesc.pRootSignature = sm_pSparkRS;
	tOpaquePsoDesc.VS = CD3DX12_SHADER_BYTECODE(pParticleSparkVS->GetBufferPointer(), pParticleSparkVS->GetBufferSize());
	tOpaquePsoDesc.GS = CD3DX12_SHADER_BYTECODE(pParticleSparkGS->GetBufferPointer(), pParticleSparkGS->GetBufferSize());
	tOpaquePsoDesc.PS = CD3DX12_SHADER_BYTECODE(pParticleSparkPS->GetBufferPointer(), pParticleSparkPS->GetBufferSize());
	tOpaquePsoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	tOpaquePsoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	tOpaquePsoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	tOpaquePsoDesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
	tOpaquePsoDesc.DepthStencilState.StencilEnable = FALSE;
	tOpaquePsoDesc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
	tOpaquePsoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
	tOpaquePsoDesc.SampleMask = UINT_MAX;
	tOpaquePsoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT;
	tOpaquePsoDesc.NumRenderTargets = 1;
	tOpaquePsoDesc.RTVFormats[0] = _pRenderer->GetFloatRTVFormat();
	tOpaquePsoDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
	tOpaquePsoDesc.SampleDesc.Count = _pRenderer->UseMSAA() ? 4 : 1;
	tOpaquePsoDesc.SampleDesc.Quality = _pRenderer->UseMSAA() ? _pRenderer->GetNumQualityLevel() - 1 : 0;

	D3D12_GRAPHICS_PIPELINE_STATE_DESC tAccumulatePSODesc = tOpaquePsoDesc;
	D3D12_RENDER_TARGET_BLEND_DESC tTransparencyBlendDesc = {};
	tTransparencyBlendDesc.BlendEnable = AK_TRUE;
	tTransparencyBlendDesc.LogicOpEnable = AK_FALSE;
	tTransparencyBlendDesc.SrcBlend = D3D12_BLEND_SRC_ALPHA;
	tTransparencyBlendDesc.DestBlend = D3D12_BLEND_ONE;
	tTransparencyBlendDesc.BlendOp = D3D12_BLEND_OP_ADD;
	tTransparencyBlendDesc.SrcBlendAlpha = D3D12_BLEND_ONE;
	tTransparencyBlendDesc.DestBlendAlpha = D3D12_BLEND_ONE;
	tTransparencyBlendDesc.BlendOpAlpha = D3D12_BLEND_OP_ADD;
	tTransparencyBlendDesc.LogicOp = D3D12_LOGIC_OP_NOOP;
	tTransparencyBlendDesc.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

	tAccumulatePSODesc.BlendState.AlphaToCoverageEnable = AK_TRUE;
	tAccumulatePSODesc.BlendState.RenderTarget[0] = tTransparencyBlendDesc;

	if (FAILED(pDevice->CreateGraphicsPipelineState(&tAccumulatePSODesc, IID_PPV_ARGS(&sm_pSparkPSO))))
	{
		__debugbreak();
	}

	tAccumulatePSODesc.VS = CD3DX12_SHADER_BYTECODE(pParticleSpriteVS->GetBufferPointer(), pParticleSpriteVS->GetBufferSize());
	tAccumulatePSODesc.GS = CD3DX12_SHADER_BYTECODE(pParticleSpriteGS->GetBufferPointer(), pParticleSpriteGS->GetBufferSize());
	tAccumulatePSODesc.PS = CD3DX12_SHADER_BYTECODE(pParticleSpritePS->GetBufferPointer(), pParticleSpritePS->GetBufferSize());
	if (FAILED(pDevice->CreateGraphicsPipelineState(&tAccumulatePSODesc, IID_PPV_ARGS(&sm_pSpritePSO))))
	{
		__debugbreak();
	}

	if (pParticleSpritePS)
	{
		pParticleSpritePS->Release();
	}
	if (pParticleSpriteGS)
	{
		pParticleSpriteGS->Release();
	}
	if (pParticleSpriteVS)
	{
		pParticleSpriteVS->Release();
	}
	if (pParticleSparkPS)
	{
		pParticleSparkPS->Release();
	}
	if (pParticleSparkGS)
	{
		pParticleSparkGS->Release();
	}
	if (pParticleSparkVS)
	{
		pParticleSparkVS->Release();
	}

	return AK_TRUE;
}

void FParticle::DestroyCommonResources()
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

void FParticle::DestroyRootSignature()
{
	if (sm_pSparkRS)
	{
		sm_pSparkRS->Release();
		sm_pSparkRS = nullptr;
	}
	if (sm_pSpriteRS)
	{
		sm_pSpriteRS->Release();
		sm_pSpriteRS = nullptr;
	}
}

void FParticle::DestroyPipelineState()
{
	if (sm_pSparkPSO)
	{
		sm_pSparkPSO->Release();
		sm_pSparkPSO = nullptr;
	}
	if (sm_pSpritePSO)
	{
		sm_pSpritePSO->Release();
		sm_pSpritePSO = nullptr;
	}
}
