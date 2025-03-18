#include "SamplerState.hlsli"
#include "GlobalConsts.hlsli"

cbuffer MeshConsts : register(b1)
{
    matrix world;
    matrix worldIT;
};

cbuffer SpriteConsts : register(b2)
{
    float2 maxFrame;
    float2 curFrame;
}

struct VertexSprite
{
    float4 pos;
    float2 size;
    float2 padding;
};

struct GSInput
{
    float3 pos : POSITION;
    float2 size : SIZE;
};

StructuredBuffer<VertexSprite> particles : register(t0);

GSInput VSMain(uint vertexID : SV_VertexID)
{
    GSInput output;
    
    VertexSprite input = particles[vertexID];
    
    output.pos = input.pos.xyz;
    output.size = input.size;

    return output;
}

struct PSInput
{
    float4 pos : SV_Position;
    float2 texCoord : TEXCOORD;
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
    
    float3 forward = camPos - input[0].pos;
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
	
	[unroll]
    for (int i = 0; i < 4; ++i)
    {
        pixelInput.pos = mul(vertices[i], view);
        pixelInput.pos = mul(pixelInput.pos, proj);
        pixelInput.texCoord = texCoord[i];
		// pushback
        output.Append(pixelInput);
    }
}

Texture2D albedoTex : register(t1);

float4 PSMain(PSInput input) : SV_Target
{
    float2 texCoord = (input.texCoord / maxFrame) + (curFrame / maxFrame);
    
    float4 color = albedoTex.Sample(linearWrapSS, texCoord);
    
    return color;
}