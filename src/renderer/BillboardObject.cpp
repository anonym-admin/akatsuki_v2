#include "pch.h"
#include "BillboardObject.h"
#include "Renderer.h"
#include "DescriptorPool.h"
#include "ConstantBufferManager.h"
#include "ConstantBufferPool.h"
#include "ResourceManager.h"

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

AkBool FBillboardObjects::CreateBillboardBuffer(BillboardVertex_t* pBillboardVertices, AkU32 uPointNum)
{
	FResourceManager* pResourceManager = _pRenderer->GetResourceManager();
	D3D12_VERTEX_BUFFER_VIEW tVBView = {};
	ID3D12Resource* pVertexBuffer = nullptr;

	if (pResourceManager->CreateVertexBuffer(sizeof(BillboardVertex_t), uPointNum, &tVBView, &pVertexBuffer, pBillboardVertices))
	{
		_pVertexBuffer = pVertexBuffer;
		_tVertexBufferView = tVBView;
	}

	_uPointNum = uPointNum;

	return AK_TRUE;
}

void FBillboardObjects::SetTextureArray(void* pTexHandle)
{
	_pTexArray = (TextureHandle_t*)pTexHandle;
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
	AkU32 uDescriptorSize = pDescriptorPool->GetDescriptorTypeSize();

	CD3DX12_CPU_DESCRIPTOR_HANDLE hCPU = {};
	CD3DX12_GPU_DESCRIPTOR_HANDLE hGPU = {};
	AkU32 uRequiredDescriptorNum = DESCRIPTOR_COUNT_PER_OBJ * DESCRIPTOR_COUNT_PER_BILLBOARD;

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

	// Per Billboard (t0 ~ )
	if (_pTexArray->hSRV.ptr)
	{
		pDevice->CopyDescriptorsSimple(1, hDest, _pTexArray->hSRV, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
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

	CD3DX12_DESCRIPTOR_RANGE tRangesPerTriGroup[1] = {};
	tRangesPerTriGroup[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);	// t0 => texture array.

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
	if (FAILED(D3DCompileFromFile(L"../../shader/BillboardShader.hlsl", nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "VSMain", "vs_5_0", uCompileFlags, 0, &pBillboardVS, &pErrorBlob)))
	{
		if (pErrorBlob != nullptr)
		{
			OutputDebugStringA((char*)pErrorBlob->GetBufferPointer());
			pErrorBlob->Release();
		}
		__debugbreak();
	}
	if (FAILED(D3DCompileFromFile(L"../../shader/BillboardShader.hlsl", nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "GSMain", "gs_5_0", uCompileFlags, 0, &pBillboardGS, &pErrorBlob)))
	{
		if (pErrorBlob != nullptr)
		{
			OutputDebugStringA((char*)pErrorBlob->GetBufferPointer());
			pErrorBlob->Release();
		}
		__debugbreak();
	}
	if (FAILED(D3DCompileFromFile(L"../../shader/BillboardShader.hlsl", nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "PSMain", "ps_5_0", uCompileFlags, 0, &pBillboardPS, &pErrorBlob)))
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
		{ "SIZE", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
	};

	// Describe and create the graphics pipeline state object (PSO).
	D3D12_GRAPHICS_PIPELINE_STATE_DESC tPsoDesc = {};
	tPsoDesc.InputLayout = { tInputElementDescs, _countof(tInputElementDescs) };
	tPsoDesc.pRootSignature = sm_pRootSignature;
	tPsoDesc.VS = CD3DX12_SHADER_BYTECODE(pBillboardVS->GetBufferPointer(), pBillboardVS->GetBufferSize());
	tPsoDesc.GS = CD3DX12_SHADER_BYTECODE(pBillboardGS->GetBufferPointer(), pBillboardGS->GetBufferSize());
	tPsoDesc.PS = CD3DX12_SHADER_BYTECODE(pBillboardPS->GetBufferPointer(), pBillboardPS->GetBufferSize());
	tPsoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	tPsoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	tPsoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	tPsoDesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
	tPsoDesc.DepthStencilState.StencilEnable = FALSE;
	tPsoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
	//tPsoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_BACK;
	tPsoDesc.SampleMask = UINT_MAX;
	tPsoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT;
	tPsoDesc.NumRenderTargets = 1;
	tPsoDesc.RTVFormats[0] = _pRenderer->GetFloatRTVFormat();
	tPsoDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
	tPsoDesc.SampleDesc.Count = 1;
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
