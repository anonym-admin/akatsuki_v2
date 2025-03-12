Texture2D tex0 : register(t0);
Texture2D tex1 : register(t1);
SamplerState linearWrapSS : register(s0);

cbuffer Const : register(b0)
{
    float dx;
    float dy;
    float threshold;
    float strength;
    float exposure;
    float gamma;
    float option3;
    float option4;
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

float3 LinearToneMapping(float3 color)
{
    float3 invGamma = float3(1, 1, 1) / gamma;

    color = clamp(exposure * color, 0., 1.);
    color = pow(color, invGamma);
    return color;
}

float4 PSMain(PostProcessPSInput input) : SV_TARGET
{
    float3 color0 = tex0.Sample(linearWrapSS, input.texCoord).rgb;
    float3 color1 = tex1.Sample(linearWrapSS, input.texCoord).rgb;
    
    float3 combined = (1.0 - strength) * color0 + strength * color1;

    combined = LinearToneMapping(combined);
    
    return float4(combined, 1.0);
}