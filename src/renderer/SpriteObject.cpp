#include "pch.h"
#include "SpriteObject.h"
#include "Renderer.h"
#include "D3DUtils.h"
#include "ResourceManager.h"
#include "DescriptorAllocator.h"
#include "DescriptorPool.h"
#include "ConstantBufferPool.h"

/*
===============
SpriteObject
===============
*/

ID3D12RootSignature* FSpriteObject::sm_pRootSignature;
ID3D12PipelineState* FSpriteObject::sm_pDefaultPSO;
ID3D12PipelineState* FSpriteObject::sm_pAccumulatePSO;

ID3D12Resource* FSpriteObject::sm_pVertexBuffer;
D3D12_VERTEX_BUFFER_VIEW FSpriteObject::sm_tVertexBufferView;

ID3D12Resource* FSpriteObject::sm_pIndexBuffer;
D3D12_INDEX_BUFFER_VIEW FSpriteObject::sm_tIndexBufferView;

AkU32 FSpriteObject::sm_uInitRefCount;

AkBool FSpriteObject::CreateCommonResources()
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

void FSpriteObject::DestroyCommonResources()
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
		if (sm_pDefaultPSO)
		{
			sm_pDefaultPSO->Release();
			sm_pDefaultPSO = nullptr;
		}
		if (sm_pAccumulatePSO)
		{
			sm_pAccumulatePSO->Release();
			sm_pAccumulatePSO = nullptr;
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

AkBool FSpriteObject::CreateRootSignature()
{
	ID3D12Device* pDevice = _pRenderer->GetDevice();
	ID3DBlob* pSignature = nullptr;
	ID3DBlob* pError = nullptr;

	CD3DX12_DESCRIPTOR_RANGE tRanges[2] = {};
	tRanges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0);	// b0 : Constant Buffer View
	tRanges[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);	// t0 : Shader Resource View(Tex)

	CD3DX12_ROOT_PARAMETER tRootParameters[1] = {};
	tRootParameters[0].InitAsDescriptorTable(_countof(tRanges), tRanges, D3D12_SHADER_VISIBILITY_ALL);

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

	if (FAILED(pDevice->CreateRootSignature(0, pSignature->GetBufferPointer(), pSignature->GetBufferSize(), IID_PPV_ARGS(&sm_pRootSignature))))
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

AkBool FSpriteObject::CreatePipelineState()
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
	//m_pRenderer->SetCurrentPathForShader();

	ID3DBlob* pErrorBlob = nullptr;
	if (FAILED(D3DCompileFromFile(L"../../shader/SpriteShader.hlsl", nullptr, nullptr, "VSMain", "vs_5_0", compileFlags, 0, &pVertexShader, &pErrorBlob)))
	{
		if (pErrorBlob != nullptr)
		{
			OutputDebugStringA((char*)pErrorBlob->GetBufferPointer());
			pErrorBlob->Release();
		}
		__debugbreak();
	}
	if (FAILED(D3DCompileFromFile(L"../../shader/SpriteShader.hlsl", nullptr, nullptr, "PSMain", "ps_5_0", compileFlags, 0, &pPixelShader, &pErrorBlob)))
	{
		if (pErrorBlob != nullptr)
		{
			OutputDebugStringA((char*)pErrorBlob->GetBufferPointer());
			pErrorBlob->Release();
		}
		__debugbreak();
	}

	//m_pRenderer->RestoreCurrentPath();

	// Define the vertex input layout.
	D3D12_INPUT_ELEMENT_DESC tInputElementDescs[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,	0, 28,	D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
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
	//psoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
	tOpaquePsoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_BACK;
	tOpaquePsoDesc.SampleMask = UINT_MAX;
	tOpaquePsoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	tOpaquePsoDesc.NumRenderTargets = 1;
	tOpaquePsoDesc.RTVFormats[0] = _pRenderer->GetFloatRTVFormat();
	tOpaquePsoDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
	tOpaquePsoDesc.SampleDesc.Count = 1;
	if (FAILED(pDeivce->CreateGraphicsPipelineState(&tOpaquePsoDesc, IID_PPV_ARGS(&sm_pDefaultPSO))))
	{
		__debugbreak();
	}

	D3D12_GRAPHICS_PIPELINE_STATE_DESC tAccumulatePSODesc = tOpaquePsoDesc;

	D3D12_RENDER_TARGET_BLEND_DESC tTransparencyBlendDesc = {};
	tTransparencyBlendDesc.BlendEnable = AK_TRUE;
	tTransparencyBlendDesc.LogicOpEnable = AK_FALSE;
	tTransparencyBlendDesc.SrcBlend = D3D12_BLEND_ONE;
	tTransparencyBlendDesc.DestBlend = D3D12_BLEND_INV_SRC_COLOR;
	tTransparencyBlendDesc.BlendOp = D3D12_BLEND_OP_ADD;
	tTransparencyBlendDesc.SrcBlendAlpha = D3D12_BLEND_ONE;
	tTransparencyBlendDesc.DestBlendAlpha = D3D12_BLEND_ZERO;
	tTransparencyBlendDesc.BlendOpAlpha = D3D12_BLEND_OP_ADD;
	tTransparencyBlendDesc.LogicOp = D3D12_LOGIC_OP_NOOP;
	tTransparencyBlendDesc.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

	tAccumulatePSODesc.BlendState.AlphaToCoverageEnable = AK_TRUE;
	tAccumulatePSODesc.BlendState.RenderTarget[0] = tTransparencyBlendDesc;

	if (FAILED(pDeivce->CreateGraphicsPipelineState(&tAccumulatePSODesc, IID_PPV_ARGS(&sm_pAccumulatePSO))))
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

AkBool FSpriteObject::CreateMesh()
{
	// 바깥에서 버텍스데이터와 텍스처를 입력하는 식으로 변경할 것
	ID3D12Device* pDevice = _pRenderer->GetDevice();
	AkU32 uSrvDescriptorSize = pDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	FResourceManager* pResourceManager = _pRenderer->GetResourceManager();
	FDescriptorAllocator* pSingleDescriptorAllocator = _pRenderer->GetDescriptorAllocator();

	// Create the vertex buffer.
	// Define the geometry for a triangle.
	SpriteVertex_t tVertices[] =
	{
		{ { 0.0f, 1.0f, 0.0f }, { 1.0f, 1.0f, 1.0f, 1.0f }, { 0.0f, 1.0f } },
		{ { 0.0f, 0.0f, 0.0f }, { 1.0f, 1.0f, 1.0f, 1.0f }, { 0.0f, 0.0f } },
		{ { 1.0f, 0.0f, 0.0f }, { 1.0f, 1.0f, 1.0f, 1.0f }, { 1.0f, 0.0f } },
		{ { 1.0f, 1.0f, 0.0f }, { 1.0f, 1.0f, 1.0f, 1.0f }, { 1.0f, 1.0f } },
	};


	AkU32 tIndices[] =
	{
		0, 1, 2,
		0, 2, 3
	};

	const AkU32 uVertexBufferSize = sizeof(tVertices);

	if (FAILED(pResourceManager->CreateVertexBuffer(sizeof(SpriteVertex_t), (AkU32)_countof(tVertices), &sm_tVertexBufferView, &sm_pVertexBuffer, tVertices)))
	{
		__debugbreak();
		return AK_FALSE;
	}

	if (FAILED(pResourceManager->CreateIndexBuffer(_countof(tIndices), &sm_tIndexBufferView, &sm_pIndexBuffer, tIndices)))
	{
		__debugbreak();
		return AK_FALSE;
	}

	return AK_TRUE;
}

void FSpriteObject::CleanUp()
{
	if (_pTexHandle)
	{
		_pRenderer->DestroyTexture(_pTexHandle);
		_pTexHandle = nullptr;
	}
	DestroyCommonResources();
}

HRESULT STDMETHODCALLTYPE FSpriteObject::QueryInterface(REFIID, void** ppv)
{
	return E_NOTIMPL;
}

ULONG STDMETHODCALLTYPE FSpriteObject::AddRef()
{
	_uRefCount++;
	return _uRefCount;
}

ULONG STDMETHODCALLTYPE FSpriteObject::Release()
{
	AkU32	ref_count = --_uRefCount;
	if (!_uRefCount)
		delete this;

	return ref_count;
}

AkBool FSpriteObject::Initialize(FRenderer* pRenderer)
{
	_pRenderer = pRenderer;

	AkBool bResult = CreateCommonResources();
	return bResult;
}

AkBool FSpriteObject::Initialize(FRenderer* pRenderer, const wchar_t* wcTexFileName, const RECT* pRect)
{
	_pRenderer = pRenderer;

	AkBool bResult = (CreateCommonResources() != 0);
	if (bResult)
	{
		AkU32 uTexWidth = 1;
		AkU32 uTexHeight = 1;
		_pTexHandle = (TextureHandle_t*)_pRenderer->CreateTextureFromFile(wcTexFileName, AK_TRUE);
		
		//// For Debugging.
		//printf("Sprite Tex [%p]\n", _pTexHandle);

		if (_pTexHandle)
		{
			D3D12_RESOURCE_DESC tDesc = _pTexHandle->pTextureResource->GetDesc();
			uTexWidth = (AkU32)tDesc.Width;
			uTexHeight = (AkU32)tDesc.Height;
		}
		if (pRect)
		{
			_vRect = *pRect;
			_vScale.x = (AkF32)(_vRect.right - _vRect.left) / (AkF32)uTexWidth;
			_vScale.y = (AkF32)(_vRect.bottom - _vRect.top) / (AkF32)uTexHeight;
		}
		else
		{
			if (_pTexHandle)
			{
				D3D12_RESOURCE_DESC tDesc = _pTexHandle->pTextureResource->GetDesc();
				_vRect.left = 0;
				_vRect.top = 0;
				_vRect.right = (LONG)tDesc.Width;
				_vRect.bottom = (LONG)tDesc.Height;
			}
		}
	}
	return bResult;
}

void FSpriteObject::DrawWithTex(AkU32 uThreadIndex, ID3D12GraphicsCommandList* pCommandList, const Vector2* pPos, const Vector2* pScale, const RECT* pRect, float Z, TextureHandle_t* pTexHandle, const Vector3* pFontColor)
{
	// 각각의 draw()작업의 무결성을 보장하려면 draw() 작업마다 다른 영역의 descriptor table(shader visible)과 다른 영역의 CBV를 사용해야 한다.
	// 따라서 draw()할 때마다 CBV는 ConstantBuffer Pool로부터 할당받고, 렌더리용 descriptor table(shader visible)은 descriptor pool로부터 할당 받는다.

	ID3D12Device* pDeivce = _pRenderer->GetDevice();
	FDescriptorPool* pDescriptorPool = _pRenderer->GetDescriptorPool(uThreadIndex);
	ID3D12DescriptorHeap* pDescriptorHeap = pDescriptorPool->GetDescriptorHeap();
	FConstantBufferPool* pConstantBufferPool = _pRenderer->GetConstantBufferPool(uThreadIndex, CONSTANT_BUFFER_TYPE::CONSTANT_BUFFER_TYPE_SPRITE);
	AkU32 uDescriptorSize = pDescriptorPool->GetDescriptorTypeSize();

	AkU32 uTexWidth = 0;
	AkU32 uTexHeight = 0;

	D3D12_CPU_DESCRIPTOR_HANDLE hSRV = {};
	if (pTexHandle)
	{
		D3D12_RESOURCE_DESC tDesc = pTexHandle->pTextureResource->GetDesc();
		uTexWidth = (AkU32)tDesc.Width;
		uTexHeight = (AkU32)tDesc.Height;
		hSRV = pTexHandle->hSRV;
	}

	RECT tRect;
	if (!pRect)
	{
		tRect.left = 0;
		tRect.top = 0;
		tRect.right = uTexWidth;
		tRect.bottom = uTexHeight;
		pRect = &tRect;
	}

	CD3DX12_GPU_DESCRIPTOR_HANDLE hGPU = {};
	CD3DX12_CPU_DESCRIPTOR_HANDLE hCPU = {};

	if (!pDescriptorPool->AllocDescriptorTable(&hCPU, &hGPU, DESCRIPTOR_COUNT_FOR_DRAW))
	{
		__debugbreak();
	}

	// 각각의 draw()에 대해 독립적인 constant buffer(내부적으로는 같은 resource의 다른 영역)를 사용한다.
	CBContainer_t* pCB = pConstantBufferPool->Alloc();
	if (!pCB)
	{
		__debugbreak();
	}
	SpriteConstantBuffer_t* pConstantBufferSprite = (SpriteConstantBuffer_t*)pCB->pSystemMemAddr;

	// constant buffer의 내용을 설정
	pConstantBufferSprite->vScreenRes.x = (AkF32)_pRenderer->GetScreenWidth();
	pConstantBufferSprite->vScreenRes.y = (AkF32)_pRenderer->GetSreenHeight();
	pConstantBufferSprite->vPos = *pPos;
	pConstantBufferSprite->vScale = *pScale;
	pConstantBufferSprite->vTexSize.x = (AkF32)uTexWidth;
	pConstantBufferSprite->vTexSize.y = (AkF32)uTexHeight;
	pConstantBufferSprite->vTexSampePos.x = (AkF32)pRect->left;
	pConstantBufferSprite->vTexSampePos.y = (AkF32)pRect->top;
	pConstantBufferSprite->vTexSampleSize.x = (AkF32)(pRect->right - pRect->left);
	pConstantBufferSprite->vTexSampleSize.y = (AkF32)(pRect->bottom - pRect->top);
	pConstantBufferSprite->fZ = Z;
	pConstantBufferSprite->fAlpha = 1.0f;
	pConstantBufferSprite->uDrawBackground = _bDrawBackground;
	if (pFontColor)
	{
		pConstantBufferSprite->vFontColor = *pFontColor;
	}
	else
	{
		pConstantBufferSprite->vFontColor = Vector3(1.0f);
	}

	// set RootSignature
	pCommandList->SetGraphicsRootSignature(sm_pRootSignature);
	pCommandList->SetDescriptorHeaps(1, &pDescriptorHeap);

	// Descriptor Table 구성
	// 이번에 사용할 constant buffer의 descriptor를 렌더링용(shader visible) descriptor table에 카피

	CD3DX12_CPU_DESCRIPTOR_HANDLE hDest(hCPU, AkU32(SPRITE_DESCRIPTOR_INDEX::SPRITE_DESCRIPTOR_INDEX_CBV), uDescriptorSize);
	pDeivce->CopyDescriptorsSimple(1, hDest, pCB->hCPU, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	if (hSRV.ptr)
	{
		CD3DX12_CPU_DESCRIPTOR_HANDLE srvDest(hCPU, AkU32(SPRITE_DESCRIPTOR_INDEX::SPRITE_DESCRIPTOR_INDEX_TEX), uDescriptorSize);
		pDeivce->CopyDescriptorsSimple(1, srvDest, hSRV, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	}

	pCommandList->SetGraphicsRootDescriptorTable(0, hGPU);

	pCommandList->SetPipelineState(sm_pDefaultPSO);
	pCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	pCommandList->IASetVertexBuffers(0, 1, &sm_tVertexBufferView);
	pCommandList->IASetIndexBuffer(&sm_tIndexBufferView);
	pCommandList->DrawIndexedInstanced(6, 1, 0, 0, 0);
}

void FSpriteObject::Draw(AkU32 uThreadIndex, ID3D12GraphicsCommandList* pCommandList, const Vector2* pPos, const Vector2* pScale, float Z)
{
	Vector2 Scale = { _vScale.x * pScale->x, _vScale.y * pScale->y };
	DrawWithTex(uThreadIndex, pCommandList, pPos, &Scale, &_vRect, Z, _pTexHandle, nullptr);
}

FSpriteObject::FSpriteObject()
{
}

FSpriteObject::~FSpriteObject()
{
	CleanUp();
}
