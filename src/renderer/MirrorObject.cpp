#include "pch.h"
#include "MirrorObject.h"
#include "Renderer.h"
#include "ResourceManager.h"
#include "DescriptorPool.h"
#include "ConstantBufferPool.h"

ID3D12PipelineState* FMirrorObject::sm_pStencilMaskPSO;
ID3D12RootSignature* FMirrorObject::sm_pRootSignature;
AkU32 FMirrorObject::sm_uInitRefCount;
Mesh_t* FMirrorObject::sm_pMesh;

FMirrorObject::FMirrorObject()
{
}

FMirrorObject::~FMirrorObject()
{
	CleanUp();
}

AkBool FMirrorObject::Initialize(FRenderer* pRenderer)
{
	AkBool bResult = AK_TRUE;

	_pRenderer = pRenderer;

	bResult = CreateCommonResources();

	return bResult;
}

void FMirrorObject::CleanUp()
{
	_pRenderer->EnsureCompleted();

	DestroyCommonResources();
}

AkBool FMirrorObject::CreateCommonResources()
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
	if (!CreateMeshBuffers())
	{
		__debugbreak();
		return AK_FALSE;
	}

	sm_uInitRefCount++;

	return AK_TRUE;
}

// This Function is masked in stencil.
void FMirrorObject::Masking(AkU32 uThreadIndex, ID3D12GraphicsCommandList* pCmdList, const Matrix* pWorldMat)
{
	ID3D12Device* pDevice = _pRenderer->GetDevice();
	FDescriptorPool* pDescriptorPool = _pRenderer->GetDescriptorPool(0);
	ID3D12DescriptorHeap* pDescriptorHeap = pDescriptorPool->GetDescriptorHeap();
	FConstantBufferPool* pGlobalCBPool = _pRenderer->GetConstantBufferPool(0, CONSTANT_BUFFER_TYPE::CONSTANT_BUFFER_TYPE_GLOBAL);
	FConstantBufferPool* pMeshCBPool = _pRenderer->GetConstantBufferPool(0, CONSTANT_BUFFER_TYPE::CONSTANT_BUFFER_TYPE_MESH);
	AkU32 uDescriptorSize = pDescriptorPool->GetDescriptorTypeSize();

	CD3DX12_CPU_DESCRIPTOR_HANDLE hCPU = {};
	CD3DX12_GPU_DESCRIPTOR_HANDLE hGPU = {};
	AkU32 uRequiredDescriptorNum = DESCRIPTOR_COUNT_PER_OBJ;

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

	// Set RootSignature.
	pCmdList->SetGraphicsRootSignature(sm_pRootSignature);
	pCmdList->SetDescriptorHeaps(1, &pDescriptorHeap);
	pCmdList->SetPipelineState(sm_pStencilMaskPSO);

	pCmdList->OMSetStencilRef(1);

	// Obj (root param 0)
	pCmdList->SetGraphicsRootDescriptorTable(0, hGPU);
	pCmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	pCmdList->IASetVertexBuffers(0, 1, &sm_pMesh->tVBView);
	pCmdList->IASetIndexBuffer(&sm_pMesh->tIBView);
	pCmdList->DrawIndexedInstanced(sm_pMesh->uIndexCountPerInstance, 1, 0, 0, 0);
}

void FMirrorObject::Draw(AkU32 uThreadIndex, ID3D12GraphicsCommandList* pCmdList, const Matrix* pWorldMat)
{
}

