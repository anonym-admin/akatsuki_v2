#include "Common.hlsli"
#include "DiskSample.hlsli"

// Vertex Shader
PSInput VSMain(VSInput input)
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
    
    PSInput output;
    
    matrix vpMat = mul(view, proj); // view x proj
    matrix wvpMat = mul(world, vpMat); // world x view x proj
    
    output.posModel = input.posModel;
    
    output.posWorld = mul(float4(input.posModel, 1.0), world);
    
    output.posProj = mul(float4(input.posModel, 1.0), wvpMat); // pojtected vertex = vertex x world x view x proj
    output.normalWorld = mul(float4(input.normalModel, 0.0), worldIT).xyz;
    output.texCoord = input.texCoord;
    output.tangentWorld = mul(float4(input.tangentModel, 0.0), world).xyz;
    
    return output;
}

// Pixel Shader
Texture2D albedoTex : register(t0);
Texture2D normalTex : register(t1);
Texture2D emissiveTex : register(t2);
Texture2D metallicTex : register(t3);
Texture2D roughnessTex : register(t4);
Texture2D aoTex : register(t5);

static const float3 Fdielectric = 0.04; // 비금속(Dielectric) 재질의 F0

float3 GetNormal(PSInput input)
{
    float3 normalWorld = normalize(input.normalWorld);
    
    if (useNormalMap) // NormalWorld를 교체
    {
        float3 normal = normalTex.Sample(linearWrapSS, input.texCoord).rgb;
        normal = 2.0 * normal - 1.0; // 범위 조절 [-1.0, 1.0]

        float3 N = normalWorld;
        float3 T = normalize(input.tangentWorld - dot(input.tangentWorld, N) * N);
        float3 B = cross(N, T);
        
        // matrix는 float4x4, 여기서는 벡터 변환용이라서 3x3 사용
        float3x3 TBN = float3x3(T, B, N);
        normalWorld = normalize(mul(normal, TBN));
    }
    
    return normalWorld;
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
    float att = 1.0; //saturate((light.fallOffEnd - lightDist) / (light.fallOffEnd - light.fallOffStart));
    
    // Shadow map
    float shadowFactor = 1.0;
    
    if (light.type & LIGHT_SHADOW)
    {        
        shadowFactor = CalcShadowFactor(light, posWorld, normalWorld);
    }
    
    float3 radiance = light.radiance * spotFator * att * shadowFactor;
    
    return radiance;
}

float4 PSMain(PSInput input) : SV_TARGET
{
    float3 pixelToEye = normalize(eyeWorld - input.posWorld);
    float3 normalWorld = GetNormal(input);
    
    float4 albedo = useAlbedoMap ? albedoTex.Sample(linearWrapSS, input.texCoord) * float4(albedoFactor, 1.0) : float4(albedoFactor, 1.0);
    float ao = useAOMap ? aoTex.Sample(linearWrapSS, input.texCoord).r : 1.0;
    float metallic = useMetallicMap ? metallicTex.Sample(linearWrapSS, input.texCoord).r : metallicFactor;
    float roughness = useRoughnessMap ? roughnessTex.Sample(linearWrapSS, input.texCoord).r : roughnessFactor;
    float3 emission = useEimissiveMap ? emissiveTex.Sample(linearWrapSS, input.texCoord).rgb : emissionFactor;
    
    float3 ambientLighting = AmbientLightingByIBL(albedo.rgb, normalWorld, pixelToEye, ao, metallic, roughness) * strengthIBL;
    float3 directLighting = float3(0, 0, 0);
    
    [unroll]
    for (int i = 0; i < MAX_LIGHTS_COUNT; i++)
    {
        if (lights[i].type)
        {
            float3 L = lights[i].position - input.posWorld;
            float3 r = normalize(reflect(eyeWorld - input.posWorld, normalWorld));
            float3 centerToRay = dot(L, r) * r - L;
            float3 representativePoint = L + centerToRay * clamp(lights[i].radius / length(centerToRay), 0.0, 1.0);
            representativePoint += input.posWorld;
            float3 lightVec = representativePoint - input.posWorld;
            if (lights[i].type & LIGHT_DIRECTIONAL)
            {
                lightVec = lights[i].direction;
            }
            
            float lightDist = length(lightVec);
            lightVec /= lightDist;
            float3 halfway = normalize(pixelToEye + lightVec);
            
            float NdotI = max(0.0, dot(normalWorld, lightVec));
            float NdotH = max(0.0, dot(normalWorld, halfway));
            float NdotO = max(0.0, dot(normalWorld, pixelToEye));
            
            const float3 Fdielectric = 0.04; // 비금속(Dielectric) 재질의 F0
            float3 F0 = lerp(Fdielectric, albedo.rgb, metallic);
            float3 F = SchlickFresnel(F0, max(0.0, dot(halfway, pixelToEye)));
            float3 kd = lerp(float3(1, 1, 1) - F, float3(0, 0, 0), metallic);
            float3 diffuseBRDF = kd * albedo.rgb;
            
            // Sphere Normalization
            float alpha = roughness * roughness;
            float alphaPrime = saturate(alpha + lights[i].radius / (2.0 * lightDist));
            
            float D = NdfGGX(NdotH, roughness, alphaPrime);
            float3 G = SchlickGGX(NdotI, NdotO, roughness);
            float3 specularBRDF = (F * D * G) / max(1e-5, 4.0 * NdotI * NdotO);
            
            float3 radiance = LightRadiance(lights[i], representativePoint, input.posWorld, normalWorld);
            
            if (abs(dot(float3(1, 1, 1), radiance)) > 1e-5)
            {
                directLighting += (diffuseBRDF + specularBRDF) * radiance * NdotI;
            }
        }
    }
    
    float4 pixelColor = float4(ambientLighting + directLighting + emission, 1.0);
    pixelColor = clamp(pixelColor, 0.0, 1000.0);
    
    return pixelColor;
}
