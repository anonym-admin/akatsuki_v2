#include "Common.hlsli"

struct VertexShaderInput
{
    float3 posModel : POSITION;
    float3 color : COLOR;
};

struct PixelShaderInput
{
    float4 posProj : SV_POSITION;
    float3 color : COLOR;
};

PixelShaderInput VSMain(VertexShaderInput input)
{
    PixelShaderInput output;
    
    output.posProj = mul(float4(input.posModel, 1.0), world);
    output.posProj = mul(output.posProj, view);
    output.posProj = mul(output.posProj, proj);
    
    output.color = input.color;
    
    return output;
}

float4 PSMain(PixelShaderInput input) : SV_TARGET
{
    return float4(input.color, 1.0);
}