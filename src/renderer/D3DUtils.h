#pragma once

class FD3DUtils
{
public:
	static void SetDefaultSamplerDesc(D3D12_STATIC_SAMPLER_DESC* pOutSamperDesc, AkU32 uRegisterIndex);
	static void GetStaticSamplers(CD3DX12_STATIC_SAMPLER_DESC* pOutSamplerDesc);
	static AkU32 AlignConstantBufferSize(AkU32 uSize);
	static void UpdateTexture(ID3D12Device* pDevice, ID3D12GraphicsCommandList* pCmdList, ID3D12Resource* pDestTexResource, ID3D12Resource* pSrcTexResource);
};

