// NOTE: Configurations:
/*
#define SPRITE_MULTI_TEXTURE_TEXTURE_SLOTS	{1 ... 32}
#define SPRITE_MULTI_TEXTURE_MULTIPLY_BLEND	{0 ...  1}
*/

#if !defined(SPRITE_MULTI_TEXTURE_TEXTURE_SLOTS)
#error Define SPRITE_MULTI_TEXTURE_TEXTURE_SLOTS less than or equal to the number of texture bounds by the Renderer2D for the current draw call
#endif /* SPRITE_MULTI_TEXTURE_TEXTURE_SLOTS */

#if !defined(SPRITE_MULTI_TEXTURE_MULTIPLY_BLEND)
#error Define SPRITE_MULTI_TEXTURE_MULTIPLY_BLEND to either 0 or 1
#endif /* SPRITE_MULTI_TEXTURE_MULTIPLY_BLEND */

#if SPRITE_MULTI_TEXTURE_MULTIPLY_BLEND
#include "../../Common/BlendModes.hlsl"
#endif /* SPRITE_MULTI_TEXTURE_MULTIPLY_BLEND */

#include "../../Common/YCbCr.hlsl"

struct VS_OUTPUT
{
	float4 Position         : SV_POSITION;
	float2 TexCoord         : TEXCOORD0;
	float2 TexMaskCoord     : TEXCOORD1;
	float4 Color            : COLOR;
	uint TextureIndex       : TEXINDEX;
};

cbuffer SpriteConstantData : register(b0)
{
	int CB_BlendMode;
	int CB_TextureFormat;
	int CB_TextureMaskFormat;
	uint CB_TextureFormatFlags;
	float4 CB_CheckerboardSize;
};

SamplerState SpriteSampler : register(s0);
Texture2D SpriteTextures[SPRITE_MULTI_TEXTURE_TEXTURE_SLOTS] : register(t0);

float4 PS_main(VS_OUTPUT input) : SV_Target
{
	float4 outputColor = input.Color;

	// NOTE: Minor optimization because reading the vertex TextureIndex is redundant
#if SPRITE_MULTI_TEXTURE_TEXTURE_SLOTS == 1

	if (CB_TextureFormatFlags != 0)
		outputColor *= RGTC2_ConvertYACbCrToRGBA(SpriteTextures[0], SpriteSampler, input.TexCoord);
	else
		outputColor *= SpriteTextures[0].Sample(SpriteSampler, input.TexCoord);

#else /* SPRITE_MULTI_TEXTURE_TEXTURE_SLOTS > 1 */

	[unroll]
	for (uint i = 0; i < SPRITE_MULTI_TEXTURE_TEXTURE_SLOTS; i++)
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

#endif /* SPRITE_MULTI_TEXTURE_TEXTURE_SLOTS */

#if SPRITE_MULTI_TEXTURE_MULTIPLY_BLEND
	return AdjustMultiplyBlending(outputColor);
#else /* */
	return outputColor;
#endif /* SPRITE_MULTI_TEXTURE_MULTIPLY_BLEND */
}
