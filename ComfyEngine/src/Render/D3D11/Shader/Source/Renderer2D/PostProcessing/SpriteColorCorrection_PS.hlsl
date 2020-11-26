#include "../../Common/ColorCorrection.hlsl"

struct VS_OUTPUT
{
    float4 Position     : SV_POSITION;
    float2 TexCoord     : TEXCOORD0;
};

cbuffer PostProcessConstantData : register(b1)
{
    float4 CB_PostProcessParam;
    float4 CB_PostProcessCoefficients[3];
};

SamplerState LinearTextureSampler
{
    Filter = MIN_MAG_MIP_LINEAR;
    AddressU = CLAMP;
    AddressV = CLAMP;
};

Texture2D ScreenTexture : register(t0);

float4 PS_main(VS_OUTPUT input) : SV_Target
{
    const float4 screenColor = ScreenTexture.Sample(LinearTextureSampler, input.TexCoord);
    const float4 screenColorSaturated =  ColorCorrect_PP(screenColor, 
        CB_PostProcessParam[0], 
        CB_PostProcessParam[1], 
        CB_PostProcessCoefficients[0].rgb, 
        CB_PostProcessCoefficients[1].rgb,
        CB_PostProcessCoefficients[2].rgb);
    
    return screenColorSaturated;
}
