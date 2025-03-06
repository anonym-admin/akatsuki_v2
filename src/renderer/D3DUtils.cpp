#include "pch.h"
#include "D3DUtils.h"

void FD3DUtils::SetDefaultSamplerDesc(D3D12_STATIC_SAMPLER_DESC* pOutSamperDesc, AkU32 uRegisterIndex)
{
	D3D12_STATIC_SAMPLER_DESC sampler = {};
	//pOutSamperDesc->Filter = D3D12_FILTER_ANISOTROPIC;
	pOutSamperDesc->Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;

	pOutSamperDesc->AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	pOutSamperDesc->AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	pOutSamperDesc->AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	pOutSamperDesc->MipLODBias = 0.0f;
	pOutSamperDesc->MaxAnisotropy = 16;
	pOutSamperDesc->ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
	pOutSamperDesc->BorderColor = D3D12_STATIC_BORDER_COLOR_OPAQUE_WHITE;
	pOutSamperDesc->MinLOD = -FLT_MAX;
	pOutSamperDesc->MaxLOD = D3D12_FLOAT32_MAX;
	pOutSamperDesc->ShaderRegister = uRegisterIndex;
	pOutSamperDesc->RegisterSpace = 0;
	pOutSamperDesc->ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
}

void FD3DUtils::GetStaticSamplers(CD3DX12_STATIC_SAMPLER_DESC* pOutSamplerDesc)
{
	const CD3DX12_STATIC_SAMPLER_DESC tPointWrap(
		0, // shaderRegister
		D3D12_FILTER_MIN_MAG_MIP_POINT, // filter
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressU
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressV
		D3D12_TEXTURE_ADDRESS_MODE_WRAP); // addressW

	const CD3DX12_STATIC_SAMPLER_DESC tPointClamp(
		1, // shaderRegister
		D3D12_FILTER_MIN_MAG_MIP_POINT, // filter
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressU
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressV
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP); // addressW

	const CD3DX12_STATIC_SAMPLER_DESC tLinearWrap(
		2, // shaderRegister
		D3D12_FILTER_MIN_MAG_MIP_LINEAR, // filter
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressU
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressV
		D3D12_TEXTURE_ADDRESS_MODE_WRAP); // addressW

	const CD3DX12_STATIC_SAMPLER_DESC tLinearClamp(
		3, // shaderRegister
		D3D12_FILTER_MIN_MAG_MIP_LINEAR, // filter
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressU
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressV
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP); // addressW

	const CD3DX12_STATIC_SAMPLER_DESC tAnisotropicWrap(
		4, // shaderRegister
		D3D12_FILTER_ANISOTROPIC, // filter
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressU
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressV
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressW
		0.0f,                             // mipLODBias
		8);                               // maxAnisotropy

	const CD3DX12_STATIC_SAMPLER_DESC tAnisotropicClamp(
		5, // shaderRegister
		D3D12_FILTER_ANISOTROPIC, // filter
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressU
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressV
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressW
		0.0f,                              // mipLODBias
		8);                                // maxAnisotropy

	const CD3DX12_STATIC_SAMPLER_DESC tShadow(
		6, // shaderRegister
		D3D12_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT, // filter
		D3D12_TEXTURE_ADDRESS_MODE_BORDER,  // addressU
		D3D12_TEXTURE_ADDRESS_MODE_BORDER,  // addressV
		D3D12_TEXTURE_ADDRESS_MODE_BORDER,  // addressW
		0.0f,                               // mipLODBias
		16,                                 // maxAnisotropy
		D3D12_COMPARISON_FUNC_LESS_EQUAL,
		D3D12_STATIC_BORDER_COLOR_OPAQUE_WHITE);

	pOutSamplerDesc[0] = tPointWrap;
	pOutSamplerDesc[1] = tPointClamp;
	pOutSamplerDesc[2] = tLinearWrap;
	pOutSamplerDesc[3] = tLinearClamp;
	pOutSamplerDesc[4] = tAnisotropicWrap;
	pOutSamplerDesc[5] = tAnisotropicClamp;
	pOutSamplerDesc[6] = tShadow;
}

AkU32 FD3DUtils::AlignConstantBufferSize(AkU32 uSize)
{
	AkU32 uAlignSize = (uSize + 255) & (~255); 
	return uAlignSize;
}

void FD3DUtils::UpdateTexture(ID3D12Device* pDevice, ID3D12GraphicsCommandList* pCmdList, ID3D12Resource* pDestTexResource, ID3D12Resource* pSrcTexResource)
{
	const AkU32 MAX_SUB_RESOURCE_NUM = 32;
	D3D12_PLACED_SUBRESOURCE_FOOTPRINT tFootprint[MAX_SUB_RESOURCE_NUM] = {};
	AkU32 uRows[MAX_SUB_RESOURCE_NUM] = {};
	AkU64 u64RowSize[MAX_SUB_RESOURCE_NUM] = {};
	AkU64 u64TotalBytes = 0;

	D3D12_RESOURCE_DESC tDesc = pDestTexResource->GetDesc();
	if (tDesc.MipLevels > (AkU32)_countof(tFootprint))
		__debugbreak();

	pDevice->GetCopyableFootprints(&tDesc, 0, tDesc.MipLevels, 0, tFootprint, uRows, u64RowSize, &u64TotalBytes);

	pCmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(pDestTexResource, D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_COPY_DEST));
	for (AkU32 i = 0; i < tDesc.MipLevels; i++)
	{

		D3D12_TEXTURE_COPY_LOCATION	tDestLocation = {};
		tDestLocation.PlacedFootprint = tFootprint[i];
		tDestLocation.pResource = pDestTexResource;
		tDestLocation.SubresourceIndex = i;
		tDestLocation.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;

		D3D12_TEXTURE_COPY_LOCATION	tSrcLocation = {};
		tSrcLocation.PlacedFootprint = tFootprint[i];
		tSrcLocation.pResource = pSrcTexResource;
		tSrcLocation.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;

		pCmdList->CopyTextureRegion(&tDestLocation, 0, 0, 0, &tSrcLocation, nullptr);
	}
	pCmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(pDestTexResource, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE));

}