AkBool FMirrorObject::CreateRootSignature()
{
	ID3D12Device* pDevice = _pRenderer->GetDevice();
	ID3DBlob* pSignature = nullptr;
	ID3DBlob* pError = nullptr;

	CD3DX12_DESCRIPTOR_RANGE tRangesPerObj[1] = {};
	tRangesPerObj[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 2, 0);	// b0, b1: Constant Buffer View per Object.

	CD3DX12_DESCRIPTOR_RANGE tRangesPerTriGroup[4] = {};
	tRangesPerTriGroup[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 2);	// b2: Constant Buffer View per Mesh
	tRangesPerTriGroup[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 7, 0);	// t0~t6 : Shader Resource View(Tex) per Mesh.
	tRangesPerTriGroup[2].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 3, 11);	// t10, t11, t12, t13 : Shader Resource View(Tex) per Mesh. (IBL Texture)
	tRangesPerTriGroup[3].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 5, 15);	// t15, t16, t17, t18, t19 : Shadow Map

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

AkBool FMirrorObject::CreatePipelineState()
{
	ID3D12Device* pDevice = _pRenderer->GetDevice();

	ID3DBlob* pDepthOnlyVS = nullptr;
	ID3DBlob* pDepthOnlyPS = nullptr;

#if defined(_DEBUG)
	// Enable better shader debugging with the graphics debugging tools.
	AkU32 uCompileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
	AkU32 uCompileFlags = 0;
#endif

	ID3DBlob* pErrorBlob = nullptr;
	if (FAILED(D3DCompileFromFile(L"../../shader/DepthOnly.hlsl", nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "VSMain", "vs_5_0", uCompileFlags, 0, &pDepthOnlyVS, &pErrorBlob)))
	{
		if (pErrorBlob != nullptr)
		{
			OutputDebugStringA((char*)pErrorBlob->GetBufferPointer());
			pErrorBlob->Release();
		}
		__debugbreak();
	}
	if (FAILED(D3DCompileFromFile(L"../../shader/DepthOnly.hlsl", nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "PSMain", "ps_5_0", uCompileFlags, 0, &pDepthOnlyPS, &pErrorBlob)))
	{
		if (pErrorBlob != nullptr)
		{
			OutputDebugStringA((char*)pErrorBlob->GetBufferPointer());
			pErrorBlob->Release();
		}
		__debugbreak();
	}

	// Define the vertex input layout.
	D3D12_INPUT_ELEMENT_DESC tInputElementDescs[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,	0, 24,	D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 32, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
	};

	D3D12_GRAPHICS_PIPELINE_STATE_DESC tPsoDesc = {};
	tPsoDesc.InputLayout = { tInputElementDescs, _countof(tInputElementDescs) };
	tPsoDesc.pRootSignature = sm_pRootSignature;
	tPsoDesc.VS = CD3DX12_SHADER_BYTECODE(pDepthOnlyVS->GetBufferPointer(), pDepthOnlyVS->GetBufferSize());
	tPsoDesc.PS = CD3DX12_SHADER_BYTECODE(pDepthOnlyPS->GetBufferPointer(), pDepthOnlyPS->GetBufferSize());
	tPsoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	tPsoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	tPsoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	tPsoDesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
	tPsoDesc.DepthStencilState.StencilEnable = FALSE;
	//psoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
	tPsoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_BACK;
	tPsoDesc.SampleMask = UINT_MAX;
	tPsoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	tPsoDesc.NumRenderTargets = 0;
	tPsoDesc.RTVFormats[0] = DXGI_FORMAT_UNKNOWN;
	tPsoDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
	tPsoDesc.SampleDesc.Count = _pRenderer->UseMSAA() ? 4 : 1;
	tPsoDesc.SampleDesc.Quality = _pRenderer->UseMSAA() ? _pRenderer->GetNumQualityLevel() - 1 : 0;
	// Stencil 에 1로 표기해주는 State.
	tPsoDesc.DepthStencilState.StencilEnable = TRUE;
	tPsoDesc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
	const D3D12_DEPTH_STENCILOP_DESC tDefaultStencilOp =
	{ D3D12_STENCIL_OP_KEEP, D3D12_STENCIL_OP_KEEP, D3D12_STENCIL_OP_REPLACE, D3D12_COMPARISON_FUNC_ALWAYS };
	tPsoDesc.DepthStencilState.FrontFace = tDefaultStencilOp;
	if (FAILED(pDevice->CreateGraphicsPipelineState(&tPsoDesc, IID_PPV_ARGS(&sm_pStencilMaskPSO))))
	{
		__debugbreak();
	}

	if (pDepthOnlyPS)
	{
		pDepthOnlyPS->Release();
		pDepthOnlyPS = nullptr;
	}
	if (pDepthOnlyVS)
	{
		pDepthOnlyVS->Release();
		pDepthOnlyVS = nullptr;
	}

	return AK_TRUE;
}

