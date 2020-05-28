#include "../Include/Comfy/BlendModes.hlsl"
#include "../Include/Comfy/Checkerboard.hlsl"
#include "../Include/Comfy/UncompressRGTC.hlsl"
#include "../Include/Comfy/Font.hlsl"

struct VS_OUTPUT
{
    float4 Position         : SV_POSITION;
    float2 TexCoord         : TEXCOORD0;
    float2 TexMaskCoord     : TEXCOORD1;
    float4 Color            : COLOR;
    
#if !defined(COMFY_ENGINE_RENDERER2D_IMPL_FIXED_TEX)
    uint2 TextureIndices    : TEXINDEX;
#endif
};

static const uint SpriteTextureSlots = 16;

cbuffer SpriteConstantData : register(b0)
{
#if defined(COMFY_ENGINE_RENDERER2D_IMPL_FIXED_TEX)
    int CB_TextureFormat;
    int CB_TextureMaskFormat;
#else
    int CB_TextureFormats[SpriteTextureSlots];
    int CB_TextureMaskFormats[SpriteTextureSlots];
#endif

    int CB_BlendMode;

    bool CB_Flags;
    bool CB_DrawTextBorder;

    bool CB_DrawCheckerboard;
    float2 CB_CheckerboardSize;
};

#if defined(COMFY_ENGINE_RENDERER2D_IMPL_FIXED_TEX)
SamplerState SpriteSampler      : register(s0);
SamplerState SpriteMaskSampler  : register(s1);

Texture2D SpriteTexture         : register(t0);
Texture2D SpriteMaskTexture     : register(t1);
#else
SamplerState SpriteSamplers[SpriteTextureSlots]    : register(s0);
Texture2D SpriteTextures[SpriteTextureSlots]       : register(t1);

// WTF something like this is only supported for shader model 5.1 D3D12?? PepeHands
SamplerState IndexSampler(uint index)
{
    //if (index ==  0) return SpriteSamplers[ 0];
    //if (index ==  1) return SpriteSamplers[ 1];
    //if (index ==  2) return SpriteSamplers[ 2];
    //if (index ==  3) return SpriteSamplers[ 3];
    //if (index ==  4) return SpriteSamplers[ 4];
    //if (index ==  5) return SpriteSamplers[ 5];
    //if (index ==  6) return SpriteSamplers[ 6];
    //if (index ==  7) return SpriteSamplers[ 7];
    //if (index ==  8) return SpriteSamplers[ 8];
    //if (index ==  9) return SpriteSamplers[ 9];
    //if (index == 10) return SpriteSamplers[10];
    //if (index == 11) return SpriteSamplers[11];
    //if (index == 12) return SpriteSamplers[12];
    //if (index == 13) return SpriteSamplers[13];
    //if (index == 14) return SpriteSamplers[14];
    //if (index == 15) return SpriteSamplers[15];
    
    //[unroll]
    //for (uint i = 0; i < SpriteTextureSlots; i++) { if (i == index) { return SpriteSamplers[i]; } }
    return SpriteSamplers[0];
}

Texture2D IndexTexture(uint index)
{
    //if (index ==  0) return SpriteTextures[ 0];
    //if (index ==  1) return SpriteTextures[ 1];
    //if (index ==  2) return SpriteTextures[ 2];
    //if (index ==  3) return SpriteTextures[ 3];
    //if (index ==  4) return SpriteTextures[ 4];
    //if (index ==  5) return SpriteTextures[ 5];
    //if (index ==  6) return SpriteTextures[ 6];
    //if (index ==  7) return SpriteTextures[ 7];
    //if (index ==  8) return SpriteTextures[ 8];
    //if (index ==  9) return SpriteTextures[ 9];
    //if (index == 10) return SpriteTextures[10];
    //if (index == 11) return SpriteTextures[11];
    //if (index == 12) return SpriteTextures[12];
    //if (index == 13) return SpriteTextures[13];
    //if (index == 14) return SpriteTextures[14];
    //if (index == 15) return SpriteTextures[15];
    
    //[unroll]
    //for (uint i = 0; i < SpriteTextureSlots; i++) { if (i == index) { return SpriteTextures[i]; } }
    return SpriteTextures[0];
}
#endif

float4 PS_main(VS_OUTPUT input) : SV_Target
{
    float4 outputColor = input.Color;

#if defined(COMFY_ENGINE_RENDERER2D_IMPL_FIXED_TEX)
    if (CB_DrawCheckerboard)
    {
        outputColor *= GetCheckerboardFactor(input.TexCoord, CB_CheckerboardSize);
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
    const uint spriteIndex = input.TextureIndices[0];
    const uint maskIndex = input.TextureIndices[1];
    
    if (CB_DrawCheckerboard)
    {
        outputColor *= GetCheckerboardFactor(input.TexCoord, CB_CheckerboardSize);
    }
    else if (spriteIndex < SpriteTextureSlots)
    {
        if (CB_DrawTextBorder)
        {
            outputColor = SampleFontWithBorder(IndexTexture(spriteIndex), IndexSampler(spriteIndex), input.TexCoord, input.Color);
        }
        else if (maskIndex < SpriteTextureSlots)
        {
            outputColor.rgba *= FormatAwareSampleTexture_RGBA(IndexTexture(spriteIndex), IndexSampler(spriteIndex), input.TexMaskCoord, CB_TextureFormats[spriteIndex]);
            outputColor.a *= FormatAwareSampleTexture_Alpha(IndexTexture(maskIndex), IndexSampler(maskIndex), input.TexCoord, CB_TextureMaskFormats[maskIndex]);
        }
        else // if (CB_TextureFormat > TextureFormat_Unknown)
        {
            outputColor *= FormatAwareSampleTexture_RGBA(IndexTexture(spriteIndex), IndexSampler(spriteIndex), input.TexCoord, CB_TextureFormats[spriteIndex]);
        }
    }
#endif
    
    if (CB_BlendMode == BlendMode_Multiply)
        outputColor = AdjustMultiplyBlending(outputColor);

    return outputColor;
}
