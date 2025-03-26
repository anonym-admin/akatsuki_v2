#include "Common.hlsli"

Texture2DArray arrayTex : register(t20);
SamplerState samp : register(s0);

struct BillboardPSInput
{
    float4 posProj : SV_POSITION;
    float3 posWorld : POSITION0;
    float3 posModel : POSITION1;
    float3 normalWorld : NORMAL;
    float3 tangentWorld : TANGENT;
    float2 texCoord : TEXCOORD;
    uint primID : SV_PrimitiveID;
};

void PSMain(BillboardPSInput input)
{
    float3 uvw = float3(input.texCoord, input.primID % 4);
    float4 albedo = arrayTex.Sample(samp, uvw);
    
    clip(albedo.a - 0.1);
}