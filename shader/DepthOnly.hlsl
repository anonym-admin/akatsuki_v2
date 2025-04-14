#include "GlobalConsts.hlsli"
#include "SamplerState.hlsli"

Texture2D heightTex : register(t0);

cbuffer MeshConsts : register(b1)
{
    matrix world;
    matrix worldIT;
    
    float heightScale;
    float3 clipMin;
    float reserve0;
    float3 clipMax;
};

cbuffer MaterialConsts : register(b2)
{
    float3 albedoFactor;
    float roughnessFactor;
    float3 emissionFactor;
    float metallicFactor;

    uint useAlbedoMap;
    uint useNormalMap;
    uint useEimissiveMap;
    uint useAOMap;
    uint invertNormalMapY;
    uint useMetallicMap;
    uint useRoughnessMap;
    uint useHeightMap;
};

#ifdef SKINNED
cbuffer SkinnedConsts : register(b3)
{
    matrix boneTransform[96];
}
#endif

struct DepthOnlyVSInput
{
    float3 posModel : POSITION;
    float3 normalModel : NORMAL;
    float2 texCoord : TEXCOORD;
    float3 tangentModel : TANGENT;
    
#ifdef SKINNED
    float4 blendWeight0 : BLENDWEIGHT0;
    float4 blendWieght1 : BLENDWEIGHT1;
    uint4 boneIndices0 : BLENDINDICES0;
    uint4 boneIndices1 : BLENDINDICES1;
#endif
};

struct DepthOnlyPSInput
{
    float4 posProj : SV_POSITION;
};

// Vertex Shader
DepthOnlyPSInput VSMain(DepthOnlyVSInput input)
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
    
    float3 posWorld = mul(float4(input.posModel, 1.0), world).xyz;
    if (useHeightMap)
    {
        float height = heightTex.SampleLevel(linearClampSS, input.texCoord, 0).r;
        height = height * 2.0 - 1.0;
        posWorld += posWorld * height * heightScale;
    }
    
    output.posProj = mul(float4(posWorld, 1.0), view);
    output.posProj = mul(output.posProj, proj);; // pojtected vertex = vertex x world x view x proj
    
    return output;
}

void PSMain(float4 pos : SV_POSITION)
{
    
}