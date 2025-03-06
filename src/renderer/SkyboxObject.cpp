#include "pch.h"
#include "SkyboxObject.h"
#include "Renderer.h"
#include "D3DUtils.h"
#include "ResourceManager.h"
#include "DescriptorPool.h"
#include "ConstantBufferPool.h"

/*
================
SkyboxObject
================
*/

AkU32 FSkyboxObject::sm_uInitRefCount;
ID3D12RootSignature* FSkyboxObject::sm_pRootSignature;
ID3D12PipelineState* FSkyboxObject::sm_pSkyboxPSO;

ID3D12Resource* FSkyboxObject::sm_pVertexBuffer;
D3D12_VERTEX_BUFFER_VIEW FSkyboxObject::sm_tVertexBufferView;

ID3D12Resource* FSkyboxObject::sm_pIndexBuffer;
D3D12_INDEX_BUFFER_VIEW FSkyboxObject::sm_tIndexBufferView;

FSkyboxObject::FSkyboxObject()
{
}

FSkyboxObject::~FSkyboxObject()
{
	CleanUp();
}

AkBool FSkyboxObject::Initialize(FRenderer* pRenderer)
{
	_pRenderer = pRenderer;

	AkBool bResult = CreateCommonResources();
	return bResult;
}

void FSkyboxObject::Draw(AkU32 uThreadIndex, ID3D12GraphicsCommandList* pCmdList, const Matrix* pWorldMat, TextureHandle_t* pEnvHDR, TextureHandle_t* pDiffuseHDR, TextureHandle_t* pSpecular)
{
	ID3D12Device* pDevice = _pRenderer->GetDevice();
	FDescriptorPool* pDescriptorPool = _pRenderer->GetDescriptorPool(uThreadIndex);
	ID3D12DescriptorHeap* pDescriptorHeap = pDescriptorPool->GetDescriptorHeap();
	FConstantBufferPool* pGlobalCBPool = _pRenderer->GetConstantBufferPool(uThreadIndex, CONSTANT_BUFFER_TYPE::CONSTANT_BUFFER_TYPE_GLOBAL);
	FConstantBufferPool* pMeshCBPool = _pRenderer->GetConstantBufferPool(uThreadIndex, CONSTANT_BUFFER_TYPE::CONSTANT_BUFFER_TYPE_MESH);
	AkU32 uDescriptorSize = pDescriptorPool->GetDescriptorTypeSize();

	CD3DX12_CPU_DESCRIPTOR_HANDLE hCPU = {};
	CD3DX12_GPU_DESCRIPTOR_HANDLE hGPU = {};
	AkU32 uRequiredDescriptorNum = DESCRIPTOR_COUNT_PER_OBJ + (MAX_MESH_COUNT_PER_OBJ * DESCRIPTOR_COUNT_PER_MESH);

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

	// Per Mesh
	pDevice->CopyDescriptorsSimple(1, hDest, pEnvHDR->hSRV, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	hDest.Offset(1, uDescriptorSize);

	pDevice->CopyDescriptorsSimple(1, hDest, pDiffuseHDR->hSRV, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	hDest.Offset(1, uDescriptorSize);

	pDevice->CopyDescriptorsSimple(1, hDest, pSpecular->hSRV, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	hDest.Offset(1, uDescriptorSize);

	// Set RootSignature.
	pCmdList->SetGraphicsRootSignature(sm_pRootSignature);
	pCmdList->SetDescriptorHeaps(1, &pDescriptorHeap);
	pCmdList->SetPipelineState(sm_pSkyboxPSO);

	// Obj (root param 0)
	pCmdList->SetGraphicsRootDescriptorTable(0, hGPU);

	hGPU.Offset(1, uDescriptorSize);

	// Obj (root param 1)
	pCmdList->SetGraphicsRootDescriptorTable(1, hGPU);

	pCmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	pCmdList->IASetVertexBuffers(0, 1, &sm_tVertexBufferView);
	pCmdList->IASetIndexBuffer(&sm_tIndexBufferView);
	pCmdList->DrawIndexedInstanced(36, 1, 0, 0, 0);
}

HRESULT __stdcall FSkyboxObject::QueryInterface(REFIID riid, void** ppvObject)
{
	return E_NOTIMPL;
}

ULONG __stdcall FSkyboxObject::AddRef(void)
{
	AkU32 uRefCount = ++_uRefCount;
	return uRefCount;
}

ULONG __stdcall FSkyboxObject::Release(void)
{
	AkU32 uRefCount = --_uRefCount;
	if (!uRefCount)
	{
		delete this;
	}
	return uRefCount;
}

void FSkyboxObject::CleanUp()
{
	if (_pDiffuseHDR)
	{
		_pRenderer->DestroyTexture(_pDiffuseHDR);
		_pDiffuseHDR = nullptr;
	}
	if (_pSpecularHDR)
	{
		_pRenderer->DestroyTexture(_pSpecularHDR);
		_pSpecularHDR = nullptr;
	}
	if (_pEnvHDR)
	{
		_pRenderer->DestroyTexture(_pEnvHDR);
		_pEnvHDR = nullptr;
	}

	DestroyCommonResources();
}

AkBool FSkyboxObject::CreateCommonResources()
{
	if (sm_uInitRefCount)
	{
		return AK_TRUE;
	}

	CreateRootSignature();
	CreatePipelineState();
	CreateMesh();

	sm_uInitRefCount++;
	return sm_uInitRefCount;
}

AkBool FSkyboxObject::CreateRootSignature()
{
	ID3D12Device* pD3DDeivce = _pRenderer->GetDevice();
	ID3DBlob* pSignature = nullptr;
	ID3DBlob* pError = nullptr;

	CD3DX12_DESCRIPTOR_RANGE tRanges[1] = {};
	tRanges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 2, 0);	// b0, b1: Constant Buffer View

	CD3DX12_DESCRIPTOR_RANGE tRangesPerTriGroup[1] = {};
	tRangesPerTriGroup[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 3, 10);	// t10, t11, t12 : Shader Resource View(Tex) per Mesh.

	CD3DX12_ROOT_PARAMETER tRootParameters[2] = {};
	tRootParameters[0].InitAsDescriptorTable(_countof(tRanges), tRanges, D3D12_SHADER_VISIBILITY_ALL);
	tRootParameters[1].InitAsDescriptorTable(_countof(tRangesPerTriGroup), tRangesPerTriGroup, D3D12_SHADER_VISIBILITY_ALL);

	// default sampler
	D3D12_STATIC_SAMPLER_DESC tSampler = {};
	FD3DUtils::SetDefaultSamplerDesc(&tSampler, 0);
	tSampler.Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;

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
	tRootSignatureDesc.Init(_countof(tRootParameters), tRootParameters, 1, &tSampler, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	if (FAILED(D3D12SerializeRootSignature(&tRootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &pSignature, &pError)))
	{
		__debugbreak();
	}

	if (FAILED(pD3DDeivce->CreateRootSignature(0, pSignature->GetBufferPointer(), pSignature->GetBufferSize(), IID_PPV_ARGS(&sm_pRootSignature))))
	{
		__debugbreak();
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

AkBool FSkyboxObject::CreatePipelineState()
{
	ID3D12Device* pD3DDeivce = _pRenderer->GetDevice();

	ID3DBlob* pVertexShader = nullptr;
	ID3DBlob* pPixelShader = nullptr;


#if defined(_DEBUG)
	// Enable better shader debugging with the graphics debugging tools.
	AkU32 compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
	UINT compileFlags = 0;
#endif
	//m_pRenderer->SetCurrentPathForShader();

	ID3DBlob* pErrorBlob = nullptr;
	if (FAILED(D3DCompileFromFile(L"../../shader/SkyboxShader.hlsl", nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "VSMain", "vs_5_0", compileFlags, 0, &pVertexShader, &pErrorBlob)))
	{
		if (pErrorBlob != nullptr)
		{
			OutputDebugStringA((char*)pErrorBlob->GetBufferPointer());
			pErrorBlob->Release();
		}
		__debugbreak();
	}
	if (FAILED(D3DCompileFromFile(L"../../shader/SkyboxShader.hlsl", nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "PSMain", "ps_5_0", compileFlags, 0, &pPixelShader, &pErrorBlob)))
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
	};


	// Describe and create the graphics pipeline state object (PSO).
	D3D12_GRAPHICS_PIPELINE_STATE_DESC tOpaquePsoDesc = {};
	tOpaquePsoDesc.InputLayout = { tInputElementDescs, _countof(tInputElementDescs) };
	tOpaquePsoDesc.pRootSignature = sm_pRootSignature;
	tOpaquePsoDesc.VS = CD3DX12_SHADER_BYTECODE(pVertexShader->GetBufferPointer(), pVertexShader->GetBufferSize());
	tOpaquePsoDesc.PS = CD3DX12_SHADER_BYTECODE(pPixelShader->GetBufferPointer(), pPixelShader->GetBufferSize());
	tOpaquePsoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	tOpaquePsoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	tOpaquePsoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	tOpaquePsoDesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
	tOpaquePsoDesc.DepthStencilState.StencilEnable = FALSE;
	tOpaquePsoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
	//tOpaquePsoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_BACK;
	tOpaquePsoDesc.SampleMask = UINT_MAX;
	tOpaquePsoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	tOpaquePsoDesc.NumRenderTargets = 1;
	tOpaquePsoDesc.RTVFormats[0] = _pRenderer->GetFloatRTVFormat();
	tOpaquePsoDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
	tOpaquePsoDesc.SampleDesc.Count = 1;
	if (FAILED(pD3DDeivce->CreateGraphicsPipelineState(&tOpaquePsoDesc, IID_PPV_ARGS(&sm_pSkyboxPSO))))
	{
		__debugbreak();
	}

	if (pVertexShader)
	{
		pVertexShader->Release();
		pVertexShader = nullptr;
	}
	if (pPixelShader)
	{
		pPixelShader->Release();
		pPixelShader = nullptr;
	}
	return AK_TRUE;
}

AkBool FSkyboxObject::CreateMesh()
{
	ID3D12Device* pD3DDeivce = _pRenderer->GetDevice();
	AkU32 uSrvDescriptorSize = pD3DDeivce->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	FResourceManager* pResourceManager = _pRenderer->GetResourceManager();
	FDescriptorAllocator* pSingleDescriptorAllocator = _pRenderer->GetDescriptorAllocator();

	AkU32 uVerticesNum = 24;
	AkU32 uIndicesNum = 36;

	Vector3* pVertices = new Vector3[uVerticesNum];
	AkU32* pIndices = new AkU32[uIndicesNum];

	pVertices[0] = Vector3(-1.0f, 1.0f, -1.0f) * 500.0f;
	pVertices[1] = Vector3(-1.0f, 1.0f, 1.0f) * 500.0f;
	pVertices[2] = Vector3(1.0f, 1.0f, 1.0f) * 500.0f;
	pVertices[3] = Vector3(1.0f, 1.0f, -1.0f) * 500.0f;
	pVertices[4] = Vector3(-1.0f, -1.0f, -1.0f) * 500.0f;
	pVertices[5] = Vector3(1.0f, -1.0f, -1.0f) * 500.0f;
	pVertices[6] = Vector3(1.0f, -1.0f, 1.0f) * 500.0f;
	pVertices[7] = Vector3(-1.0f, -1.0f, 1.0f) * 500.0f;
	pVertices[8] = Vector3(-1.0f, -1.0f, -1.0f) * 500.0f;
	pVertices[9] = Vector3(-1.0f, 1.0f, -1.0f) * 500.0f;
	pVertices[10] = Vector3(1.0f, 1.0f, -1.0f) * 500.0f;
	pVertices[11] = Vector3(1.0f, -1.0f, -1.0f) * 500.0f;
	pVertices[12] = Vector3(-1.0f, -1.0f, 1.0f) * 500.0f;
	pVertices[13] = Vector3(1.0f, -1.0f, 1.0f) * 500.0f;
	pVertices[14] = Vector3(1.0f, 1.0f, 1.0f) * 500.0f;
	pVertices[15] = Vector3(-1.0f, 1.0f, 1.0f) * 500.0f;
	pVertices[16] = Vector3(-1.0f, -1.0f, 1.0f) * 500.0f;
	pVertices[17] = Vector3(-1.0f, 1.0f, 1.0f) * 500.0f;
	pVertices[18] = Vector3(-1.0f, 1.0f, -1.0f) * 500.0f;
	pVertices[19] = Vector3(-1.0f, -1.0f, -1.0f) * 500.0f;
	pVertices[20] = Vector3(1.0f, -1.0f, 1.0f) * 500.0f;
	pVertices[21] = Vector3(1.0f, -1.0f, -1.0f) * 500.0f;
	pVertices[22] = Vector3(1.0f, 1.0f, -1.0f) * 500.0f;
	pVertices[23] = Vector3(1.0f, 1.0f, 1.0f) * 500.0f;

	// Fill in the front face index data
	pIndices[0] = 0;
	pIndices[1] = 1;
	pIndices[2] = 2;
	pIndices[3] = 0;
	pIndices[4] = 2;
	pIndices[5] = 3;

	// Fill in the back face index data
	pIndices[6] = 4;
	pIndices[7] = 5;
	pIndices[8] = 6;
	pIndices[9] = 4;
	pIndices[10] = 6;
	pIndices[11] = 7;

	// Fill in the top face index data
	pIndices[12] = 8;
	pIndices[13] = 9;
	pIndices[14] = 10;
	pIndices[15] = 8;
	pIndices[16] = 10;
	pIndices[17] = 11;

	// Fill in the bottom face index data
	pIndices[18] = 12;
	pIndices[19] = 13;
	pIndices[20] = 14;
	pIndices[21] = 12;
	pIndices[22] = 14;
	pIndices[23] = 15;

	// Fill in the left face index data
	pIndices[24] = 16;
	pIndices[25] = 17;
	pIndices[26] = 18;
	pIndices[27] = 16;
	pIndices[28] = 18;
	pIndices[29] = 19;

	// Fill in the right face index data
	pIndices[30] = 20;
	pIndices[31] = 21;
	pIndices[32] = 22;
	pIndices[33] = 20;
	pIndices[34] = 22;
	pIndices[35] = 23;

	if (FAILED(pResourceManager->CreateVertexBuffer(sizeof(Vector3), uVerticesNum, &sm_tVertexBufferView, &sm_pVertexBuffer, pVertices)))
	{
		__debugbreak();
		return AK_FALSE;
	}

	if (FAILED(pResourceManager->CreateIndexBuffer(uIndicesNum, &sm_tIndexBufferView, &sm_pIndexBuffer, pIndices)))
	{
		__debugbreak();
		return AK_FALSE;
	}

	delete[] pVertices;
	pVertices = nullptr;

	delete[] pIndices;
	pIndices = nullptr;

	return AK_TRUE;
}

void FSkyboxObject::DestroyCommonResources()
{
	if (!sm_uInitRefCount)
		return;

	AkU32 uRefCount = --sm_uInitRefCount;
	if (!uRefCount)
	{
		if (sm_pRootSignature)
		{
			sm_pRootSignature->Release();
			sm_pRootSignature = nullptr;
		}
		if (sm_pSkyboxPSO)
		{
			sm_pSkyboxPSO->Release();
			sm_pSkyboxPSO = nullptr;
		}
		if (sm_pVertexBuffer)
		{
			sm_pVertexBuffer->Release();
			sm_pVertexBuffer = nullptr;
		}
		if (sm_pIndexBuffer)
		{
			sm_pIndexBuffer->Release();
			sm_pIndexBuffer = nullptr;
		}
	}
}

void FSkyboxObject::DestroyRootSignature()
{
}

void FSkyboxObject::DestroyPipelineState()
{
}

void FSkyboxObject::DestroyMesh()
{
}
