#include "SamplerState.hlsli"
#include "GlobalConsts.hlsli"

struct Sprite
{
    float2 texScale;
    int frameIndex;
    int frameIndexNext;
    float3 position;
    float life;
    float radius;
};

StructuredBuffer<Sprite> particles : register(t0);

struct GSInput
{
    float3 posModel : POSITION;
    int frameIndex : PSIZE0;
    int frameIndexNext : PSIZE1;
    float life : PSIZE2;
    float size : PSIZE3;
    float2 texScale : TEXCOORD;
};

GSInput VSMain(uint vertexID : SV_VertexID)
{
    const float fadeLife = 0.2f;
    
    Sprite p = particles[vertexID];
    
    GSInput output;
    
    output.posModel = p.position;
    output.frameIndex = p.frameIndex;
    output.frameIndexNext = p.frameIndexNext;
    output.life = p.life;
    output.size = p.radius;
    output.texScale = p.texScale;
    
    return output;
}

struct PSInput
{
    float4 posProj : SV_POSITION;
    float2 texCoord : TEXCOORD0;
    uint primID : SV_PrimitiveID;   
    int frameIndex : PSIZE0;
    int frameIndexNext : PSIZE1;
    float2 texScale : TEXCOORD1;
};

[maxvertexcount(4)]
void GSMain(point GSInput input[1], uint primID : SV_PrimitiveID, inout TriangleStream<PSInput> outputStream)
{
    PSInput output;
    
    if (input[0].life < 0.0f)
        return;
    
    float3 up = float3(0.0, 1.0, 0.0);
    float3 look = eyeWorld - input[0].posModel; 
    look = normalize(look);
    
    float3 right = normalize(cross(look, up));
    
    float halfWidth = input[0].size * 0.5f;
    float halfHeight = input[0].size * 0.5f;
    
    float4 v[4];
    v[0] = float4(input[0].posModel + halfWidth * right - halfHeight * up, 1.0f);
    v[1] = float4(input[0].posModel + halfWidth * right + halfHeight * up, 1.0f);
    v[2] = float4(input[0].posModel - halfWidth * right - halfHeight * up, 1.0f);
    v[3] = float4(input[0].posModel - halfWidth * right + halfHeight * up, 1.0f);
    
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
        output.posProj = mul(v[i], view);
        output.posProj = mul(output.posProj, proj);
        output.texCoord = texCoord[i];
        output.primID = primID;
        output.frameIndex = input[0].frameIndex;
        output.frameIndexNext = input[0].frameIndexNext;
        output.texScale = input[0].texScale;
        
        outputStream.Append(output);
    }
}

Texture2D spriteTex : register(t1);

float4 PSMain(PSInput input) : SV_TARGET
{
    float2 invTexScale = 1.0 / input.texScale;
    
    float2 curTexCoord = input.texCoord * invTexScale;
    //float2 nextTexCoord = input.texCoord * input.frameIndexNext * invTexScale;

    float3 color0 = spriteTex.Sample(linearWrapSS, curTexCoord).rgb;
    //float3 color1 = spriteTex.Sample(linearWrapSS, nextTexCoord);
    
    clip(color0 - 0.1);
    
    return float4(color0 * 2.0, 1.0);
}
