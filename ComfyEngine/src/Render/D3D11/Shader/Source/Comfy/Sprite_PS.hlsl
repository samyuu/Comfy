#include "../Include/Comfy/BlendModes.hlsl"
#include "../Include/Comfy/Checkerboard.hlsl"
#include "../Include/Comfy/UncompressRGTC.hlsl"
#include "../Include/Comfy/Font.hlsl"

#define COMFY_RENDERER2D_SINGLE_TEXTURE_BATCH

struct VS_OUTPUT
{
    float4 Position         : SV_POSITION;
    float2 TexCoord         : TEXCOORD0;
    float2 TexMaskCoord     : TEXCOORD1;
    float4 Color            : COLOR;
    
#if !defined(COMFY_RENDERER2D_SINGLE_TEXTURE_BATCH)
    uint TextureIndex      : TEXINDEX;
#endif
};

static const uint SpriteTextureSlots = 7;

cbuffer SpriteConstantData : register(b0)
{
#if defined(COMFY_RENDERER2D_SINGLE_TEXTURE_BATCH)
    int CB_TextureFormat;
    int CB_TextureMaskFormat;
#endif

    int CB_BlendMode;
    int CB_Flags;
    int CB_DrawTextBorder;
    int CB_DrawCheckerboard;
    
    float4 CB_CheckerboardSize;
    
#if !defined(COMFY_RENDERER2D_SINGLE_TEXTURE_BATCH)
    int CB_TextureFormats[SpriteTextureSlots + 1];
#endif
};

#if defined(COMFY_RENDERER2D_SINGLE_TEXTURE_BATCH)
SamplerState SpriteSampler      : register(s0);
SamplerState SpriteMaskSampler  : register(s1);

Texture2D SpriteTexture         : register(t0);
Texture2D SpriteMaskTexture     : register(t1);
#else
SamplerState SpriteSampler : register(s0);

Texture2D SpriteMaskTexture                     : register(t0);
Texture2D SpriteTextures[SpriteTextureSlots]    : register(t1);
#endif

float4 PS_main(VS_OUTPUT input) : SV_Target
{
    float4 outputColor = input.Color;

#if defined(COMFY_RENDERER2D_SINGLE_TEXTURE_BATCH)
    if (CB_DrawCheckerboard)
    {
        outputColor *= GetCheckerboardFactor(input.TexCoord, CB_CheckerboardSize.xy);
    }
    else if (CB_DrawTextBorder)
    {
        outputColor = SampleFontWithBorder(SpriteTexture, SpriteSampler, input.TexCoord, input.Color);
    }
    else if (CB_TextureMaskFormat > TextureFormat_Unknown)
    {
        outputColor.rgba *= FormatAwareSampleTexture_RGBA(SpriteTexture, SpriteSampler, input.TexMaskCoord, CB_TextureFormat);
        outputColor.a *= FormatAwareSampleTexture_Alpha(SpriteMaskTexture, SpriteMaskSampler, input.TexCoord, CB_TextureMaskFormat);
    }
    else if (CB_TextureFormat > TextureFormat_Unknown)
    {
        outputColor *= FormatAwareSampleTexture_RGBA(SpriteTexture, SpriteSampler, input.TexCoord, CB_TextureFormat);
    }
#else
    const uint spriteIndex = uint(input.TextureIndex);
    const int textureFormat = CB_TextureFormats[spriteIndex + 1];
    const int textureMaskFormat = CB_TextureFormats[0];
    
    if (CB_DrawCheckerboard)
    {
        outputColor *= GetCheckerboardFactor(input.TexCoord, CB_CheckerboardSize.xy);
    }
    else if (spriteIndex < SpriteTextureSlots)
    {
        if (CB_DrawTextBorder)
        {
            [unroll]
            for (uint i = 0; i < SpriteTextureSlots; i++) 
            { 
                if (i == spriteIndex) 
                {
                    outputColor = SampleFontWithBorder(SpriteTextures[i], SpriteSampler, input.TexCoord, input.Color);
                    break;
                }
            }
        }
        else if (textureMaskFormat > TextureFormat_Unknown)
        {
            [unroll]
            for (uint i = 0; i < SpriteTextureSlots; i++) 
            { 
                if (i == spriteIndex) 
                {
                    outputColor.rgba *= FormatAwareSampleTexture_RGBA(SpriteTextures[i], SpriteSampler, input.TexMaskCoord, textureFormat);
                    outputColor.a *= FormatAwareSampleTexture_Alpha(SpriteMaskTexture, SpriteSampler, input.TexCoord, textureMaskFormat);
                    break;
                }
            }
        }
        else if (textureFormat > TextureFormat_Unknown)
        {
            [unroll]
            for (uint i = 0; i < SpriteTextureSlots; i++) 
            { 
                if (i == spriteIndex) 
                {
                    outputColor *= FormatAwareSampleTexture_RGBA(SpriteTextures[i], SpriteSampler, input.TexCoord, textureFormat);
                    break;
                }
            }
        }
    }
#endif
    
    if (CB_BlendMode == BlendMode_Multiply)
        outputColor = AdjustMultiplyBlending(outputColor);

    return outputColor;
}
