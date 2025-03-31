#include "GlobalConsts.hlsli"

Texture2D renderTex : register(t0);
Texture2D depthOnlyTex : register(t1);
SamplerState linearClampSS : register(s0);

cbuffer PostEffectConst : register(b1)
{
    int mode;
    float depthScale;
    float fogStrength;
};

struct PostProcessVSInput
{
    float3 posModel : POSITION;
    float2 texCoord : TEXCOORD;
};

struct PostProcessPSInput
{
    float4 posProj : SV_POSITION;
    float2 texCoord : TEXCOORD;
};

PostProcessPSInput VSMain(PostProcessVSInput input)
{
    PostProcessPSInput output;
    
    output.posProj = float4(input.posModel, 1.0);
    output.texCoord = input.texCoord;
    
    return output;
}

float4 TexcoordToView(float2 texCoord)
{
    float4 posProj;

    // [0, 1]x[0, 1] -> [-1, 1]x[-1, 1]
    posProj.xy = texCoord * 2.0 - 1.0;
    posProj.y *= -1; // y 좌표 뒤집기
    posProj.z = depthOnlyTex.Sample(linearClampSS, texCoord).r;
    posProj.w = 1.0;
    
    // ProjectSpace -> ViewSpace
    //float4 posView = mul(posProj, lights[0].invProj);
    float4 posView = mul(posProj, invProj);
    posView.xyz /= posView.w;
    
    return posView;
}

float4 PSMain(PostProcessPSInput input) : SV_TARGET
{
    [flatten]
    if(mode == 1)
    {
        float3 color = clamp(renderTex.Sample(linearClampSS, input.texCoord).rgb, 0.0, 1.0);
        
        float4 posView = TexcoordToView(input.texCoord);
        
        // Fog
        float dist = length(posView.xyz); // 눈의 위치가 원점인 좌표계
        float3 fogColor = float3(1, 1, 1);
        float fogMin = 1.0;
        float fogMax = 10.0;
        float distFog = saturate((dist - fogMin) / (fogMax - fogMin));
        float fogFactor = exp(-distFog * fogStrength);
        
        color = lerp(fogColor, color, fogFactor);
        
        return float4(color, 1.0);
    }
    else if(mode == 2)
    {
        float z = TexcoordToView(input.texCoord).z * depthScale;
        return float4(z, z, z, 1);
    }
    else
    {
        return float4(0, 0, 0, 1);
    }
}