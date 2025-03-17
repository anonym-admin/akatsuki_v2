#include "SamplerState.hlsli"
#include "GlobalConsts.hlsli"

struct Particle
{
    float3 position;
    float3 velocity;
    float3 color;
    float life;
    float size;
};

StructuredBuffer<Particle> particles : register(t0);

struct GSInput
{
    float3 posModel : POSITION;
    float3 color : COLOR;
    float life : PSIZE0;
    float size : PSIZE1;
};

GSInput VSMain(uint vertexID : SV_VertexID)
{
    const float fadeLife = 0.2f;
    
    Particle p = particles[vertexID];
    
    GSInput output;
    
    output.posModel = p.position;
    output.color = p.color * saturate(p.life / fadeLife);
    output.life = p.life;
    output.size = p.size;
    
    return output;
}

struct PSInput
{
    float4 posProj : SV_POSITION;
    float2 texCoord : TEXCOORD;
    float3 color : COLOR;
    uint primID : SV_PrimitiveID;
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
        output.color = input[0].color;
        output.primID = primID;

        outputStream.Append(output);
    }
}

Texture2D spriteTex : register(t1);

// https://en.wikipedia.org/wiki/Smoothstep
float smootherstep(float x, float edge0 = 0.0f, float edge1 = 1.0f)
{
  // Scale, and clamp x to 0..1 range
    x = clamp((x - edge0) / (edge1 - edge0), 0, 1);

    return x * x * x * (3 * x * (2 * x - 5) + 10.0f);
}

// 일반적으로 Sprite는 텍스춰를 많이 사용합니다.
// 이 예제처럼 수식으로 패턴을 만들 수도 있습니다.
float4 PSMain(PSInput input) : SV_TARGET
{
    float2 uv = input.texCoord;
    if (input.primID % 4 == 0 || input.primID % 4 == 2)
    {
        uv.x -= 0.5;
        uv.x = -uv.x;
        uv.x += 0.5;
    }
    if (input.primID % 4 == 1 || input.primID % 4 == 2)
    {
        uv.y -= 0.5;
        uv.y = -uv.y;
        uv.y += 0.5;
    }
    
    float4 sprite = spriteTex.Sample(linearWrapSS, uv);

    return float4(input.color.rgb * sprite.rgb * sprite.a * 0.5, 1);
}