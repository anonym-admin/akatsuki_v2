#include "Common.hlsli"

struct DepthOnlyPSInput
{
    float4 posProj : SV_POSITION;
};

// Vertex Shader
DepthOnlyPSInput VSMain(VSInput input)
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
    float3 tangentModel = float3(0.0, 0.0, 0.0);
    
    for (int i = 0; i < 8; i++)
    {
        posModel += weights[i] * mul(float4(input.posModel, 1.0), boneTransform[indices[i]]).xyz;
        normalModel += weights[i] * mul(float4(input.normalModel, 0.0), boneTransform[indices[i]]).xyz;
        tangentModel += weights[i] * mul(float4(input.tangentModel, 0.0), boneTransform[indices[i]]).xyz;
    }
    
    input.posModel = posModel;
    input.normalModel = normalModel;
    input.tangentModel = tangentModel;
    
#endif
    
    DepthOnlyPSInput output;
    
    matrix vpMat = mul(view, proj); // view x proj
    matrix wvpMat = mul(world, vpMat); // world x view x proj
    
    output.posProj = mul(float4(input.posModel, 1.0), wvpMat); // pojtected vertex = vertex x world x view x proj
    
    return output;
}

void PSMain(float4 pos : SV_POSITION)
{
    
}