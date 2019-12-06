SamplerState DiffuseSampler     : register(s0);
SamplerState AmbientSampler     : register(s1);
SamplerState NormalSampler      : register(s2);
SamplerState SpecularSampler    : register(s3);

SamplerState ReflectionSampler  : register(s5);

SamplerState LightMapSampler    : register(s15)
{
    Filter = MIN_MAG_MIP_LINEAR;
    AddressU = CLAMP;
    AddressV = CLAMP;
};

Texture2D DiffuseTexture        : register(t0);
Texture2D AmbientTexture        : register(t1);
Texture2D NormalTexture         : register(t2);
Texture2D SpecularTexture       : register(t3);

TextureCube ReflectionCubeMap   : register(t5);

TextureCube CharacterLightMap   : register(t9);  // Unused
TextureCube SunLightMap         : register(t10);
TextureCube ReflectLightMap     : register(t11);
TextureCube ShadowLightMap      : register(t12); // Unused
TextureCube CharColorLightMap   : register(t13); // Unused
