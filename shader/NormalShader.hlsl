#include "Common.hlsli"

struct NormalGSInput
{
    float4 posModel : SV_POSITION;
    float3 normalModel : NORMAL;
};

struct NormalPSInput
{
    float4 posProj : SV_POSITION;
    float3 color : COLOR;
};

static const float lineScale = 0.02;

NormalGSInput VSMain(VSInput input)
{
#ifdef SKINNED
    
    float weights[8];
    weights[0] = input.blendWeight0.x;
    weights[1] = input.blendWeight0.y;
    weights[2] = input.blendWeight0.z;
    weights[3] = input.blendWeight0.w;
    weights[4] = input.blendWieght1.x;
    weights[5] = input.blendWieght1.y;
    weights[6] = input.blendWieght1.z;
    weights[7] = input.blendWieght1.w;
    
    uint indices[8];
    indices[0] = input.boneIndices0.x;
    indices[1] = input.boneIndices0.y;
    indices[2] = input.boneIndices0.z;
    indices[3] = input.boneIndices0.w;
    indices[4] = input.boneIndices1.x;
    indices[5] = input.boneIndices1.y;
    indices[6] = input.boneIndices1.z;
    indices[7] = input.boneIndices1.w;
    
    float3 posModel = float3(0.0, 0.0, 0.0);
    float3 normalModel = float3(0.0, 0.0, 0.0);
    
    for (int i = 0; i < 8; i++)
    {
        posModel += weights[i] * mul(float4(input.posModel, 1.0), boneTransform[indices[i]]).xyz;
        normalModel += weights[i] * mul(float4(input.normalModel, 0.0), boneTransform[indices[i]]).xyz;
    }
    
    input.posModel = posModel;
    input.normalModel = normalModel;
    
#endif
    
    NormalGSInput output;
    
    output.posModel = float4(input.posModel, 1.0);
    output.normalModel = input.normalModel;
    
    return output;
}

[maxvertexcount(2)]
void GSMain(point NormalGSInput input[1], inout LineStream<NormalPSInput> outputStream)
{
    NormalPSInput output;
    
    float4 posWorld = mul(input[0].posModel, world);
    float4 normalModel = float4(input[0].normalModel, 0.0);
    float4 normalWorld = mul(normalModel, worldIT);
    normalWorld = float4(normalize(normalWorld.xyz), 0.0);
    
    output.posProj = mul(posWorld, view);
    output.posProj = mul(output.posProj, proj);
    output.color = float3(1.0, 1.0, 0.0);
    outputStream.Append(output);
    
    output.posProj = mul(posWorld + lineScale * normalWorld, view);
    output.posProj = mul(output.posProj, proj);
    output.color = float3(1.0, 0.0, 0.0);
    outputStream.Append(output);
}

float4 PSMain(NormalPSInput input) : SV_TARGET
{
    return float4(input.color, 1.0f);
}
