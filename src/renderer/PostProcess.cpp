#include "pch.h"
#include "PostProcess.h"
#include "Renderer.h"
#include "DescriptorPool.h"
#include "D3DUtils.h"
#include "ResourceManager.h"

/*
===================
PostProcess
===================
*/

FPostProcess::FPostProcess()
{
}

FPostProcess::~FPostProcess()
{
	CleanUp();
}

AkBool FPostProcess::Initialize(FRenderer* pRenderer)
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

void FPostProcess::ApplyPostProcess(AkU32 uThreadIndex, ID3D12GraphicsCommandList* pCmdList, D3D12_CPU_DESCRIPTOR_HANDLE hBackBufferRTV, const D3D12_VIEWPORT* pViewport, const D3D12_RECT* pScissorRect)
{
	ID3D12Device* pDevice = _pRenderer->GetDevice();
	FDescriptorPool* pDescriptorPool = _pRenderer->GetDescriptorPool(uThreadIndex);
	ID3D12DescriptorHeap* pDescriptorHeap = pDescriptorPool->GetDescriptorHeap();
	AkU32 uDescriptorSize = pDescriptorPool->GetDescriptorTypeSize();

	const AkF32 fClearColor[] = { 0.0f, 0.0f, 1.0f, 1.0f };
	pCmdList->ClearRenderTargetView(hBackBufferRTV, fClearColor, 0, nullptr);

	pCmdList->RSSetViewports(1, pViewport);
	pCmdList->RSSetScissorRects(1, pScissorRect);
	pCmdList->OMSetRenderTargets(1, &hBackBufferRTV, AK_FALSE, nullptr);

	CD3DX12_CPU_DESCRIPTOR_HANDLE hCPU = {};
	CD3DX12_GPU_DESCRIPTOR_HANDLE hGPU = {};
	if (!pDescriptorPool->AllocDescriptorTable(&hCPU, &hGPU, 1))
	{
		__debugbreak();
		return;
	}

	CD3DX12_CPU_DESCRIPTOR_HANDLE hDest(hCPU, 0, uDescriptorSize);
	pDevice->CopyDescriptorsSimple(1, hDest, _pRenderer->GetFloatBufferSrvCpu(), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	pCmdList->SetGraphicsRootSignature(_pRootSignature);
	pCmdList->SetDescriptorHeaps(1, &pDescriptorHeap);
	pCmdList->SetPipelineState(_pPostProcessPSO);

	pCmdList->SetGraphicsRootDescriptorTable(0, hGPU);
	pCmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	pCmdList->IASetVertexBuffers(0, 1, &_tVertexBufferView);
	pCmdList->IASetIndexBuffer(&_tIndexBufferView);
	pCmdList->DrawIndexedInstanced(6, 1, 0, 0, 0);
}

void FPostProcess::CleanUp()
{
	DestroyMeshBuffers();

	DestroyPipelineState();

	DestroyRootSignature();
}

AkBool FPostProcess::CreateRootSignature()
{
	ID3D12Device* pDevice = _pRenderer->GetDevice();
	ID3DBlob* pSignature = nullptr;
	ID3DBlob* pError = nullptr;

	CD3DX12_DESCRIPTOR_RANGE tRanges[1] = {};
	tRanges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);

	CD3DX12_ROOT_PARAMETER tRootParameters[1] = {};
	tRootParameters[0].InitAsDescriptorTable(_countof(tRanges), tRanges, D3D12_SHADER_VISIBILITY_ALL);

	// default sampler
	D3D12_STATIC_SAMPLER_DESC tSampler = {};
	FD3DUtils::SetDefaultSamplerDesc(&tSampler, 0);
	// tSampler.Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;

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

AkBool FPostProcess::CreatePipelineState()
{
	ID3D12Device* pDeivce = _pRenderer->GetDevice();

	ID3DBlob* pVertexShader = nullptr;
	ID3DBlob* pPixelShader = nullptr;

#if defined(_DEBUG)
	// Enable better shader debugging with the graphics debugging tools.
	AkU32 compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
	UINT compileFlags = 0;
#endif

	ID3DBlob* pErrorBlob = nullptr;
	if (FAILED(D3DCompileFromFile(L"../../shader/PostProcessShader.hlsl", nullptr, nullptr, "VSMain", "vs_5_0", compileFlags, 0, &pVertexShader, &pErrorBlob)))
	{
		if (pErrorBlob != nullptr)
		{
			OutputDebugStringA((char*)pErrorBlob->GetBufferPointer());
			pErrorBlob->Release();
		}
		__debugbreak();
	}
	if (FAILED(D3DCompileFromFile(L"../../shader/PostProcessShader.hlsl", nullptr, nullptr, "PSMain", "ps_5_0", compileFlags, 0, &pPixelShader, &pErrorBlob)))
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
	tPostProcessPsoDesc.VS = CD3DX12_SHADER_BYTECODE(pVertexShader->GetBufferPointer(), pVertexShader->GetBufferSize());
	tPostProcessPsoDesc.PS = CD3DX12_SHADER_BYTECODE(pPixelShader->GetBufferPointer(), pPixelShader->GetBufferSize());
	tPostProcessPsoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	tPostProcessPsoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	tPostProcessPsoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	tPostProcessPsoDesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
	tPostProcessPsoDesc.DepthStencilState.StencilEnable = FALSE;
	//psoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
	tPostProcessPsoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_BACK;
	tPostProcessPsoDesc.SampleMask = UINT_MAX;
	tPostProcessPsoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	tPostProcessPsoDesc.NumRenderTargets = 1;
	tPostProcessPsoDesc.RTVFormats[0] = _pRenderer->GetBackBufferRTVFormat();
	tPostProcessPsoDesc.DepthStencilState.DepthEnable = FALSE;
	tPostProcessPsoDesc.DepthStencilState.StencilEnable = FALSE;
	tPostProcessPsoDesc.SampleDesc.Count = 1;
	if (FAILED(pDeivce->CreateGraphicsPipelineState(&tPostProcessPsoDesc, IID_PPV_ARGS(&_pPostProcessPSO))))
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

AkBool FPostProcess::CreateMeshBuffers()
{
	FResourceManager* pResourceManager = _pRenderer->GetResourceManager();

	PostProcessVertex_t tVertices[] =
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

	if (FAILED(pResourceManager->CreateVertexBuffer(sizeof(PostProcessVertex_t), (AkU32)_countof(tVertices), &_tVertexBufferView, &_pVertexBuffer, tVertices)))
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

void FPostProcess::DestroyRootSignature()
{
	if (_pRootSignature)
	{
		_pRootSignature->Release();
		_pRootSignature = nullptr;
	}
}

void FPostProcess::DestroyPipelineState()
{
	if (_pPostProcessPSO)
	{
		_pPostProcessPSO->Release();
		_pPostProcessPSO = nullptr;
	}
}

void FPostProcess::DestroyMeshBuffers()
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
