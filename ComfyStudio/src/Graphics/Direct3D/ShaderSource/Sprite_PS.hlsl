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
    int CB_TextureFormat;
    int CB_TextureMaskFormat;
    int CB_BlendMode;

    // TODO: This should be a general flags value (?)
    bool CB_DrawTextBorder;
}

SamplerState SpriteSampler;
SamplerState SpriteMaskSampler;

Texture2D SpriteTexture;
Texture2D SpriteMaskTexture;

float4 SampleTexture_RGBA(const float2 texCoord)
{
    return FormatAwareSampleTexture_RGBA(SpriteTexture, SpriteSampler, texCoord, CB_TextureFormat);
}

float4 SampleTextureMask_RGBA(const float2 texCoord)
{
	// NOTE: Special case for when the texture mask shares the same texture
    if (CB_TextureFormat < 0)
        return FormatAwareSampleTexture_RGBA(SpriteMaskTexture, SpriteMaskSampler, texCoord, CB_TextureMaskFormat);
	else
        return FormatAwareSampleTexture_RGBA(SpriteTexture, SpriteSampler, texCoord, CB_TextureFormat);
}

float SampleTextureMask_Alpha(const float2 texCoord)
{
    // TODO: ~~Shouldn't this also check for (CB_TextureFormat < 0) (?)~ if so that branch should be moved into PS_MAIN
    return FormatAwareSampleTexture_Alpha(SpriteMaskTexture, SpriteMaskSampler, texCoord, CB_TextureMaskFormat);
}

float4 PS_MAIN(VS_OUTPUT input) : SV_Target
{
    float4 outputColor = input.Color;

    if (CB_TextureMaskFormat >= 0)
    {
        outputColor.rgba *= SampleTextureMask_RGBA(input.TexCoord);
        outputColor.a *= SampleTextureMask_Alpha(input.TexCoord);
    }
    else if (CB_TextureFormat >= 0)
    {
        outputColor *= SampleTexture_RGBA(input.TexCoord);
    }
    
    if (CB_DrawTextBorder)
        outputColor = SampleFontWithBorder(SpriteTexture, SpriteSampler, input.TexCoord, input.Color);

    if (CB_BlendMode == BlendMode_Multiply)
        outputColor = AdjustMultiplyBlending(outputColor);
    
    return outputColor;
}
