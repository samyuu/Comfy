#ifndef UNCOMPRESSRGTC_HLSL
#define UNCOMPRESSRGTC_HLSL

#include "../../Materials/Include/Defines/TextureFormats.hlsl"

// NOTE: MipMap[0]: Luma, Alpha
//       MipMap[1]: Chroma blue, Chroma red
static const float TEXTURE_MIPMAP_YA = 0.0;
static const float TEXTURE_MIPMAP_CbCr = 1.0;

static const float3 RED_COEF_709 = { +1.5748, +1.0, +0.0000 };
static const float3 GRN_COEF_709 = { -0.4681, +1.0, -0.1873 };
static const float3 BLU_COEF_709 = { +0.0000, +1.0, +1.8556 };

static const float OFFSET_709 = 0.503929;
static const float FACTOR_709 = 1.003922;

float AccessY(float2 yA) { return yA.x; }
float AccessA(float2 yA) { return yA.y; }
float AccessCb(float2 cbCr) { return cbCr.x; }
float AccessCr(float2 cbCr) { return cbCr.y; }

float4 UncompressRGTC_RGBA(const Texture2D inputTexture, const SamplerState inputSampler, const float2 texCoord)
{
    const float2 textureYA = inputTexture.SampleLevel(inputSampler, texCoord, TEXTURE_MIPMAP_YA).xy;
    const float2 textureCbCr = inputTexture.SampleLevel(inputSampler, texCoord, TEXTURE_MIPMAP_CbCr).xy;
    
    const float2 cbCr = (textureCbCr * FACTOR_709 - OFFSET_709);
    const float3 mixed = float3(AccessCr(cbCr), AccessY(textureYA), AccessCb(cbCr));
    
    return float4(
        dot(mixed, RED_COEF_709),
        dot(mixed, GRN_COEF_709),
        dot(mixed, BLU_COEF_709),
        AccessA(textureYA));
}

float UncompressRGTC_Alpha(const Texture2D inputTexture, const SamplerState inputSampler, const float2 texCoord)
{
    return AccessA(inputTexture.SampleLevel(inputSampler, texCoord, TEXTURE_MIPMAP_YA).xy);
}

float4 FormatAwareSampleTexture_RGBA(const Texture2D inputTexture, const SamplerState inputSampler, const float2 texCoord, int textureFormat)
{
    if (textureFormat == TextureFormat_RGTC2)
        return UncompressRGTC_RGBA(inputTexture, inputSampler, texCoord);
    else
        return inputTexture.Sample(inputSampler, texCoord);
}

float FormatAwareSampleTexture_Alpha(const Texture2D inputTexture, const SamplerState inputSampler, const float2 texCoord, int textureFormat)
{
    if (textureFormat == TextureFormat_RGTC2)
        return UncompressRGTC_Alpha(inputTexture, inputSampler, texCoord);
    else
        return inputTexture.Sample(inputSampler, texCoord).a;
}

float4 UncompressRGTC1_RGBA(const Texture2D inputTexture, const SamplerState inputSampler, const float2 texCoord)
{
    float3 texColor;
    texColor.y = inputTexture.SampleLevel(inputSampler, texCoord, 0).x;
    texColor.x = inputTexture.SampleLevel(inputSampler, texCoord, 1).x;
    texColor.z = inputTexture.SampleLevel(inputSampler, texCoord, 2).x;
    texColor.xz = mad(texColor.xz, FACTOR_709, -OFFSET_709);
    
    return float4(
        dot(texColor, RED_COEF_709),
        dot(texColor, GRN_COEF_709),
        dot(texColor, BLU_COEF_709),
        1.0);
}

#endif /* UNCOMPRESSRGTC_HLSL */
