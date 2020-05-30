#include "../../Common/BlendModes.hlsl"
#include "../../Common/YCbCr.hlsl"

struct VS_OUTPUT
{
    float4 Position         : SV_POSITION;
    float2 TexCoord         : TEXCOORD0;
    float2 TexMaskCoord     : TEXCOORD1;
    float4 Color            : COLOR;
};

cbuffer SpriteConstantData : register(b0)
{
	int CB_BlendMode;
	int CB_TextureFormat;
	int CB_TextureMaskFormat;
	uint CB_TextureFormatFlags;
	float4 CB_CheckerboardSize;
};

SamplerState SpriteSampler      : register(s0);
Texture2D SpriteTexture         : register(t0);
Texture2D SpriteMaskTexture     : register(t1);

float4 PS_main(VS_OUTPUT input) : SV_Target
{
    float4 outputColor = input.Color;
    
    outputColor.rgba *= FormatAwareSampleTextureRGBA(SpriteTexture, SpriteSampler, input.TexMaskCoord, CB_TextureFormat);
    outputColor.a *= FormatAwareSampleTextureAlpha(SpriteMaskTexture, SpriteSampler, input.TexCoord, CB_TextureMaskFormat);
    
    return AdjustMultiplyBlending(outputColor);
}
