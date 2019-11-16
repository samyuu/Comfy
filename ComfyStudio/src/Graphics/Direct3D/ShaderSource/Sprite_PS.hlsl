#include "BlendModes.hlsl"
#include "Checkerboard.hlsl"
#include "UncompressRGTC.hlsl"
#include "Font.hlsl"

struct VS_OUTPUT
{
    float4 Position     : SV_POSITION;
    float4 Color        : COLOR;
    float2 TexCoord     : TEXCOORD0;
    float2 TexMaskCoord : TEXCOORD1;
};

cbuffer SpriteConstantBuffer : register(b0)
{
    TextureFormat CB_TextureFormat;
    TextureFormat CB_TextureMaskFormat;
    BlendMode CB_BlendMode;

    bool CB_DrawSolidColor;
    bool CB_DrawTextBorder;

    bool CB_DrawCheckerboard;
    float2 CB_CheckerboardSize;
};

SamplerState SpriteSampler      : register(s0);
SamplerState SpriteMaskSampler  : register(s1);

Texture2D SpriteTexture         : register(t0);
Texture2D SpriteMaskTexture     : register(t1);

float4 PS_MAIN(VS_OUTPUT input) : SV_Target
{
    float4 outputColor = input.Color;

    if (CB_TextureMaskFormat > TextureFormat_Unknown)
    {
        outputColor.rgba *= FormatAwareSampleTexture_RGBA(SpriteTexture, SpriteSampler, input.TexMaskCoord, CB_TextureFormat);
        outputColor.a *= FormatAwareSampleTexture_Alpha(SpriteMaskTexture, SpriteMaskSampler, input.TexCoord, CB_TextureMaskFormat);
    }
    else if (CB_TextureFormat > TextureFormat_Unknown)
    {
        outputColor *= FormatAwareSampleTexture_RGBA(SpriteTexture, SpriteSampler, input.TexCoord, CB_TextureFormat);
    }
    //else if (CB_DrawCheckerboard)
    else if ( (CB_CheckerboardSize.x + CB_CheckerboardSize.y) > 0.0)
    {
        outputColor *= GetCheckerboardFactor(input.TexCoord, CB_CheckerboardSize);
    }
    
    if (CB_DrawTextBorder)
        outputColor = SampleFontWithBorder(SpriteTexture, SpriteSampler, input.TexCoord, input.Color);

    if (CB_BlendMode == BlendMode_Multiply)
        outputColor = AdjustMultiplyBlending(outputColor);

    return outputColor;
}
