// NOTE: Configurations:
/*
#define SPRITE_SINGLE_TEXTURE_CHECKERBOARD		{0 ... 1}
#define SPRITE_SINGLE_TEXTURE_FONT_BORDER		{0 ... 1}
#define SPRITE_SINGLE_TEXTURE_TEXTURE_MASK		{0 ... 1}
#define SPRITE_SINGLE_TEXTURE_MULTIPLY_BLEND	{0 ... 1}
*/

#if !defined(SPRITE_SINGLE_TEXTURE_CHECKERBOARD)
#error Define SPRITE_SINGLE_TEXTURE_CHECKERBOARD to either 0 or 1
#endif /* SPRITE_SINGLE_TEXTURE_CHECKERBOARD */

#if !defined(SPRITE_SINGLE_TEXTURE_FONT_BORDER)
#error Define SPRITE_SINGLE_TEXTURE_FONT_BORDER to either 0 or 1
#endif /* SPRITE_SINGLE_TEXTURE_FONT_BORDER */

#if !defined(SPRITE_SINGLE_TEXTURE_TEXTURE_MASK)
#error Define SPRITE_SINGLE_TEXTURE_TEXTURE_MASK to either 0 or 1
#endif /* SPRITE_SINGLE_TEXTURE_TEXTURE_MASK */

#if !defined(SPRITE_SINGLE_TEXTURE_MULTIPLY_BLEND)
#error Define SPRITE_SINGLE_TEXTURE_MULTIPLY_BLEND to either 0 or 1
#endif /* SPRITE_SINGLE_TEXTURE_MULTIPLY_BLEND */

#if SPRITE_SINGLE_TEXTURE_MULTIPLY_BLEND
#include "../../Common/BlendModes.hlsl"
#endif /* SPRITE_SINGLE_TEXTURE_MULTIPLY_BLEND */

#if SPRITE_SINGLE_TEXTURE_CHECKERBOARD
#include "../../Common/Checkerboard.hlsl"
#endif /* SPRITE_SINGLE_TEXTURE_CHECKERBOARD*/

#if SPRITE_SINGLE_TEXTURE_FONT_BORDER
#include "../../Common/Font.hlsl"
#endif /* SPRITE_SINGLE_TEXTURE_FONT_BORDER */

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

#if SPRITE_SINGLE_TEXTURE_FONT_BORDER
SamplerState FontSampler		: register(s0);
Texture2D FontTexture			: register(t0);
#else /* */
SamplerState SpriteSampler      : register(s0);
Texture2D SpriteTexture         : register(t0);
Texture2D SpriteMaskTexture     : register(t1);
#endif /* SPRITE_SINGLE_TEXTURE_FONT_BORDER */

float4 PS_main(VS_OUTPUT input) : SV_Target
{
	float4 outputColor;

#if SPRITE_SINGLE_TEXTURE_FONT_BORDER
	outputColor = SampleFontWithBorder(FontTexture, FontSampler, input.TexCoord, input.Color);
#else /* */
	outputColor = input.Color;
#endif /* SPRITE_SINGLE_TEXTURE_FONT_BORDER */

#if SPRITE_SINGLE_TEXTURE_CHECKERBOARD
	outputColor *= GetCheckerboardFactor(input.TexCoord, CB_CheckerboardSize.xy);
#endif /* SPRITE_SINGLE_TEXTURE_CHECKERBOARD */

#if SPRITE_SINGLE_TEXTURE_TEXTURE_MASK
	outputColor.rgba *= FormatAwareSampleTextureRGBA(SpriteTexture, SpriteSampler, input.TexMaskCoord, CB_TextureFormat);
	outputColor.a *= FormatAwareSampleTextureAlpha(SpriteMaskTexture, SpriteSampler, input.TexCoord, CB_TextureMaskFormat);
#endif /* SPRITE_SINGLE_TEXTURE_TEXTURE_MASK */

#if SPRITE_SINGLE_TEXTURE_MULTIPLY_BLEND
	return AdjustMultiplyBlending(outputColor);
#else /* */
	return outputColor;
#endif /* SPRITE_SINGLE_TEXTURE_MULTIPLY_BLEND */
}
