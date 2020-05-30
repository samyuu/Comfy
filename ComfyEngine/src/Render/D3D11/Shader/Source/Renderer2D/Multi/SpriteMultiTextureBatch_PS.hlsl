#include "../../Common/YCbCr.hlsl"

struct VS_OUTPUT
{
    float4 Position         : SV_POSITION;
    float2 TexCoord         : TEXCOORD0;
    float2 TexMaskCoord     : TEXCOORD1;
    float4 Color            : COLOR;
    uint TextureIndex       : TEXINDEX;
};

static const uint SpriteTextureSlots = 7;

cbuffer SpriteConstantData : register(b0)
{
	int CB_BlendMode;
	int CB_TextureFormat;
	int CB_TextureMaskFormat;
	uint CB_TextureFormatFlags;
	float4 CB_CheckerboardSize;
};

SamplerState SpriteSampler                      : register(s0);
Texture2D SpriteTextures[SpriteTextureSlots]    : register(t0);

float4 PS_main(VS_OUTPUT input) : SV_Target
{
    float4 outputColor = input.Color;

    [unroll]
    for (uint i = 0; i < SpriteTextureSlots; i++) 
    { 
        if (i == input.TextureIndex) 
        {
            if (CB_TextureFormatFlags & (1 << i))
                outputColor *= RGTC2_ConvertYACbCrToRGBA(SpriteTextures[i], SpriteSampler, input.TexCoord);
            else
                outputColor *= SpriteTextures[i].Sample(SpriteSampler, input.TexCoord);
            
            break;
        }
    }
    
    return outputColor;
}
