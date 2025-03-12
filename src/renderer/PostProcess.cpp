#include "pch.h"
#include "PostProcess.h"
#include "Renderer.h"
#include "DescriptorPool.h"
#include "DescriptorAllocator.h"
#include "D3DUtils.h"
#include "ResourceManager.h"
#include "ImageFiler.h"
#include "ConstantBufferManager.h"
#include "ConstantBufferPool.h"

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

AkBool FPostProcess::Initialize(FRenderer* pRenderer, AkU32 uBloomLevels, AkU32 uWidth, AkU32 uHeight)
{
	_pRenderer = pRenderer;
	_uBloomLevel = uBloomLevels;

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
	if (!CreateBuffers(uWidth, uHeight))
	{
		__debugbreak();
		return AK_FALSE;
	}

	return AK_TRUE;
}

void FPostProcess::Process(AkU32 uThreadIndex, ID3D12GraphicsCommandList* pCmdList, D3D12_CPU_DESCRIPTOR_HANDLE hBackBufferRTV, const D3D12_VIEWPORT* pViewport, const D3D12_RECT* pScissorRect)
{
	ID3D12Device* pDevice = _pRenderer->GetDevice();
	FDescriptorPool* pDescriptorPool = _pRenderer->GetDescriptorPool(uThreadIndex);
	ID3D12DescriptorHeap* pDescriptorHeap = pDescriptorPool->GetDescriptorHeap();
	AkU32 uDescriptorSize = pDescriptorPool->GetDescriptorTypeSize();

	CD3DX12_CPU_DESCRIPTOR_HANDLE hCPU = {};
	CD3DX12_GPU_DESCRIPTOR_HANDLE hGPU = {};
	
	AkU32 uDecriptorCount = DESCRIPTOR_COUNT_PER_BLOOM * _uBloomLevel + DESCCIPTOR_COUNT_COMBINE;
	if (!pDescriptorPool->AllocDescriptorTable(&hCPU, &hGPU, uDecriptorCount))
	{
		__debugbreak();
		return;
	}

	pCmdList->SetDescriptorHeaps(1, &pDescriptorHeap);

	for (AkU32 i = 0; i < _uBloomLevel - 1; i++)
	{
		if (0 == i)
		{
			pCmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(_pRenderer->GetResolvedBuffer(), D3D12_RESOURCE_STATE_RESOLVE_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));
		}
		else
		{
			pCmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(_ppBloomBuffers[i], D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));
		}

		pCmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(_ppBloomBuffers[i + 1], D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET));

		RenderImageFilter(uThreadIndex, pCmdList, _ppBloomDownFilters[i], &hCPU, &hGPU); // 1 x(_uBloomLevel - 1)
	}
	for (AkU32 i = 0; i < _uBloomLevel - 1; i++)
	{
		AkU32 uLevel = _uBloomLevel - 2 - i;

		pCmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(_ppBloomBuffers[uLevel + 1], D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));
		pCmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(_ppBloomBuffers[uLevel], D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET));

		RenderImageFilter(uThreadIndex, pCmdList, _ppBloomUpFilters[i], &hCPU, &hGPU); // 1 x (_uBloomLevel - 1)
	}

	pCmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(_ppBloomBuffers[0], D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));

	_pCombineFilter->SetRtvCpu(&_pRenderer->GetBackBufferRtvCpu()); // Combine shader 의 경우 Backbuffer 의 내용을 쓰기때문에 매 프레임 업데이트 필요!!

	RenderImageFilter(uThreadIndex, pCmdList, _pCombineFilter, &hCPU, &hGPU); // 2

	pCmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(_pRenderer->GetResolvedBuffer(), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_RESOLVE_DEST));
}

AkBool FPostProcess::CreateBuffers(AkU32 uWidth, AkU32 uHeight)
{
	// 기존 리소스 해제 필요
	DestroyBuffer();
	DestroyImageFilters();

	AkU32 uBloomLevels = _uBloomLevel;

	_ppBloomBuffers = new ID3D12Resource * [uBloomLevels];

	for (AkU32 i = 0; i < uBloomLevels; i++)
	{
		AkU32 uDiv = (AkU32)pow(2, i);
		CreateBuffer(uWidth / uDiv, uHeight / uDiv, &_ppBloomBuffers[i], _hSrvCpu + i, _hRtvCpu + i, i);
	}

	_ppBloomDownFilters = new FImageFilter * [uBloomLevels - 1];
	_ppBloomUpFilters = new FImageFilter * [uBloomLevels - 1];

	CreateImageFilters(uWidth, uHeight);

	return AK_TRUE;
}

