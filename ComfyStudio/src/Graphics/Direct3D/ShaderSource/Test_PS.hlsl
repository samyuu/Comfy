#include "InputLayouts.hlsl"
#include "ConstantInputs.hlsl"

SamplerState DiffuseSampler     : register(s0);
SamplerState AmbientSampler     : register(s1);
SamplerState ReflectionSampler  : register(s2);

Texture2D DiffuseTexture        : register(t0);
Texture2D AmbientTexture        : register(t1);
TextureCube ReflectionTexture   : register(t2);

float4 PS_main(VS_OUTPUT input) : SV_Target
{
    return float4(input.Normal, 1.0);
}
