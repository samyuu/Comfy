SamplerState DiffuseSampler     : register(s0);
SamplerState AmbientSampler     : register(s1);
SamplerState NormalSampler      : register(s2);

SamplerState ReflectionSampler  : register(s5);

Texture2D DiffuseTexture        : register(t0);
Texture2D AmbientTexture        : register(t1);
Texture2D NormalTexture         : register(t2);

TextureCube ReflectionCubeMap   : register(t5);

TextureCube CharacterLightMap   : register(t9);  // ---
TextureCube SunLightMap         : register(t10);
TextureCube ReflectLightMap     : register(t11);
TextureCube ShadowLightMap      : register(t12); // ---
TextureCube CharColorLightMap   : register(t13); // ---
