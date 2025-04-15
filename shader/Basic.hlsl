#include "Common.hlsli"

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
   
    output.posModel = input.posModel;
    
    output.posWorld = mul(float4(input.posModel, 1.0), world);
    output.normalWorld = mul(float4(input.normalModel, 0.0), worldIT).xyz;
    
    if (useHeightMap)
    {
        float height = heightTex.SampleLevel(linearClampSS, input.texCoord, 0).r;
        height = height * 2.0 - 1.0;
        output.posWorld += output.normalWorld * height * heightScale;
    }
    
    output.posProj = mul(float4(output.posWorld, 1.0), view); 
    output.posProj = mul(output.posProj, proj);
    output.texCoord = input.texCoord;
    output.tangentWorld = mul(float4(input.tangentModel, 0.0), world).xyz;
    
    return output;
}

// Pixel Shader
float4 PSMain(PSInput input) : SV_TARGET
{
    if (clipMin.x < input.posWorld.x && input.posWorld.x < clipMax.x && 
        clipMin.y < input.posWorld.y && input.posWorld.y < clipMax.y &&
        clipMin.z < input.posWorld.z && input.posWorld.z < clipMax.z )
    {
        discard;
    }
    
    float3 pixelToEye = normalize(eyeWorld - input.posWorld);
    float3 normalWorld = GetNormal(input.normalWorld, input.texCoord, input.tangentWorld);
    
    float4 albedo = useAlbedoMap ? albedoTex.Sample(linearWrapSS, input.texCoord) * float4(albedoFactor, 1.0) : float4(albedoFactor, 1.0);
    
    // tree의 투명도 clip
    clip(albedo.a - 0.5);
    
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
