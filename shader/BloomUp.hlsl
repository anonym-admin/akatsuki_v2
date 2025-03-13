Texture2D tex0 : register(t0);
SamplerState samp : register(s0);

cbuffer Const : register(b0)
{
    float dx;
    float dy;
    float threshold;
    float strength;
    float4 options;
};

struct PSInput
{
    float4 posProj : SV_POSITION;
    float2 texCoord : TEXCOORD;
};

float4 PSMain(PSInput input) : SV_TARGET
{
    float x = input.texCoord.x;
    float y = input.texCoord.y;
    
    float3 a = tex0.Sample(samp, float2(x - dx, y + dy)).rgb;
    float3 b = tex0.Sample(samp, float2(x, y + dy)).rgb;
    float3 c = tex0.Sample(samp, float2(x + dx, y + dy)).rgb;

    float3 d = tex0.Sample(samp, float2(x - dx, y)).rgb;
    float3 e = tex0.Sample(samp, float2(x, y)).rgb;
    float3 f = tex0.Sample(samp, float2(x + dx, y)).rgb;

    float3 g = tex0.Sample(samp, float2(x - dx, y - dy)).rgb;
    float3 h = tex0.Sample(samp, float2(x, y - dy)).rgb;
    float3 i = tex0.Sample(samp, float2(x + dx, y - dy)).rgb;

    float3 color = e * 4.0;
    color += (b + d + f + h) * 2.0;
    color += (a + c + g + i);
    color *= 1.0 / 16.0;
  
    return float4(color, 1.0);
}
