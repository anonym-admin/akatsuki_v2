Texture2D resolvedTex : register(t0);
SamplerState linearWrapSS : register(s0);

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

static const float exposure = 1.0;
static const float gamma = 2.2;

float3 LinearToneMapping(float3 color)
{
    float3 invGamma = float3(1, 1, 1) / gamma;

    color = clamp(exposure * color, 0., 1.);
    color = pow(color, invGamma);
    return color;
}

float4 PSMain(PostProcessPSInput input) : SV_TARGET
{
    float3 baseColor = resolvedTex.Sample(linearWrapSS, input.texCoord).xyz;
    float3 toneMappingColor = float3(0.0, 0.0, 0.0);
    
    toneMappingColor = LinearToneMapping(baseColor);
    
    return float4(toneMappingColor, 1.0);
}