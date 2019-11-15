#include "UncompressRGTC.hlsl"
#include "Font.hlsl"

struct VS_OUTPUT
{
    float4 Position : SV_POSITION;
    float4 Color    : COLOR;
    float2 TexCoord : TEXCOORD;
};

cbuffer ColorConstantBuffer : register(b0)
{
    float4 CB_Color;
};

Texture2D Texture       : register(t0);
SamplerState Sampler    : register(s0);

float4 PS_main(VS_OUTPUT input) : SV_Target
{
    //return Texture.Sample(Sampler, input.TexCoord);

    float4 uncompressedRgtc = UncompressRGTC_RGBA(Texture, Sampler, input.TexCoord);
    return uncompressedRgtc;

    //float4 fontBorderColor = SampleFontWithBorder(Texture, Sampler, input.TexCoord, input.Color);
    float4 fontBorderColor = SampleFontWithBorder(Texture, Sampler, input.TexCoord, float4(1.0, 1.0, 1.0, 1.0));
    return fontBorderColor;


    // return float4(1.0, 0.0, 0.0, 1.0);
    float4 baseColor = lerp(input.Color, CB_Color, 0.7);
    float4 textureColor = Texture.Sample(Sampler, input.TexCoord);
    
#if COMFY_DEBUG
    return textureColor * float4(0.96, 0.68, 0.42, 1.0);
#endif

    return textureColor;
}
