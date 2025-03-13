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
    uint option0;
    float option1;
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

float3 Uncharted2ToneMapping(float3 color)
{
    float A = 0.15;
    float B = 0.50;
    float C = 0.10;
    float D = 0.20;
    float E = 0.02;
    float F = 0.30;
    float W = 11.2;
    
    color *= exposure;
    color = ((color * (A * color + C * B) + D * E) / (color * (A * color + B) + D * F)) - E / F;
    float white = ((W * (A * W + C * B) + D * E) / (W * (A * W + B) + D * F)) - E / F;
    color /= white;
    color = pow(color, float3(1.0, 1.0, 1.0) / gamma);
    return color;
}

float3 FilmicToneMapping(float3 color)
{
    color = max(float3(0, 0, 0), color);
    color = (color * (6.2 * color + .5)) / (color * (6.2 * color + 1.7) + 0.06);
    return color;
}

float4 PSMain(PostProcessPSInput input) : SV_TARGET
{
    float3 color0 = tex0.Sample(linearWrapSS, input.texCoord).rgb;
    float3 color1 = tex1.Sample(linearWrapSS, input.texCoord).rgb;
    
    float3 combined = (1.0 - strength) * color0 + strength * color1;

    [flatten]
    if (0 == option0)
        combined = LinearToneMapping(combined);
    else if (1 == option0)
        combined = Uncharted2ToneMapping(combined);
    else if (2 == option0)
        combined = FilmicToneMapping(combined);
    
    return float4(combined, 1.0);
}