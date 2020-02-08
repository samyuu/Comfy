#ifndef TEXTUREINPUTS_HLSL
#define TEXTUREINPUTS_HLSL

SamplerState DiffuseSampler         : register(s0);
SamplerState AmbientSampler         : register(s1);
SamplerState NormalSampler          : register(s2);
SamplerState SpecularSampler        : register(s3);
SamplerState LucencySampler         : register(s6);
SamplerState ReflectionSampler      : register(s5);

SamplerState LightMapSampler        : register(s14)
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

// TODO: Use a single "LinearSampler" for all non material textures

Texture2D<float4> DiffuseTexture                : register(t0);
Texture2D<float4> AmbientTexture                : register(t1);
Texture2D<float4> NormalTexture                 : register(t2);
Texture2D<float4> SpecularTexture               : register(t3);
Texture2D<float4> LucencyTexture                : register(t6);

TextureCube<float4> ReflectionCubeMap           : register(t5);
TextureCube<float4> CharacterLightMap           : register(t9);
TextureCube<float4> SunLightMap                 : register(t10);
TextureCube<float4> ReflectLightMap             : register(t11);
TextureCube<float4> ShadowLightMap              : register(t12);
TextureCube<float4> CharColorLightMap           : register(t13);

Texture2D<float4> ScreenReflectionTexture       : register(t15);
Texture2D<float4> SubsurfaceScatteringTexture   : register(t16);
Texture2D<float1> StageShadowMap                : register(t19);
Texture2D<float1> ESMFull                       : register(t20);
Texture2D<float1> ESMGauss                      : register(t21);

#endif /* TEXTUREINPUTS_HLSL */
