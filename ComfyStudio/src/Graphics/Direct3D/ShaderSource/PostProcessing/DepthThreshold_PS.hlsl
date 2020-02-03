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
    static const float depthThreshold = 1.0;
    
    float depth = DepthTexture.Sample(LinearTextureSampler, input.TexCoord);
#if 0
	return (depth < depthThreshold) ? 0.0 : 1.0;
#else
    return float(depth >= depthThreshold);
#endif

}
