#ifndef MATERIAL_HLSL
#define MATERIAL_HLSL

struct Material
{
    matrix DiffuseTextureTransform;
    matrix AmbientTextureTransform;
    float4 FresnelCoefficient;
    float4 Diffuse;
    float4 Ambient;
    float4 Specular;
    float4 Emission;
    float1 Shininess;
    float1 Intensity;
    float1 BumpDepth;
};

#endif /* MATERIAL_HLSL */