AkBool FMirrorObject::CreateMeshBuffers()
{
	sm_pMesh = reinterpret_cast<Mesh_t*>(malloc(sizeof(Mesh_t)));

	memset(sm_pMesh, 0, sizeof(Mesh_t));

	FResourceManager* pResourceManager = _pRenderer->GetResourceManager();
	D3D12_VERTEX_BUFFER_VIEW tVBView = {};
	D3D12_INDEX_BUFFER_VIEW tIBView = {};
	ID3D12Resource* pVertexBuffer = nullptr;
	ID3D12Resource* pIndexBuffer = nullptr;

	Vertex_t pVertices[4];
	pVertices[0].vPosition = Vector3(-1.0f, -1.0f, 0.0f);
	pVertices[1].vPosition = Vector3(-1.0f, 1.0f, 0.0f);
	pVertices[2].vPosition = Vector3(1.0f, 1.0f, 0.0f);
	pVertices[3].vPosition = Vector3(1.0f, -1.0f, 0.0f);

	pVertices[0].vNormalModel = Vector3(0.0f, 1.0f, 0.0f);
	pVertices[1].vNormalModel = Vector3(0.0f, 1.0f, 0.0f);
	pVertices[2].vNormalModel = Vector3(0.0f, 1.0f, 0.0f);
	pVertices[3].vNormalModel = Vector3(0.0f, 1.0f, 0.0f);

	AkU32 pIndices[6] =
	{
		0, 1, 2, 0, 2, 3,
	};

	if (pResourceManager->CreateVertexBuffer(sizeof(Vertex_t), 4, &tVBView, &pVertexBuffer, pVertices))
	{
		sm_pMesh->pVB = pVertexBuffer;
		sm_pMesh->tVBView = tVBView;
	}

	if (pResourceManager->CreateIndexBuffer(6, &tIBView, &pIndexBuffer, pIndices))
	{
		sm_pMesh->pIB = pIndexBuffer;
		sm_pMesh->tIBView = tIBView;
		sm_pMesh->uVertexCountPerInstance = 4;
		sm_pMesh->uIndexCountPerInstance = 6;
	}

	return AK_TRUE;
}

void FMirrorObject::DestroyCommonResources()
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
		DestroyMeshBuffers();
	}
}

void FMirrorObject::DestroyRootSignature()
{
	if (sm_pRootSignature)
	{
		sm_pRootSignature->Release();
		sm_pRootSignature = nullptr;
	}
}

void FMirrorObject::DestroyPipelineState()
{
	if (sm_pStencilMaskPSO)
	{
		sm_pStencilMaskPSO->Release();
		sm_pStencilMaskPSO = nullptr;
	}
}

void FMirrorObject::DestroyMeshBuffers()
{
	if (sm_pMesh)
	{
		if (sm_pMesh->pVB)
		{
			sm_pMesh->pVB->Release();
			sm_pMesh->pVB = nullptr;
		}
		if (sm_pMesh->pIB)
		{
			sm_pMesh->pIB->Release();
			sm_pMesh->pIB = nullptr;
		}

		free(sm_pMesh);
		sm_pMesh = nullptr;
	}
}

HRESULT __stdcall FMirrorObject::QueryInterface(REFIID riid, void** ppvObject)
{
	return E_NOTIMPL;
}

ULONG __stdcall FMirrorObject::AddRef(void)
{
	return 0;
}

ULONG __stdcall FMirrorObject::Release(void)
{
	return 0;
}
