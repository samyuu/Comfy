#ifndef MATERIAL_HLSL
#define MATERIAL_HLSL

struct Material
{
    matrix DiffuseTextureTransform;
    matrix AmbientTextureTransform;
    float3 Diffuse;
    float1 Transparency;
    float4 Ambient;
    float3 Specular;
    float1 Reflectivity;
    float4 Emission;
    float1 Shininess;
    float1 Intensity;
    float1 BumpDepth;
};

#endif /* MATERIAL_HLSL */
