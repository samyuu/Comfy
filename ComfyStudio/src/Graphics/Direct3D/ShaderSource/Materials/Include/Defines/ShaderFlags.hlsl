#ifndef SHADERFLAGS_HLSL
#define SHADERFLAGS_HLSL

static const uint ShaderFlags_VertexColor = 1 << 0;
static const uint ShaderFlags_DiffuseTexture = 1 << 1;
static const uint ShaderFlags_AmbientTexture = 1 << 2;
static const uint ShaderFlags_NormalTexture = 1 << 3;
static const uint ShaderFlags_SpecularTexture = 1 << 4;
static const uint ShaderFlags_TransparencyTexture = 1 << 5;
static const uint ShaderFlags_EnvironmentTexture = 1 << 6;
static const uint ShaderFlags_TranslucencyTexture = 1 << 7;
static const uint ShaderFlags_PunchThrough = 1 << 8;
static const uint ShaderFlags_LinearFog = 1 << 9;
static const uint ShaderFlags_Morph = 1 << 10;
static const uint ShaderFlags_MorphColor = 1 << 11;
static const uint ShaderFlags_Shadow = 1 << 12;
static const uint ShaderFlags_ShadowSecondary = 1 << 13;
static const uint ShaderFlags_SelfShadow = 1 << 14;

#define FLAGS_VERTEX_COLOR (CB_Object.ShaderFlags & ShaderFlags_VertexColor)
#define FLAGS_DIFFUSE_TEX2D (CB_Object.ShaderFlags & ShaderFlags_DiffuseTexture)
#define FLAGS_AMBIENT_TEX2D (CB_Object.ShaderFlags & ShaderFlags_AmbientTexture)
#define FLAGS_NORMAL_TEX2D (CB_Object.ShaderFlags & ShaderFlags_NormalTexture)
#define FLAGS_SPECULAR_TEX2D (CB_Object.ShaderFlags & ShaderFlags_SpecularTexture)
#define FLAGS_TRANSPARENCY_TEX2D (CB_Object.ShaderFlags & ShaderFlags_TransparencyTexture)
#define FLAGS_ENVIRONMENT_CUBE (CB_Object.ShaderFlags & ShaderFlags_EnvironmentTexture)
#define FLAGS_TRANSLUCENCY_TEX2D (CB_Object.ShaderFlags & ShaderFlags_TranslucencyTexture)
#define FLAGS_PUNCH_THROUGH (CB_Object.ShaderFlags & ShaderFlags_PunchThrough)
#define FLAGS_LINEAR_FOG (CB_Object.ShaderFlags & ShaderFlags_LinearFog)
#define FLAGS_MORPH (CB_Object.ShaderFlags & ShaderFlags_Morph)
#define FLAGS_MORPH_COLOR (CB_Object.ShaderFlags & ShaderFlags_MorphColor)
#define FLAGS_SHADOW (CB_Object.ShaderFlags & ShaderFlags_Shadow)
#define FLAGS_SHADOW_SECONDARY (CB_Object.ShaderFlags & ShaderFlags_ShadowSecondary)
#define FLAGS_SELF_SHADOW (CB_Object.ShaderFlags & ShaderFlags_SelfShadow)

#define FLAGS_DEBUG_0 (CB_Scene.DebugFlags & (1 << 0))
#define FLAGS_DEBUG_1 (CB_Scene.DebugFlags & (1 << 1))
#define FLAGS_DEBUG_2 (CB_Scene.DebugFlags & (1 << 2))
#define FLAGS_DEBUG_3 (CB_Scene.DebugFlags & (1 << 3))

#endif /* SHADERFLAGS_HLSL */