void FPostProcess::RenderImageFilter(AkU32 uThreadIndex, ID3D12GraphicsCommandList* pCmdList, FImageFilter* pImageFilter, CD3DX12_CPU_DESCRIPTOR_HANDLE* pCPU, CD3DX12_GPU_DESCRIPTOR_HANDLE* pGPU)
{
	pImageFilter->Draw(uThreadIndex, pCmdList, pCPU, pGPU);
	
	pCmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	pCmdList->IASetVertexBuffers(0, 1, &_tVertexBufferView);
	pCmdList->IASetIndexBuffer(&_tIndexBufferView);
	pCmdList->DrawIndexedInstanced(6, 1, 0, 0, 0);
}

void FPostProcess::CleanUp()
{
	DestroyImageFilters();

	DestroyBuffer();

	DestroyMeshBuffers();

	DestroyPipelineState();

	DestroyRootSignature();
}

AkBool FPostProcess::CreateRootSignature()
{
	ID3D12Device* pDevice = _pRenderer->GetDevice();
	ID3DBlob* pSignature = nullptr;
	ID3DBlob* pError = nullptr;

	// Up Down Filter Root signature.
	{
		CD3DX12_DESCRIPTOR_RANGE tRanges[2] = {};
		tRanges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0); // b0
		tRanges[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0); // t0

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

		if (FAILED(pDevice->CreateRootSignature(0, pSignature->GetBufferPointer(), pSignature->GetBufferSize(), IID_PPV_ARGS(&_pUpDownFilterRootSignature))))
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
	}
	// Combine Filter Root signature
	{
		CD3DX12_DESCRIPTOR_RANGE tRanges[2] = {};
		tRanges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0); // b0
		tRanges[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 2, 0); // t0, t1

		CD3DX12_ROOT_PARAMETER tRootParameters[1] = {};
		tRootParameters[0].InitAsDescriptorTable(_countof(tRanges), tRanges, D3D12_SHADER_VISIBILITY_ALL);

		// default sampler
		D3D12_STATIC_SAMPLER_DESC tSampler = {};
		FD3DUtils::SetDefaultSamplerDesc(&tSampler, 0);

		// Allow input layout and deny uneccessary access to certain pipeline stages.
		D3D12_ROOT_SIGNATURE_FLAGS tRootSignatureFlags =
			D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
			D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
			D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
			D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS |
			D3D12_ROOT_SIGNATURE_FLAG_DENY_PIXEL_SHADER_ROOT_ACCESS;

		CD3DX12_ROOT_SIGNATURE_DESC tRootSignatureDesc;
		tRootSignatureDesc.Init(_countof(tRootParameters), tRootParameters, 1, &tSampler, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

		if (FAILED(D3D12SerializeRootSignature(&tRootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &pSignature, &pError)))
		{
			__debugbreak();
		}

		if (FAILED(pDevice->CreateRootSignature(0, pSignature->GetBufferPointer(), pSignature->GetBufferSize(), IID_PPV_ARGS(&_pCombineRootSignature))))
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
	}

	return AK_TRUE;
}

