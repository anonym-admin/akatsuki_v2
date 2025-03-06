#ifndef __COMMON_HLSLI__
#define __COMMON_HLSLI__

#include "IBLTexture.hlsli"
#include "SamplerState.hlsli"
#include "GlobalConsts.hlsli"

#define LIGHT_OFF 0x00
#define LIGHT_DIRECTIONAL 0x01
#define LIGHT_POINT 0x02
#define LIGHT_SPOT 0x04
#define LIGHT_SHADOW 0x10

Texture2D shadowMap[5] : register(t15);

cbuffer MeshConsts : register(b1)
{
    matrix world;
    matrix worldIT;
};

#ifdef SKINNED
cbuffer SkinnedConsts : register(b3)
{
    matrix boneTransform[96];
}
#endif

struct VSInput
{
    float3 posModel : POSITION;
    float3 normalModel : NORMAL;
    float3 texCoord : TEXCOORD;
    float3 tangentModel : TANGENT;
    
#ifdef SKINNED
    float4 blendWeight0 : BLENDWEIGHT0;
    float4 blendWieght1 : BLENDWEIGHT1;
    uint4 boneIndices0 : BLENDINDICES0;
    uint4 boneIndices1 : BLENDINDICES1;
#endif
};

struct PSInput
{
    float4 posProj : SV_POSITION;
    float3 posWorld : POSITION0;
    float3 normalWorld : NORMAL;
    float2 texCoord : TEXCOORD;
    float3 tangentWorld : TANGENT;
    float3 posModel : POSITION1;
};

#endif // __COMMON_HLSLI__