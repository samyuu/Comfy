#ifndef MATERIAL_HLSL
#define MATERIAL_HLSL

struct Material
{
    float3 DiffuseColor;
    float1 Transparency;
    float4 AmbientColor;
    float3 SpecularColor;
    float1 Reflectivity;
    float4 EmissionColor;
    float1 Shininess;
    float1 Intensity;
    float1 BumpDepth;
    float1 AlphaTestThreshold;
    matrix TextureTransform;
};

#endif /* MATERIAL_HLSL */
