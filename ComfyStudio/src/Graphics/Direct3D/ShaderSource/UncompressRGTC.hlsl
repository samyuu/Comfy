#ifndef UNCOMPRESSRGTC_HLSL
#define UNCOMPRESSRGTC_HLSL

#include "TextureFormats.hlsl"

static const float GRAYSCALE_MIPMAP = 0.0;
static const float LUMINANCE_MIPMAP = 1.0;

static const float3 RED_COEF = { +1.5748, 1.0, +0.0000 };
static const float3 GRN_COEF = { -0.4681, 1.0, -0.1873 };
static const float3 BLU_COEF = { +0.0000, 1.0, +1.8556 };

static const float LUMINANCE_FACTOR = 1.003922;
static const float LUMINANCE_OFFSET = 0.503929;

float4 UncompressRGTC_RGBA(const Texture2D inputTexture, const SamplerState inputSampler, const float2 texCoord)
{
    float4 grayscale = inputTexture.SampleLevel(inputSampler, texCoord, GRAYSCALE_MIPMAP).rrrg;
    float4 luminance = inputTexture.SampleLevel(inputSampler, texCoord, LUMINANCE_MIPMAP) * LUMINANCE_FACTOR - LUMINANCE_OFFSET;
	grayscale.rb = luminance.gr;
		
    return float4(
        dot(grayscale.rgb, RED_COEF),
        dot(grayscale.rgb, GRN_COEF),
        dot(grayscale.rgb, BLU_COEF),
        grayscale.a);
}

float UncompressRGTC_Alpha(const Texture2D inputTexture, const SamplerState inputSampler, const float2 texCoord)
{
    // NOTE: Because in this case we only need to sample the grayscale mipmap
    return inputTexture.SampleLevel(inputSampler, texCoord, GRAYSCALE_MIPMAP).g;
}

float4 FormatAwareSampleTexture_RGBA(const Texture2D inputTexture, const SamplerState inputSampler, const float2 texCoord, TextureFormat format)
{
    if (format == TextureFormat_RGTC2)
        return UncompressRGTC_RGBA(inputTexture, inputSampler, texCoord);
    else
        return inputTexture.Sample(inputSampler, texCoord);
}

float FormatAwareSampleTexture_Alpha(const Texture2D inputTexture, const SamplerState inputSampler, const float2 texCoord, TextureFormat format)
{
    if (format == TextureFormat_RGTC2)
        return UncompressRGTC_Alpha(inputTexture, inputSampler, texCoord);
    else
        return inputTexture.Sample(inputSampler, texCoord).a;
}

#endif /* UNCOMPRESSRGTC_HLSL */
