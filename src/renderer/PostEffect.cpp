#include "pch.h"
#include "PostEffect.h"
#include "Renderer.h"
#include "ResourceManager.h"
#include "CommandListPool.h"
#include "ConstantBufferPool.h"
#include "DescriptorPool.h"

/*
=============
Post Effect
=============
*/

FPostEffect::FPostEffect()
{
}

FPostEffect::~FPostEffect()
{
	CleanUp();
}

AkBool FPostEffect::Initialize(FRenderer* pRenderer)
{
	_pRenderer = pRenderer;

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

	return AK_TRUE;
}

void FPostEffect::Process(AkU32 uThreadIndex, FCommandListPool* pCmdListPool, ID3D12CommandQueue* pCmdQueue, const D3D12_VIEWPORT* pViewport, const D3D12_RECT* pScissorRect)
{
	ID3D12GraphicsCommandList* pCmdList = pCmdListPool->GetCurrentCmdList();
	pCmdList->RSSetViewports(1, pViewport);
	pCmdList->RSSetScissorRects(1, pScissorRect);
	CD3DX12_CPU_DESCRIPTOR_HANDLE hRtv(_pRenderer->GetRtvHeap()->GetCPUDescriptorHandleForHeapStart(), SWAP_CHAIN_FRAME_COUNT + FRenderer::FLOAT16_BUFFER_COUNT + FRenderer::RESOLVED_BUFFER_COUNT, _pRenderer->GetRtvDescriptorSize());
	pCmdList->OMSetRenderTargets(1, &hRtv, AK_FALSE, nullptr);

	ID3D12Device* pDevice = _pRenderer->GetDevice();
	FDescriptorPool* pDescriptorPool = _pRenderer->GetDescriptorPool(uThreadIndex);
	ID3D12DescriptorHeap* pDescriptorHeap = pDescriptorPool->GetDescriptorHeap();
	FConstantBufferPool* pGlobalCBPool = _pRenderer->GetConstantBufferPool(uThreadIndex, CONSTANT_BUFFER_TYPE::CONSTANT_BUFFER_TYPE_GLOBAL);
	FConstantBufferPool* pPostEffectCBPool = _pRenderer->GetConstantBufferPool(uThreadIndex, CONSTANT_BUFFER_TYPE::CONSTANT_BUFFER_TYPE_POSTEFFECT);
	AkU32 uDescriptorSize = pDescriptorPool->GetDescriptorTypeSize();

	CD3DX12_CPU_DESCRIPTOR_HANDLE hCPU = {};
	CD3DX12_GPU_DESCRIPTOR_HANDLE hGPU = {};

	AkU32 uDecriptorCount = DESCRIPTOR_COUNT_PER_OBJ;
	if (!pDescriptorPool->AllocDescriptorTable(&hCPU, &hGPU, uDecriptorCount))
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
	pGlobalConstantBuffer->mInvProj = pGlobalConstantBuffer->mProj.Invert();

	AkU32 uLightNum = 0;
	Light_t* pLights = _pRenderer->GetLights(&uLightNum);
	memcpy(pGlobalConstantBuffer->tLights, pLights, sizeof(Light_t) * uLightNum);

	// Per Obj (b0).
	CD3DX12_CPU_DESCRIPTOR_HANDLE hDest(hCPU, 0, uDescriptorSize);
	pDevice->CopyDescriptorsSimple(1, hDest, pGlobalCBContainer->hCPU, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	hDest.Offset(1, uDescriptorSize);

	CBContainer_t* pPostEffectCBContainer = pPostEffectCBPool->Alloc();
	if (!pPostEffectCBContainer)
	{
		__debugbreak();
		return;
	}

	PostEffectConstantBuffer_t* pPostEffectConstantBuffer = reinterpret_cast<PostEffectConstantBuffer_t*>(pPostEffectCBContainer->pSystemMemAddr);
	pPostEffectConstantBuffer->iMode = _iEffectMode;
	pPostEffectConstantBuffer->fFogStrength =_fFogStrength;
	pPostEffectConstantBuffer->fDepthScale = _fDepthScale;

	// Per Obj (b1).
	pDevice->CopyDescriptorsSimple(1, hDest, pPostEffectCBContainer->hCPU, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	hDest.Offset(1, uDescriptorSize);

	// Per Obj (t0). => Resolved Buffer
	pCmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(_pRenderer->GetResolvedBuffer(), D3D12_RESOURCE_STATE_RESOLVE_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));
	pDevice->CopyDescriptorsSimple(1, hDest, _pRenderer->GetResolvedBufferSrvCpu(), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	pCmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(_pRenderer->GetResolvedBuffer(), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_RESOLVE_DEST));
	hDest.Offset(1, uDescriptorSize);

	// Per Obj (t1). => Depth Only Buffer.
	pDevice->CopyDescriptorsSimple(1, hDest, _pRenderer->GetDepthMapBufferSrvCpu(), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	hDest.Offset(1, uDescriptorSize);

	pCmdList->SetGraphicsRootSignature(_pRootSignature);
	pCmdList->SetDescriptorHeaps(1, &pDescriptorHeap);
	pCmdList->SetPipelineState(_pPostEffectPSO);

	// Obj (root param 0)
	pCmdList->SetGraphicsRootDescriptorTable(0, hGPU);
	pCmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	pCmdList->IASetVertexBuffers(0, 1, &_tVertexBufferView);
	pCmdList->IASetIndexBuffer(&_tIndexBufferView);
	pCmdList->DrawIndexedInstanced(6, 1, 0, 0, 0);

	pCmdListPool->Close();
	pCmdQueue->ExecuteCommandLists(1, (ID3D12CommandList**)&pCmdList);
}

void FPostEffect::CleanUp()
{
	_pRenderer->EnsureCompleted();

	DestroyMeshBuffers();

	DestroyPipelineState();

	DestroyRootSignature();
}

AkBool FPostEffect::CreateRootSignature()
{
	ID3D12Device* pDevice = _pRenderer->GetDevice();
	ID3DBlob* pSignature = nullptr;
	ID3DBlob* pError = nullptr;

	CD3DX12_DESCRIPTOR_RANGE tRanges[2] = {};
	tRanges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 2, 0); // b0, b1
	tRanges[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 2, 0); // t0, t1

	CD3DX12_ROOT_PARAMETER tRootParameters[1] = {};
	tRootParameters[0].InitAsDescriptorTable(_countof(tRanges), tRanges, D3D12_SHADER_VISIBILITY_ALL);

	// default sampler
	D3D12_STATIC_SAMPLER_DESC tSampler = {};
	FD3DUtils::SetDefaultSamplerDesc(&tSampler, 0);
	tSampler.AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	tSampler.AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	tSampler.AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;

	// Allow input layout and deny uneccessary access to certain pipeline stages.
	D3D12_ROOT_SIGNATURE_FLAGS tRootSignatureFlags =
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_PIXEL_SHADER_ROOT_ACCESS;

	CD3DX12_ROOT_SIGNATURE_DESC tRootSignatureDesc;
	//rootSignatureDesc.Init(0, nullptr, 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);
	tRootSignatureDesc.Init(_countof(tRootParameters), tRootParameters, 1, &tSampler, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	if (FAILED(D3D12SerializeRootSignature(&tRootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &pSignature, &pError)))
	{
		__debugbreak();
	}

	if (FAILED(pDevice->CreateRootSignature(0, pSignature->GetBufferPointer(), pSignature->GetBufferSize(), IID_PPV_ARGS(&_pRootSignature))))
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

AkBool FPostEffect::CreatePipelineState()
{
	ID3D12Device* pDeivce = _pRenderer->GetDevice();

	ID3DBlob* pSamplingVS = nullptr;
	ID3DBlob* pPostEffectPS = nullptr;

#if defined(_DEBUG)
	// Enable better shader debugging with the graphics debugging tools.
	AkU32 compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
	UINT compileFlags = 0;
#endif

	ID3DBlob* pErrorBlob = nullptr;
	if (FAILED(D3DCompileFromFile(L"../../shader/PostEffect.hlsl", nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "VSMain", "vs_5_0", compileFlags, 0, &pSamplingVS, &pErrorBlob)))
	{
		if (pErrorBlob != nullptr)
		{
			OutputDebugStringA((char*)pErrorBlob->GetBufferPointer());
			pErrorBlob->Release();
		}
		__debugbreak();
	}
	if (FAILED(D3DCompileFromFile(L"../../shader/PostEffect.hlsl", nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "PSMain", "ps_5_0", compileFlags, 0, &pPostEffectPS, &pErrorBlob)))
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
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,	0, 12,	D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
	};

	// Describe and create the graphics pipeline state object (PSO).
	D3D12_GRAPHICS_PIPELINE_STATE_DESC tPostProcessPsoDesc = {};
	tPostProcessPsoDesc.InputLayout = { tInputElementDescs, _countof(tInputElementDescs) };
	tPostProcessPsoDesc.pRootSignature = _pRootSignature;
	tPostProcessPsoDesc.VS = CD3DX12_SHADER_BYTECODE(pSamplingVS->GetBufferPointer(), pSamplingVS->GetBufferSize());
	tPostProcessPsoDesc.PS = CD3DX12_SHADER_BYTECODE(pPostEffectPS->GetBufferPointer(), pPostEffectPS->GetBufferSize());
	tPostProcessPsoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	tPostProcessPsoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	tPostProcessPsoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	tPostProcessPsoDesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
	tPostProcessPsoDesc.DepthStencilState.StencilEnable = FALSE;
	tPostProcessPsoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_BACK;
	tPostProcessPsoDesc.SampleMask = UINT_MAX;
	tPostProcessPsoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	tPostProcessPsoDesc.NumRenderTargets = 1;
	tPostProcessPsoDesc.RTVFormats[0] = _pRenderer->GetBackBufferRTVFormat();
	tPostProcessPsoDesc.DepthStencilState.DepthEnable = FALSE;
	tPostProcessPsoDesc.DepthStencilState.StencilEnable = FALSE;
	tPostProcessPsoDesc.SampleDesc.Count = 1;
	if (FAILED(pDeivce->CreateGraphicsPipelineState(&tPostProcessPsoDesc, IID_PPV_ARGS(&_pPostEffectPSO))))
	{
		__debugbreak();
	}

	if (pSamplingVS)
	{
		pSamplingVS->Release();
		pSamplingVS = nullptr;
	}
	if (pPostEffectPS)
	{
		pPostEffectPS->Release();
		pPostEffectPS = nullptr;
	}

	return AK_TRUE;
}

AkBool FPostEffect::CreateMeshBuffers()
{
	FResourceManager* pResourceManager = _pRenderer->GetResourceManager();

	VertexTexcoord_t tVertices[] =
	{
		{ { -1.0f, -1.0f, 0.0f }, { 0.0f, 1.0f } },
		{ { -1.0f, 1.0f, 0.0f }, { 0.0f, 0.0f } },
		{ { 1.0f, 1.0f, 0.0f }, { 1.0f, 0.0f } },
		{ { 1.0f, -1.0f, 0.0f }, { 1.0f, 1.0f } },
	};

	AkU32 tIndices[] =
	{
		0, 1, 2,
		0, 2, 3
	};

	const AkU32 uVertexBufferSize = sizeof(tVertices);

	if (FAILED(pResourceManager->CreateVertexBuffer(sizeof(VertexTexcoord_t), (AkU32)_countof(tVertices), &_tVertexBufferView, &_pVertexBuffer, tVertices)))
	{
		__debugbreak();
		return AK_FALSE;
	}

	if (FAILED(pResourceManager->CreateIndexBuffer(_countof(tIndices), &_tIndexBufferView, &_pIndexBuffer, tIndices)))
	{
		__debugbreak();
		return AK_FALSE;
	}

	return AK_TRUE;
}

void FPostEffect::DestroyRootSignature()
{
	if (_pRootSignature)
	{
		_pRootSignature->Release();
		_pRootSignature = nullptr;
	}
}

void FPostEffect::DestroyPipelineState()
{
	if (_pPostEffectPSO)
	{
		_pPostEffectPSO->Release();
		_pPostEffectPSO = nullptr;
	}
}

void FPostEffect::DestroyMeshBuffers()
{
	if (_pIndexBuffer)
	{
		_pIndexBuffer->Release();
		_pIndexBuffer = nullptr;
	}
	if (_pVertexBuffer)
	{
		_pVertexBuffer->Release();
		_pVertexBuffer = nullptr;
	}
}
