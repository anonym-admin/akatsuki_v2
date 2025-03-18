#include "SamplerState.hlsli"
#include "GlobalConsts.hlsli"

cbuffer MeshConsts : register(b1)
{
    matrix world;
    matrix worldIT;
};

cbuffer SparkConsts : register(b2)
{
    float time;
    float duration;
    float2 startSize;
    float3 startDirection;
    float sizeOverLifeTime;
    float3 rotOverLifeTime;
};

cbuffer ColorConsts : register(b3)
{
    float4 totalColor;
    float4 colorOverLifeTime;
}

struct VertexParticle
{
    float4 pos;
    float4 particleColor;
    
    float startLifeTime;
    float speed;
    float2 size;
    
    float3 direction;
    float padding;
};

struct GSInput
{
    float3 pos : POSITION;
    float2 size : SIZE;
    float time : TIME;
    float4 particleColor : COLOR;
};

StructuredBuffer<VertexParticle> particles : register(t0);

GSInput VSMain(uint vertexID : SV_VertexID)
{
    GSInput output;
    
    VertexParticle input = particles[vertexID];
    
    output.time = time / input.startLifeTime;
    input.direction += startDirection * time;
    input.pos.w = 1.0;
    input.pos = mul(input.pos, world);
    output.pos = input.pos.xyz + (input.direction * time) * input.speed;
    output.size = input.size;
    output.particleColor = input.particleColor;
    
    return output;
}

struct PSInput
{
    float4 pos : SV_Position;
    float2 texCoord : UV;
    float time : Time;
    float4 particleColor : Color;
};

static const float2 texCoord[4] =
{
    float2(0.0f, 1.0f),
	float2(0.0f, 0.0f),
	float2(1.0f, 1.0f),
	float2(1.0f, 0.0f)
};

[maxvertexcount(4)]
void GSMain(point GSInput input[1], inout TriangleStream<PSInput> output)
{
    float3 camPos = invView._41_42_43;
    float3 up = invView._21_22_23;
    
    // float3 up = float3(0.0, 1.0, 0.0);
    
    float3 forward = eyeWorld - input[0].pos;
    forward = normalize(forward);
   
    float3 right = normalize(cross(up, forward));

    float halfWidth = input[0].size.x * 0.5f;
    float halfHeight = input[0].size.y * 0.5f;
	
    float4 vertices[4];
    vertices[0] = float4(input[0].pos + halfWidth * right - halfHeight * up, 1.0f);
    vertices[1] = float4(input[0].pos + halfWidth * right + halfHeight * up, 1.0f);
    vertices[2] = float4(input[0].pos - halfWidth * right - halfHeight * up, 1.0f);
    vertices[3] = float4(input[0].pos - halfWidth * right + halfHeight * up, 1.0f);
	
    PSInput pixelInput;
    pixelInput.time = input[0].time;
	
	[unroll]
    for (int i = 0; i < 4; ++i)
    {
        pixelInput.pos = mul(vertices[i], view);
        pixelInput.pos = mul(pixelInput.pos, proj);
        pixelInput.texCoord = texCoord[i];
        pixelInput.particleColor = input[0].particleColor;
		// pushback
        output.Append(pixelInput);
    }
}

Texture2D albedoTex : register(t1);

float4 PSMain(PSInput input) : SV_Target
{
    float4 albedo = albedoTex.Sample(linearWrapSS, input.texCoord);
	
    float4 color = lerp(input.particleColor, colorOverLifeTime, input.time);
    
    return albedo * color;
}