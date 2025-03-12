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
    
    float3 a = tex0.Sample(samp, float2(x - 2 * dx, y + 2 * dy)).rgb;
    float3 b = tex0.Sample(samp, float2(x, y + 2 * dy)).rgb;
    float3 c = tex0.Sample(samp, float2(x + 2 * dx, y + 2 * dy)).rgb;

    float3 d = tex0.Sample(samp, float2(x - 2 * dx, y)).rgb;
    float3 e = tex0.Sample(samp, float2(x, y)).rgb;
    float3 f = tex0.Sample(samp, float2(x + 2 * dx, y)).rgb;

    float3 g = tex0.Sample(samp, float2(x - 2 * dx, y - 2 * dy)).rgb;
    float3 h = tex0.Sample(samp, float2(x, y - 2 * dy)).rgb;
    float3 i = tex0.Sample(samp, float2(x + 2 * dx, y - 2 * dy)).rgb;

    float3 j = tex0.Sample(samp, float2(x - dx, y + dy)).rgb;
    float3 k = tex0.Sample(samp, float2(x + dx, y + dy)).rgb;
    float3 l = tex0.Sample(samp, float2(x - dx, y - dy)).rgb;
    float3 m = tex0.Sample(samp, float2(x + dx, y - dy)).rgb;

    float3 color = e * 0.125;
    color += (a + c + g + i) * 0.03125;
    color += (b + d + f + h) * 0.0625;
    color += (j + k + l + m) * 0.125;
  
    return float4(color, 1.0);
}