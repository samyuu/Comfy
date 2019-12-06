#ifndef SHADERFLAGS_HLSL
#define SHADERFLAGS_HLSL

static const uint ShaderFlags_VertexColor = 1 << 0;
static const uint ShaderFlags_DiffuseTexture = 1 << 1;
static const uint ShaderFlags_AmbientTexture = 1 << 2;
static const uint ShaderFlags_NormalTexture = 1 << 3;
static const uint ShaderFlags_SpecularTexture = 1 << 4;
static const uint ShaderFlags_AlphaTest = 1 << 5;
static const uint ShaderFlags_CubeMapReflection = 1 << 6;

#endif /* SHADERFLAGS_HLSL */
