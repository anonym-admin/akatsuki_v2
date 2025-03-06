Texture2D texDiffuse : register(t0);
SamplerState samplerDiffuse : register(s0);

cbuffer SpriteConst : register(b0)
{
    float2 c_screenRes;
    float2 c_pos;
    float2 c_scale;
    float2 c_texSize;
    float2 c_texSamplePos;
    float2 c_texSampleSize;
    float c_depth;
    float c_alpha;
    float c_reserved0;
    float c_reserved1;
    uint c_drawBackground;
    float3 c_fontColor;
};

struct VSInput
{
    float4 pos : POSITION;
    float4 color : COLOR;
    float2 texCoord : TEXCOORD0;
};

struct PSInput
{
    float4 pos : SV_POSITION;
    float4 color : COLOR;
    float2 texCoord : TEXCOORD0;
};

PSInput VSMain(VSInput input)
{
    PSInput result = (PSInput) 0;
    
    float2 scale = (c_texSize / c_screenRes) * c_scale;
    float2 offset = (c_pos / c_screenRes); // float좌표계 기준 지정한 위치
    float2 screenPosition = input.pos.xy * scale + offset;
    result.pos = float4(screenPosition.xy * float2(2, -2) + float2(-1, 1), c_depth, 1); // 정규좌표게로 변환
 
    float2 texScale = (c_texSampleSize / c_texSize);
    float2 texOffset = (c_texSamplePos / c_texSize);
    result.texCoord = input.texCoord * texScale + texOffset;
    //result.TexCoord = mul(float4(input.TexCoord, 0, 1), g_matTransformUV);
    
    result.color = input.color;
    
    return result;
}

float4 PSMain(PSInput input) : SV_TARGET
{
    float4 texColor = texDiffuse.Sample(samplerDiffuse, input.texCoord);
    
    // 배경이 검정색일 경우 해당 픽셀을 그리지 않는다.
    if (!c_drawBackground)
    {
        clip(texColor.r + texColor.g + texColor.b < 0.3 ? -1 : 1);
        texColor.rgb = c_fontColor;
    }
    
    return texColor * input.color;
}
