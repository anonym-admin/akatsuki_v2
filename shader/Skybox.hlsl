#include "GlobalConsts.hlsli"

TextureCube envTex : register(t10);
SamplerState pointWrapSS : register(s0);

struct VertexShaderInput
{
    float3 posModel : POSITION;
};

struct PixelShaderInput
{
    float4 posProj : SV_POSITION;
    float3 posModel : POSITION;
};

PixelShaderInput VSMain(VertexShaderInput input)
{
    PixelShaderInput output;
    
    output.posModel = input.posModel;
    output.posProj = mul(float4(input.posModel, 0.0), view);
    output.posProj = mul(float4(output.posProj.xyz, 1.0), proj);
    
    return output;
}

float4 PSMain(PixelShaderInput input) : SV_TARGET
{
    float3 color = 0.0;
    
    color = envTex.Sample(pointWrapSS, input.posModel.xyz);

    return float4(color, 1.0);
}
