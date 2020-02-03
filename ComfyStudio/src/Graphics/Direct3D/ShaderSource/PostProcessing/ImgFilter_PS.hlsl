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
    float4 outputColor;
    
    outputColor = DepthTexture.Sample(LinearTextureSampler, input.TexCoord);

    return outputColor;
}
