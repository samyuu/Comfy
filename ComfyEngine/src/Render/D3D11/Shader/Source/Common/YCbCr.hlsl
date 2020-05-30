#ifndef YCBCR_HLSL
#define YCBCR_HLSL

#include "TextureFormats.hlsl"

// NOTE: MipMaps[0]: Luma, Alpha
//       MipMaps[1]: Chroma blue, Chroma red
static const float RGTC2_TEXTURE_MIPMAP_YA = 0.0;
static const float RGTC2_TEXTURE_MIPMAP_CbCr = 1.0;

// NOTE: Textures[0]: Luma
//       Textures[1]: Chroma blue
//       Textures[2]: Chroma red
static const float RGTC1_TEXTURE_MIPMAP_Y = 0.0;
static const float RGTC1_TEXTURE_MIPMAP_Cb = 1.0;
static const float RGTC1_TEXTURE_MIPMAP_Cr = 2.0;

static const float3 RED_COEF_709 = { +1.5748, +1.0, +0.0000 };
static const float3 GRN_COEF_709 = { -0.4681, +1.0, -0.1873 };
static const float3 BLU_COEF_709 = { +0.0000, +1.0, +1.8556 };

static const float OFFSET_709 = 0.503929;
static const float FACTOR_709 = 1.003922;

float RGTC2_AccessY(float2 yA) { return yA.x; }
float RGTC2_AccessA(float2 yA) { return yA.y; }
float RGTC2_AccessCb(float2 cbCr) { return cbCr.x; }
float RGTC2_AccessCr(float2 cbCr) { return cbCr.y; }

// NOTE: Used by RGTC2 encoded sprite sheets
float4 RGTC2_ConvertYACbCrToRGBA(Texture2D inputTexture, SamplerState inputSampler, const float2 texCoord)
{
	const float2 textureYA = inputTexture.SampleLevel(inputSampler, texCoord, RGTC2_TEXTURE_MIPMAP_YA).xy;
	const float2 textureCbCr = inputTexture.SampleLevel(inputSampler, texCoord, RGTC2_TEXTURE_MIPMAP_CbCr).xy;
	
	const float2 cbCr = (textureCbCr * FACTOR_709 - OFFSET_709);
	const float3 mixed = float3(RGTC2_AccessCr(cbCr), RGTC2_AccessY(textureYA), RGTC2_AccessCb(cbCr));
	
	return float4(
		dot(mixed, RED_COEF_709),
		dot(mixed, GRN_COEF_709),
		dot(mixed, BLU_COEF_709),
		RGTC2_AccessA(textureYA));
}

// NOTE: Optimization specifically for sampling alpha masks, rarely used in practice
float RGTC2_ConvertYACbCrToAlpha(Texture2D inputTexture, SamplerState inputSampler, const float2 texCoord)
{
	const float2 textureYA = inputTexture.SampleLevel(inputSampler, texCoord, RGTC2_TEXTURE_MIPMAP_YA).xy;
	return RGTC2_AccessA(textureYA);
}

float4 FormatAwareSampleTextureRGBA(Texture2D inputTexture, SamplerState inputSampler, const float2 texCoord, int textureFormat)
{
	if (textureFormat == TextureFormat_RGTC2)
	    return RGTC2_ConvertYACbCrToRGBA(inputTexture, inputSampler, texCoord);
	else
	    return inputTexture.Sample(inputSampler, texCoord);
}

float FormatAwareSampleTextureAlpha(Texture2D inputTexture, SamplerState inputSampler, const float2 texCoord, int textureFormat)
{
	if (textureFormat == TextureFormat_RGTC2)
	    return RGTC2_ConvertYACbCrToAlpha(inputTexture, inputSampler, texCoord);
	else
	    return inputTexture.Sample(inputSampler, texCoord).a;
}

// NOTE: Sometimes used by RGTC1 encoded material textures
float3 RGTC1_ConvertYCbCrToRGB(Texture2D inputTexture, SamplerState inputSampler, const float2 texCoord)
{
	float3 texColor;
	texColor.y = inputTexture.SampleLevel(inputSampler, texCoord, RGTC1_TEXTURE_MIPMAP_Y).x;
	texColor.x = inputTexture.SampleLevel(inputSampler, texCoord, RGTC1_TEXTURE_MIPMAP_Cb).x;
	texColor.z = inputTexture.SampleLevel(inputSampler, texCoord, RGTC1_TEXTURE_MIPMAP_Cr).x;
	texColor.xz = mad(texColor.xz, FACTOR_709, -OFFSET_709);
	
	return float3(
	    dot(texColor, RED_COEF_709),
	    dot(texColor, GRN_COEF_709),
	    dot(texColor, BLU_COEF_709));
}

#endif /* YCBCR_HLSL */
