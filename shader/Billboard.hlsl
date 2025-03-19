#include "Common.hlsli"

Texture2DArray arrayTex : register(t20);
SamplerState samp : register(s0);

struct BillboardVSInput
{
    float4 posModel : POSITION;
    float2 size : SIZE0;
    float2 padding : SIZE1;
};

struct BillboardGSInput
{
    float3 center : POSITION;
    float2 size : SIZE;
};

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

BillboardGSInput VSMain(BillboardVSInput input)
{
    BillboardGSInput output;
    
    output.center = input.posModel;
    output.size = input.size;
    
    return output;
}

// Billboard 생성 시 Clear 색과 곂쳐서 랜더링되는 버그 발생
[maxvertexcount(4)]
void GSMain(point BillboardGSInput input[1], uint primID : SV_PrimitiveID, inout TriangleStream<BillboardPSInput> outputStream)
{
    BillboardPSInput output;
    
    float3 up = float3(0.0, 1.0, 0.0);
    float3 look = eyeWorld - input[0].center; // eyeWorld 의 문제는 아님...
    look.y = 0.0;
    look = normalize(look);
    
    float3 right = normalize(cross(look, up));
    
    float halfWidth = input[0].size.x * 0.5f;
    float halfHeight = input[0].size.y * 0.5f;
    
    float4 v[4];
    v[0] = float4(input[0].center + halfWidth * right - halfHeight * up, 1.0f);
    v[1] = float4(input[0].center + halfWidth * right + halfHeight * up, 1.0f);
    v[2] = float4(input[0].center - halfWidth * right - halfHeight * up, 1.0f);
    v[3] = float4(input[0].center - halfWidth * right + halfHeight * up, 1.0f);
    
    float2 texCoord[4] =
    {
        float2(0.0, 1.0),
        float2(0.0, 0.0),
        float2(1.0, 1.0),
        float2(1.0, 0.0)
    };
    
    [unroll]
    for (int i = 0; i < 4; i++)
    {
        output.posWorld = mul(v[i], world).xyz;
        output.posProj = mul(float4(output.posWorld, 1.0), view);
        output.posProj = mul(output.posProj, proj);
        output.posModel = v[i].xyz;
        output.normalWorld = look;
        output.tangentWorld = float3(1.0, 0.0, 0.0);
        output.texCoord = texCoord[i];
        output.primID = primID;

        outputStream.Append(output);
    }
}

float4 PSMain(BillboardPSInput input) : SV_TARGET
{
    float3 pixelToEye = normalize(eyeWorld - input.posWorld);
    float3 normalWorld = GetNormal(input.normalWorld, input.texCoord, input.tangentWorld);
    
    float3 uvw = float3(input.texCoord, input.primID % 4);
    float4 albedo = arrayTex.Sample(samp, uvw);
    
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
