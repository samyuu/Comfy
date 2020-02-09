#ifndef MATERIALDATA_HLSL
#define MATERIALDATA_HLSL

struct MaterialData
{
    matrix DiffuseTextureTransform;
    matrix AmbientTextureTransform;
    float4 FresnelCoefficient;
    float4 Diffuse;
    float4 Ambient;
    float4 Specular;
    float4 Emission;
    float2 Shininess;
    float1 Intensity;
    float1 BumpDepth;
};

#endif /* MATERIALDATA_HLSL */
