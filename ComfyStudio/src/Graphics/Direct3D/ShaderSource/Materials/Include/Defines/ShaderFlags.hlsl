#ifndef SHADERFLAGS_HLSL
#define SHADERFLAGS_HLSL

static const uint ShaderFlags_VertexColor = 1 << 0;
static const uint ShaderFlags_DiffuseTexture = 1 << 1;
static const uint ShaderFlags_AmbientTexture = 1 << 2;
static const uint ShaderFlags_NormalTexture = 1 << 3;
static const uint ShaderFlags_SpecularTexture = 1 << 4;
static const uint ShaderFlags_AlphaTest = 1 << 5;
static const uint ShaderFlags_CubeMapReflection = 1 << 6;
static const uint ShaderFlags_LinearFog = 1 << 7;
static const uint ShaderFlags_Morph = 1 << 8;
static const uint ShaderFlags_Shadow = 1 << 9;

#define FLAGS_VERTEX_COLOR (CB_Object.ShaderFlags & ShaderFlags_VertexColor)
#define FLAGS_DIFFUSE_TEX2D (CB_Object.ShaderFlags & ShaderFlags_DiffuseTexture)
#define FLAGS_AMBIENT_TEX2D (CB_Object.ShaderFlags & ShaderFlags_AmbientTexture)
#define FLAGS_NORMAL_TEX2D (CB_Object.ShaderFlags & ShaderFlags_NormalTexture)
#define FLAGS_SPECULAR_TEX2D (CB_Object.ShaderFlags & ShaderFlags_SpecularTexture)
#define FLAGS_ALPHA_TEST (CB_Object.ShaderFlags & ShaderFlags_AlphaTest)
#define FLAGS_REFLECTION_CUBE (CB_Object.ShaderFlags & ShaderFlags_CubeMapReflection)
#define FLAGS_LINEAR_FOG (CB_Object.ShaderFlags & ShaderFlags_LinearFog)
#define FLAGS_MORPH (CB_Object.ShaderFlags & ShaderFlags_Morph)
#define FLAGS_SHADOW (CB_Object.ShaderFlags & ShaderFlags_Shadow)

#define DEBUG_FLAGS (CB_Scene.DebugFlags)

#endif /* SHADERFLAGS_HLSL */
