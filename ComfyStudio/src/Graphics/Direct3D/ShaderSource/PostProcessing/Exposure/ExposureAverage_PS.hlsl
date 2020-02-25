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

float4 PS_main(VS_OUTPUT input) : SV_Target
{
    float4 colorSum = 0.0;
    colorSum += ScreenTexture.Sample(LinearTextureSampler, float2(0.03125, 0.25));
    colorSum += ScreenTexture.Sample(LinearTextureSampler, float2(0.09375, 0.25));
    colorSum += ScreenTexture.Sample(LinearTextureSampler, float2(0.15625, 0.25));
    colorSum += ScreenTexture.Sample(LinearTextureSampler, float2(0.21875, 0.25));
    colorSum += ScreenTexture.Sample(LinearTextureSampler, float2(0.28125, 0.25));
    colorSum += ScreenTexture.Sample(LinearTextureSampler, float2(0.34375, 0.25));
    colorSum += ScreenTexture.Sample(LinearTextureSampler, float2(0.40625, 0.25));
    colorSum += ScreenTexture.Sample(LinearTextureSampler, float2(0.46875, 0.25));
    colorSum += ScreenTexture.Sample(LinearTextureSampler, float2(0.53125, 0.25));
    colorSum += ScreenTexture.Sample(LinearTextureSampler, float2(0.59375, 0.25));
    colorSum += ScreenTexture.Sample(LinearTextureSampler, float2(0.65625, 0.25));
    colorSum += ScreenTexture.Sample(LinearTextureSampler, float2(0.71875, 0.25));
    colorSum += ScreenTexture.Sample(LinearTextureSampler, float2(0.78125, 0.25));
    colorSum += ScreenTexture.Sample(LinearTextureSampler, float2(0.84375, 0.25));
    colorSum += ScreenTexture.Sample(LinearTextureSampler, float2(0.90625, 0.25));
    colorSum += ScreenTexture.Sample(LinearTextureSampler, float2(0.96875, 0.25));
    return colorSum / 16.0;
}
