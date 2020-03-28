struct VS_OUTPUT
{
    float4 Position : SV_POSITION;
    float2 TexCoord : TEXCOORD;
};

SamplerState LinearTextureSampler
{
    Filter = MIN_MAG_MIP_LINEAR;
    AddressU = CLAMP;
    AddressV = CLAMP;
};

Texture2D<float4> ScreenTexture : register(t0);

static const float2 TextureSize = float2(32.0 / 1.0, 18.0 / 1.0);

float4 PS_main(VS_OUTPUT input) : SV_Target
{
    float4 colorSum = 0.0;
    colorSum += ScreenTexture.Sample(LinearTextureSampler, mad(TextureSize, float2(-1.5, -0.6), input.TexCoord));
    colorSum += ScreenTexture.Sample(LinearTextureSampler, mad(TextureSize, float2(-0.5, -0.6), input.TexCoord));
    colorSum += ScreenTexture.Sample(LinearTextureSampler, mad(TextureSize, float2(+0.5, -0.6), input.TexCoord));
    colorSum += ScreenTexture.Sample(LinearTextureSampler, mad(TextureSize, float2(+1.5, -0.6), input.TexCoord));
    colorSum += ScreenTexture.Sample(LinearTextureSampler, mad(TextureSize, float2(-1.5, +0.6), input.TexCoord));
    colorSum += ScreenTexture.Sample(LinearTextureSampler, mad(TextureSize, float2(-0.5, +0.6), input.TexCoord));
    colorSum += ScreenTexture.Sample(LinearTextureSampler, mad(TextureSize, float2(+0.5, +0.6), input.TexCoord));
    colorSum += ScreenTexture.Sample(LinearTextureSampler, mad(TextureSize, float2(+1.5, +0.6), input.TexCoord));
    return colorSum / 8.0;
}