AkBool FPostProcess::CreatePipelineState()
{
	ID3D12Device* pDeivce = _pRenderer->GetDevice();

	ID3DBlob* pSamplingVS = nullptr;
	ID3DBlob* pCombinePS = nullptr;
	ID3DBlob* pBloomUpPS = nullptr;
	ID3DBlob* pBloomDownPS = nullptr;

#if defined(_DEBUG)
	// Enable better shader debugging with the graphics debugging tools.
	AkU32 compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
	UINT compileFlags = 0;
#endif

	ID3DBlob* pErrorBlob = nullptr;
	if (FAILED(D3DCompileFromFile(L"../../shader/PostProcessShader.hlsl", nullptr, nullptr, "VSMain", "vs_5_0", compileFlags, 0, &pSamplingVS, &pErrorBlob)))
	{
		if (pErrorBlob != nullptr)
		{
			OutputDebugStringA((char*)pErrorBlob->GetBufferPointer());
			pErrorBlob->Release();
		}
		__debugbreak();
	}
	if (FAILED(D3DCompileFromFile(L"../../shader/PostProcessShader.hlsl", nullptr, nullptr, "PSMain", "ps_5_0", compileFlags, 0, &pCombinePS, &pErrorBlob)))
	{
		if (pErrorBlob != nullptr)
		{
			OutputDebugStringA((char*)pErrorBlob->GetBufferPointer());
			pErrorBlob->Release();
		}
		__debugbreak();
	}
	if (FAILED(D3DCompileFromFile(L"../../shader/BloomUpShader.hlsl", nullptr, nullptr, "PSMain", "ps_5_0", compileFlags, 0, &pBloomUpPS, &pErrorBlob)))
	{
		if (pErrorBlob != nullptr)
		{
			OutputDebugStringA((char*)pErrorBlob->GetBufferPointer());
			pErrorBlob->Release();
		}
		__debugbreak();
	}
	if (FAILED(D3DCompileFromFile(L"../../shader/BloomDownShader.hlsl", nullptr, nullptr, "PSMain", "ps_5_0", compileFlags, 0, &pBloomDownPS, &pErrorBlob)))
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
	tPostProcessPsoDesc.pRootSignature = _pCombineRootSignature;
	tPostProcessPsoDesc.VS = CD3DX12_SHADER_BYTECODE(pSamplingVS->GetBufferPointer(), pSamplingVS->GetBufferSize());
	tPostProcessPsoDesc.PS = CD3DX12_SHADER_BYTECODE(pCombinePS->GetBufferPointer(), pCombinePS->GetBufferSize());
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
	if (FAILED(pDeivce->CreateGraphicsPipelineState(&tPostProcessPsoDesc, IID_PPV_ARGS(&_pCombinePSO))))
	{
		__debugbreak();
	}

	tPostProcessPsoDesc.pRootSignature = _pUpDownFilterRootSignature;
	tPostProcessPsoDesc.PS = CD3DX12_SHADER_BYTECODE(pBloomUpPS->GetBufferPointer(), pBloomUpPS->GetBufferSize());
	tPostProcessPsoDesc.RTVFormats[0] = _pRenderer->GetFloatRTVFormat();
	if (FAILED(pDeivce->CreateGraphicsPipelineState(&tPostProcessPsoDesc, IID_PPV_ARGS(&_pBloomUpPSO))))
	{
		__debugbreak();
	}

	tPostProcessPsoDesc.PS = CD3DX12_SHADER_BYTECODE(pBloomDownPS->GetBufferPointer(), pBloomDownPS->GetBufferSize());
	if (FAILED(pDeivce->CreateGraphicsPipelineState(&tPostProcessPsoDesc, IID_PPV_ARGS(&_pBloomDownPSO))))
	{
		__debugbreak();
	}

	if (pBloomDownPS)
	{
		pBloomDownPS->Release();
		pBloomDownPS = nullptr;
	}
	if (pBloomUpPS)
	{
		pBloomUpPS->Release();
		pBloomUpPS = nullptr;
	}
	if (pSamplingVS)
	{
		pSamplingVS->Release();
		pSamplingVS = nullptr;
	}
	if (pCombinePS)
	{
		pCombinePS->Release();
		pCombinePS = nullptr;
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

AkBool FPostProcess::CreateImageFilters(AkU32 uWidth, AkU32 uHeight)
{
	AkU32 uBloomLevels = _uBloomLevel;

	// Bloom Down.
	for (AkU32 i = 0; i < uBloomLevels - 1; i++)
	{
		AkU32 uDiv = (AkU32)pow(2, i + 1);
		_ppBloomDownFilters[i] = new FImageFilter;
		_ppBloomDownFilters[i]->Initialize(_pRenderer, uWidth / uDiv, uHeight / uDiv, _pUpDownFilterRootSignature, _pBloomDownPSO);
		if (0 == i)
		{
			_ppBloomDownFilters[i]->SetSrvCpu(&_pRenderer->GetResolvedBufferSrvCpu());
		}
		else
		{
			_ppBloomDownFilters[i]->SetSrvCpu(&_hSrvCpu[i]);
		}
		_ppBloomDownFilters[i]->SetRtvCpu(&_hRtvCpu[i + 1]);
	}

	// Bloom Up
	for (AkU32 i = 0; i < uBloomLevels - 1; i++)
	{
		AkU32 uLevel = uBloomLevels - 2 - i;
		AkU32 uDiv = (AkU32)pow(2, uLevel);
		_ppBloomUpFilters[i] = new FImageFilter;
		_ppBloomUpFilters[i]->Initialize(_pRenderer, uWidth / uDiv, uHeight / uDiv, _pUpDownFilterRootSignature, _pBloomUpPSO);
		_ppBloomUpFilters[i]->SetSrvCpu(&_hSrvCpu[uLevel + 1]);
		_ppBloomUpFilters[i]->SetRtvCpu(&_hRtvCpu[uLevel]);
	}

	// Combine + ToneMApping
	D3D12_CPU_DESCRIPTOR_HANDLE pSrvCpus[] =
	{
		_pRenderer->GetResolvedBufferSrvCpu(),
		_hSrvCpu[0],
	};
	_pCombineFilter = new FImageFilter;
	_pCombineFilter->Initialize(_pRenderer, uWidth, uHeight, _pCombineRootSignature, _pCombinePSO);
	_pCombineFilter->SetSrvCpu(pSrvCpus, _countof(pSrvCpus));
	_pCombineFilter->SetRtvCpu(&_pRenderer->GetBackBufferRtvCpu());

	return AkBool();
}

void FPostProcess::DestroyRootSignature()
{
	if (_pCombineRootSignature)
	{
		_pCombineRootSignature->Release();
		_pCombineRootSignature = nullptr;
	}
	if (_pUpDownFilterRootSignature)
	{
		_pUpDownFilterRootSignature->Release();
		_pUpDownFilterRootSignature = nullptr;
	}
}

void FPostProcess::DestroyPipelineState()
{
	if (_pBloomDownPSO)
	{
		_pBloomDownPSO->Release();
		_pBloomDownPSO = nullptr;
	}
	if (_pBloomUpPSO)
	{
		_pBloomUpPSO->Release();
		_pBloomUpPSO = nullptr;
	}
	if (_pCombinePSO)
	{
		_pCombinePSO->Release();
		_pCombinePSO = nullptr;
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

void FPostProcess::DestroyImageFilters()
{
	if (_pCombineFilter)
	{
		delete _pCombineFilter;
		_pCombineFilter = nullptr;
	}
	if (_ppBloomUpFilters)
	{
		for (AkU32 i = 0; i < _uBloomLevel - 1; i++)
		{
			if (_ppBloomUpFilters[i])
			{
				delete _ppBloomUpFilters[i];
				_ppBloomUpFilters[i] = nullptr;
			}
		}

		delete[] _ppBloomUpFilters;
		_ppBloomUpFilters = nullptr;
	}
	if (_ppBloomDownFilters)
	{
		for (AkU32 i = 0; i < _uBloomLevel - 1; i++)
		{
			if (_ppBloomDownFilters[i])
			{
				delete _ppBloomDownFilters[i];
				_ppBloomDownFilters[i] = nullptr;
			}
		}

		delete[] _ppBloomDownFilters;
		_ppBloomDownFilters = nullptr;
	}
}

void FPostProcess::DestroyBuffer()
{
	if (_ppBloomBuffers)
	{
		for (AkU32 i = 0; i < _uBloomLevel; i++)
		{
			if (_ppBloomBuffers[i])
			{
				_ppBloomBuffers[i]->Release();
				_ppBloomBuffers[i] = nullptr;
			}
		}
		delete[]  _ppBloomBuffers;
		_ppBloomBuffers = nullptr;
	}
}

void FPostProcess::CreateBuffer(AkU32 uWidth, AkU32 uHeight, ID3D12Resource** ppOutBuffer, D3D12_CPU_DESCRIPTOR_HANDLE* pOutSrvCpu, D3D12_CPU_DESCRIPTOR_HANDLE* pOutRtvCpu, AkU32 uIndex)
{
	ID3D12Device* pDevice = _pRenderer->GetDevice();
	FDescriptorAllocator* pDescriptorAllocator = _pRenderer->GetDescriptorAllocator();
	ID3D12Resource* pResource = nullptr;

	D3D12_RESOURCE_DESC tRtvDesc = {};
	tRtvDesc.Width = uWidth;
	tRtvDesc.Height = uHeight;
	tRtvDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	tRtvDesc.MipLevels = tRtvDesc.DepthOrArraySize = 1;
	tRtvDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
	tRtvDesc.SampleDesc.Count = 1;
	tRtvDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;

	D3D12_CLEAR_VALUE tClearValue = {};
	memcpy(tClearValue.Color, _pRenderer->GetRTVClearColor(), sizeof(_pRenderer->GetRTVClearColor()));
	tClearValue.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;

	if (FAILED(pDevice->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT), D3D12_HEAP_FLAG_NONE, &tRtvDesc, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, &tClearValue, IID_PPV_ARGS(&pResource))))
	{
		__debugbreak();
	}

	CD3DX12_CPU_DESCRIPTOR_HANDLE hRtvCpu(_pRenderer->GetRtvHeap()->GetCPUDescriptorHandleForHeapStart(), 5 + uIndex, _pRenderer->GetRtvDescriptorSize());

	pDevice->CreateRenderTargetView(pResource, nullptr, hRtvCpu);

	CD3DX12_CPU_DESCRIPTOR_HANDLE hSrvCpu = {};
	if (pDescriptorAllocator->AllocDescriptorHandle(&hSrvCpu))
	{
		pDevice->CreateShaderResourceView(pResource, nullptr, hSrvCpu);
	}

	*pOutRtvCpu = hRtvCpu;
	*pOutSrvCpu = hSrvCpu;
	*ppOutBuffer = pResource;
}
