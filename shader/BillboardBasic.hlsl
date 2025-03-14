#include "Common.hlsli"
#include "DiskSample.hlsli"

// Vertex Shader
PSInput VSMain(VSInput input)
{
    PSInput output;
    
    matrix vpMat = mul(view, proj); // view x proj
    matrix wvpMat = mul(world, vpMat); // world x view x proj
    
    output.posModel = input.posModel;
    output.posWorld = mul(float4(input.posModel, 1.0), world);
    output.posProj = mul(float4(output.posWorld, 1.0), vpMat);
    output.normalWorld = mul(float4(input.normalModel, 0.0), worldIT);
    output.texCoord = input.texCoord;
    output.tangentWorld = mul(float4(input.tangentModel, 1.0), world);
    
    return output;
}

float4 PSMain(PSInput input) : SV_TARGET
{
    float3 pixelToEye = normalize(eyeWorld - input.posWorld);
    float3 normalWorld = GetNormal(input.normalWorld, input.texCoord, input.tangentWorld);
    
    float4 albedo = useAlbedoMap ? albedoTex.Sample(linearWrapSS, input.texCoord) * float4(albedoFactor, 1.0) : float4(albedoFactor, 1.0);
    
    clip(albedo.a - 0.1);
    
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
