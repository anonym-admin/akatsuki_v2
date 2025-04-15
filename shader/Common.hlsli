#ifndef __COMMON_HLSLI__
#define __COMMON_HLSLI__

#include "IBLTexture.hlsli"
#include "SamplerState.hlsli"
#include "GlobalConsts.hlsli"

Texture2D albedoTex : register(t0);
Texture2D normalTex : register(t1);
Texture2D emissiveTex : register(t2);
Texture2D metallicTex : register(t3);
Texture2D roughnessTex : register(t4);
Texture2D aoTex : register(t5);
Texture2D heightTex : register(t6);
Texture2D shadowMap[5] : register(t15);

cbuffer MeshConsts : register(b1)
{
    matrix world;
    matrix worldIT;
    matrix worldInv;
    
    float heightScale;
    float3 clipMin;
    float reserve0;
    float3 clipMax;
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
    float2 texCoord : TEXCOORD;
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

/*
===========
PBR
===========
*/

static const float3 Fdielectric = 0.04; // 비금속(Dielectric) 재질의 F0

float3 GetNormal(float3 normalWorld, float2 texCoord, float3 tangentWorld)
{
    float3 normalWorldAfter = normalize(normalWorld);
    
    if (useNormalMap) // NormalWorld를 교체
    {
        float3 normal = normalTex.Sample(linearWrapSS, texCoord).rgb;
        normal = 2.0 * normal - 1.0; // 범위 조절 [-1.0, 1.0]

        float3 N = normalWorldAfter;
        float3 T = normalize(tangentWorld - dot(tangentWorld, N) * N);
        float3 B = cross(N, T);
        
        // matrix는 float4x4, 여기서는 벡터 변환용이라서 3x3 사용
        float3x3 TBN = float3x3(T, B, N);
        normalWorldAfter = normalize(mul(normal, TBN));
    }
    
    return normalWorldAfter;
}

float3 SchlickFresnel(float3 F0, float NdotH)
{
    return F0 + (1.0 - F0) * pow(2.0, (-5.55473 * NdotH - 6.98316) * NdotH);
}

float3 DiffuseIBL(float3 albedo, float3 normalWorld, float3 pixelToEye, float metallic)
{
    float3 F0 = lerp(Fdielectric, albedo, metallic);
    float3 F = SchlickFresnel(F0, max(0.0, dot(normalWorld, pixelToEye)));
    float3 kd = lerp(1.0 - F, 0.0, metallic);
    float3 irradiance = irradianceIBLTex.SampleLevel(linearWrapSS, normalWorld, 0).rgb;
    
    return kd * albedo * irradiance;
}

float3 SpecularIBL(float3 albedo, float3 normalWorld, float3 pixelToEye, float metallic, float roughness)
{
    float2 specularBRDF = brdfTex.SampleLevel(linearClampSS, float2(dot(normalWorld, pixelToEye), 1.0 - roughness), 0.0f).rg;
    float3 specularIrradiance = specularIBLTex.SampleLevel(linearWrapSS, reflect(-pixelToEye, normalWorld),
                                                            2 + roughness * 5.0f).rgb;
    const float3 Fdielectric = 0.04; // 비금속(Dielectric) 재질의 F0
    float3 F0 = lerp(Fdielectric, albedo, metallic);

    return (F0 * specularBRDF.x + specularBRDF.y) * specularIrradiance;
}

float3 AmbientLightingByIBL(float3 albedo, float3 normalW, float3 pixelToEye, float ao, float metallic, float roughness)
{
    float3 diffuseIBL = DiffuseIBL(albedo, normalW, pixelToEye, metallic);
    float3 specularIBL = SpecularIBL(albedo, normalW, pixelToEye, metallic, roughness);
    
    return (diffuseIBL + specularIBL) * ao;
}

// GGX/Towbridge-Reitz normal distribution function.
// Uses Disney's reparametrization of alpha = roughness^2.
float NdfGGX(float NdotH, float roughness, float alphaPrime)
{
    float alpha = roughness * roughness;
    float alphaSq = alpha * alpha;
    float denom = (NdotH * NdotH) * (alphaSq - 1.0) + 1.0;
    return alphaPrime * alphaPrime / (3.141592 * denom * denom);
}

// Single term for separable Schlick-GGX below.
float SchlickG1(float NdotV, float k)
{
    return NdotV / (NdotV * (1.0 - k) + k);
}

// Schlick-GGX approximation of geometric attenuation function using Smith's method.
float SchlickGGX(float NdotI, float NdotO, float roughness)
{
    float r = roughness + 1.0;
    float k = (r * r) / 8.0;
    return SchlickG1(NdotI, k) * SchlickG1(NdotO, k);
}

float PCF(Texture2D shadowMap, float2 texCoord, float depth)
{
    uint width, height, numMips;
    shadowMap.GetDimensions(0, width, height, numMips);
    
    // Texel size.
    float dx = 1.0f / (float) width;
    
    float percentLit = 0.0f;
    const float2 offsets[9] =
    {
        float2(-dx, -dx), float2(0.0f, -dx), float2(dx, -dx),
        float2(-dx, 0.0f), float2(0.0f, 0.0f), float2(dx, 0.0f),
        float2(-dx, +dx), float2(0.0f, +dx), float2(dx, +dx)
    };
    
    // sampling 16 이상 사용해도 성능 및 품질 차이 없음. ??
    [unroll]
    for (int i = 0; i < 9; ++i)
    {
        percentLit += shadowMap.SampleCmpLevelZero(shadowSS, texCoord + offsets[i], depth).r;
    }
    
    return percentLit / 9;
}

float CalcShadowFactor(LightConsts light, float3 posWorld, float3 normalWorld)
{
    const float tuneShadow = 0.5;
    float shadowFactor = 1.0;
    float3 posView = mul(float4(posWorld, 1.0), view);
    
    float depthView = posView.z;
    float cascadePlandeDistance[5] = { 1000.0 / 50.0, 1000.0 / 25.0, 1000.0 / 10.0, 1000.0 / 2.0, 1000.0 }; // Hard coding.
    int layer = -1;
    for (int i = 0; i < 5; i++)
    {
        if (depthView < cascadePlandeDistance[i])
        {
            layer = i;
            break;
        }
    }
    
    float3 lightVec = -normalize(light.direction);
    float ndotl = dot(normalWorld, lightVec);
    if (ndotl >= 0.0)
    {
        return shadowFactor;
    }
    
    float4 lightScreen = mul(float4(posWorld, 1.0), light.viewProj[layer]);
    lightScreen.xyz /= lightScreen.w;
        
    float2 lightTexCoord = float2(lightScreen.x, -lightScreen.y);
    lightTexCoord += 1.0; // [-1, 1] => [0, 2]
    lightTexCoord *= 0.5; // [0, 2] => [0, 1]
    
    switch (layer)
    {
        case 0:
            shadowFactor = PCF(shadowMap[0], lightTexCoord.xy, lightScreen.z - 0.001);
            break;
        case 1:
            shadowFactor = PCF(shadowMap[1], lightTexCoord.xy, lightScreen.z - 0.001);
            break;
        case 2:
            shadowFactor = PCF(shadowMap[2], lightTexCoord.xy, lightScreen.z - 0.001);
            break;
        case 3:
            shadowFactor = PCF(shadowMap[3], lightTexCoord.xy, lightScreen.z - 0.001);
            break;
        case 4:
            shadowFactor = PCF(shadowMap[4], lightTexCoord.xy, lightScreen.z - 0.001);
            break;
    }
    
    return shadowFactor;
}

float3 LightRadiance(LightConsts light, float3 representativePoint, float3 posWorld, float3 normalWorld)
{
    // Directional light
    float3 lightVec = light.type & LIGHT_DIRECTIONAL ? -light.direction
                                                     : representativePoint - posWorld; //: light.position - posWorld;

    float lightDist = length(lightVec);
    lightVec /= lightDist;

    // Spot light
    float spotFator = light.type & LIGHT_SPOT ? pow(max(-dot(lightVec, light.direction), 0.0f), light.spotPower)
                                              : 1.0f;
    // Distance attenuation
    float att = saturate((light.fallOffEnd - lightDist) / (light.fallOffEnd - light.fallOffStart));
    
    // Shadow map
    float shadowFactor = 1.0;
    
    if (light.type & LIGHT_SHADOW)
    {
        shadowFactor = CalcShadowFactor(light, posWorld, normalWorld);
    }
    
    float3 radiance = light.radiance * spotFator * att * shadowFactor;
    
    return radiance;
}

#endif