#ifndef OBJECTDATA_HLSL
#define OBJECTDATA_HLSL

#include "MaterialData.hlsl"
#include "../Defines/ShaderFlags.hlsl"
#include "../Defines/TextureFormats.hlsl"

struct TextureFormats
{
    int Diffuse;
    int Ambient;
    int Normal;
    int Specular;
    int ToonCurve;
    int Reflection;
    int Tangent;
    uint AmbientType;
};

struct ObjectData
{
    matrix Model;
    matrix ModelView;
    matrix ModelViewProjection;
    MaterialData Material;
    uint ShaderFlags;
    TextureFormats TextureFormats;
    float4 MorphWeight;
};

#endif /* OBJECTDATA_HLSL */
