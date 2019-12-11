#ifndef SHADERFLAGS_HLSL
#define SHADERFLAGS_HLSL

static const uint ShaderFlags_VertexColor = 1 << 0;
static const uint ShaderFlags_DiffuseTexture = 1 << 1;
static const uint ShaderFlags_AmbientTexture = 1 << 2;
static const uint ShaderFlags_NormalTexture = 1 << 3;
static const uint ShaderFlags_SpecularTexture = 1 << 4;
static const uint ShaderFlags_AlphaTest = 1 << 5;
static const uint ShaderFlags_CubeMapReflection = 1 << 6;

#define FLAGS_VERTEX_COLOR (CB_ShaderFlags & ShaderFlags_VertexColor)
#define FLAGS_DIFFUSE_TEX2D (CB_ShaderFlags & ShaderFlags_DiffuseTexture)
#define FLAGS_AMBIENT_TEX2D (CB_ShaderFlags & ShaderFlags_AmbientTexture)
#define FLAGS_NORMAL_TEX2D (CB_ShaderFlags & ShaderFlags_NormalTexture)
#define FLAGS_SPECULAR_TEX2D (CB_ShaderFlags & ShaderFlags_SpecularTexture)
#define FLAGS_ALPHA_TEST (CB_ShaderFlags & ShaderFlags_AlphaTest)
#define FLAGS_REFLECTION_CUBE (CB_ShaderFlags & ShaderFlags_CubeMapReflection)

#endif /* SHADERFLAGS_HLSL */
