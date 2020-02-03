#define COMFY_PS
#include "../Include/Assembly/DebugInterface.hlsl"

struct VS_OUTPUT
{
    float4 Position     : SV_POSITION;
    float2 TexCoord     : TEXCOORD0;
};

SamplerState LinearTextureSampler
{
    Filter = MIN_MAG_MIP_LINEAR;
    AddressU = CLAMP;
    AddressV = CLAMP;
};

Texture2D<float> DepthTexture : register(t0);

float4 PS_main(VS_OUTPUT input) : SV_Target
{
#if 0
    float2 textureSize;
    DepthTexture.GetDimensions(textureSize.x, textureSize.y);

    float2 texelSize = 1.0 / textureSize;
    const float4 outputSum = 
        DepthTexture.Sample(LinearTextureSampler, input.TexCoord + float2(-texelSize.x, -texelSize.y)) + 
        DepthTexture.Sample(LinearTextureSampler, input.TexCoord + float2(-texelSize.x, +texelSize.y)) +
        DepthTexture.Sample(LinearTextureSampler, input.TexCoord + float2(+texelSize.x, -texelSize.y)) +
        DepthTexture.Sample(LinearTextureSampler, input.TexCoord + float2(+texelSize.x, +texelSize.y));
#else
    const float4 outputSum = 
        DepthTexture.Sample(LinearTextureSampler, input.TexCoord, int2(-1, -1)) + 
        DepthTexture.Sample(LinearTextureSampler, input.TexCoord, int2(-1, +1)) +
        DepthTexture.Sample(LinearTextureSampler, input.TexCoord, int2(+1, -1)) +
        DepthTexture.Sample(LinearTextureSampler, input.TexCoord, int2(+1, +1));
#endif
    
    return outputSum / 4.0;
}
