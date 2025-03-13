#include "pch.h"
#include "ImageFiler.h"
#include "Renderer.h"
#include "ConstantBufferManager.h"
#include "ConstantBufferPool.h"

/*
============
Image Filer
============
*/

FImageFilter::FImageFilter()
{
}

FImageFilter::~FImageFilter()
{
    CleanUp();;
}

AkBool FImageFilter::Initialize(FRenderer* pRenderer, AkU32 uWidth, AkU32 uHeight, ID3D12RootSignature* pRootSignature, ID3D12PipelineState* pPSO)
{
    _pRenderer = pRenderer;
    _uWidth = uWidth;
    _uHeight = uHeight;
    _pRootSignature = pRootSignature;
    _pPSO = pPSO;

    _tViewPort.Width = (AkF32)_uWidth;
    _tViewPort.Height = (AkF32)_uHeight;
    _tViewPort.MinDepth = 0.0f;
    _tViewPort.MaxDepth = 1.0f;

    _tScissorRect.left = 0;
    _tScissorRect.top = 0;
    _tScissorRect.right = _uWidth;
    _tScissorRect.bottom = _uHeight;

    return AK_TRUE;
}

void FImageFilter::Draw(AkU32 uThreadIndex, ID3D12GraphicsCommandList* pCmdList, CD3DX12_CPU_DESCRIPTOR_HANDLE* pCPU, CD3DX12_GPU_DESCRIPTOR_HANDLE* pGPU)
{
    ID3D12Device* pDevice = _pRenderer->GetDevice();
    FConstantBufferPool* pPostProcessCBPool = _pRenderer->GetConstantBufferPool(uThreadIndex, CONSTANT_BUFFER_TYPE::CONSTANT_BUFFER_TYPE_POSTPROCESS);
    AkU32 uDecriptorSize = pDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

    pCmdList->RSSetViewports(1, &_tViewPort);
    pCmdList->RSSetScissorRects(1, &_tScissorRect);
    pCmdList->OMSetRenderTargets(1, &_pRtvCpuList[0], AK_FALSE, nullptr);

    CBContainer_t* pPostProcessCBContainer = pPostProcessCBPool->Alloc();
    if (!pPostProcessCBContainer)
    {
        __debugbreak();
        return;
    }
    
    PostProcessConstantBuffer_t* pPostProcessConstantBuffer = (PostProcessConstantBuffer_t*)pPostProcessCBContainer->pSystemMemAddr;
    memset(pPostProcessConstantBuffer, 0, sizeof(PostProcessConstantBuffer_t));
    pPostProcessConstantBuffer->fDx = 1.0f / (AkF32)_uWidth;
    pPostProcessConstantBuffer->fDy = 1.0f / (AkF32)_uHeight;
    pPostProcessConstantBuffer->fStrength = _pRenderer->GetBloomStrength();
    pPostProcessConstantBuffer->fExposure = 1.0f;
    pPostProcessConstantBuffer->fGamma = 2.2f;
    pPostProcessConstantBuffer->uOption0 = (AkU32)_pRenderer->GetToneMappingType(); // Default : Linear

    // b0
    CD3DX12_CPU_DESCRIPTOR_HANDLE hDest(*pCPU, 0, uDecriptorSize);
    pDevice->CopyDescriptorsSimple(1, hDest, pPostProcessCBContainer->hCPU, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
    hDest.Offset(1, uDecriptorSize);

    // t0 // t1
    AkU32 uSrvCount = 0;
    for(AkU32 i = 0; i < _countof(_pSrvCpuList); i++)
    {
        if(_pSrvCpuList[i].ptr)
        {
            pDevice->CopyDescriptorsSimple(1, hDest, _pSrvCpuList[i], D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
            hDest.Offset(1, uDecriptorSize);

            uSrvCount++;
        }
    }

    pCmdList->SetGraphicsRootSignature(_pRootSignature);
    pCmdList->SetPipelineState(_pPSO);

    pCmdList->SetGraphicsRootDescriptorTable(0, *pGPU);

    *pCPU = hDest;
    *pGPU = CD3DX12_GPU_DESCRIPTOR_HANDLE(*pGPU, 1 + uSrvCount, uDecriptorSize);
}

void FImageFilter::SetSrvCpu(D3D12_CPU_DESCRIPTOR_HANDLE* pSrvCpuList, AkU32 uNum)
{
    memcpy(_pSrvCpuList, pSrvCpuList, sizeof(D3D12_CPU_DESCRIPTOR_HANDLE) * uNum);
    _uSrvNum += uNum;
}

void FImageFilter::SetRtvCpu(D3D12_CPU_DESCRIPTOR_HANDLE* pRtvCpuList, AkU32 uNum)
{
    memcpy(_pRtvCpuList, pRtvCpuList, sizeof(D3D12_CPU_DESCRIPTOR_HANDLE) * uNum);
    _uRtvNum += uNum;
}

void FImageFilter::CleanUp()
{
}
