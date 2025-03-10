#include "Common.hlsli"

Texture2DArray textureArray : register(t0);

struct BillboardVSInput
{
    float3 posModel : POSITION;
    float2 size : SIZE;
};

struct BillboardGSInput
{
    float3 center : POSITION;
    float2 size : SIZE;
};

struct BillboardPSInput
{
    float4 posProj : SV_POSITION;
    float3 posModel : POSITION;
    float3 normal : NORMAL;
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

[maxvertexcount(4)]
void GSMain(point BillboardGSInput input[1], uint primID : SV_PrimitiveID, inout TriangleStream<BillboardPSInput> outputStream)
{
    BillboardPSInput output;
    
    float3 up = float3(0.0, 1.0, 0.0);
    float3 look = eyeWorld - input[0].center;
    look.y = 0.0;
    look = normalize(look);
    
    float3 right = cross(up, look);
    
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
        output.posProj = mul(v[i], view);
        output.posProj = mul(output.posProj, proj);
        output.posModel = v[i].xyz;
        output.normal = look;
        output.texCoord = texCoord[i];
        output.primID = primID;

        outputStream.Append(output);
    }
}

float4 PSMain(BillboardPSInput input) : SV_TARGET
{
    float3 uvw = float3(input.texCoord, input.primID % 3);
    float4 color = textureArray.Sample(anisotropicWrapSS, uvw);
    
    return color;
}
