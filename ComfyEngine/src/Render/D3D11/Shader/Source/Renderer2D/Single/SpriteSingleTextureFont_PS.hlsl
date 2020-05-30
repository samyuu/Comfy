#include "../../Common/Font.hlsl"

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

SamplerState FontSampler : register(s0);
Texture2D FontTexture    : register(t0);

float4 PS_main(VS_OUTPUT input) : SV_Target
{
    return SampleFontWithBorder(FontTexture, FontSampler, input.TexCoord, input.Color);
}
