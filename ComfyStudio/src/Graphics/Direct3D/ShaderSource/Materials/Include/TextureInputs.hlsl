#ifndef TEXTUREINPUTS_HLSL
#define TEXTUREINPUTS_HLSL

SamplerState DiffuseSampler         : register(s0);
SamplerState AmbientSampler         : register(s1);
SamplerState NormalSampler          : register(s2);
SamplerState SpecularSampler        : register(s3);
SamplerState TransparencySampler    : register(s4);
SamplerState EnvironmentSampler     : register(s5);
SamplerState TranslucencySampler    : register(s6);

SamplerState IBL_LightMapSampler    : register(s13)
{
    Filter = MIN_MAG_MIP_LINEAR;
    AddressU = CLAMP;
    AddressV = CLAMP;
};

SamplerState LinearSampler          : register(s14)
{
    Filter = MIN_MAG_MIP_LINEAR;
    AddressU = CLAMP;
    AddressV = CLAMP;
};

SamplerState ScreenReflectionSampler : register(s15)
{
    Filter = MIN_MAG_MIP_LINEAR;
    AddressU = CLAMP;
    AddressV = CLAMP;
};

Texture2D<float4> DiffuseTexture                : register(t0);
Texture2D<float4> AmbientTexture                : register(t1);
Texture2D<float4> NormalTexture                 : register(t2);
Texture2D<float4> SpecularTexture               : register(t3);
Texture2D<float4> TransparencyTexture           : register(t4);
TextureCube<float4> EnvironmentTexture          : register(t5);
Texture2D<float4> TranslucencyTexture           : register(t6);

TextureCube<float4> IBL_LightMaps[3]            : register(t9);

Texture2D<float4> ScreenReflectionTexture       : register(t15);
Texture2D<float4> SubsurfaceScatteringTexture   : register(t16);
Texture2D<float1> ShadowMap                     : register(t19);
Texture2D<float1> ESMFull                       : register(t20);
Texture2D<float1> ESMGauss                      : register(t21);

#endif /* TEXTUREINPUTS_HLSL */
