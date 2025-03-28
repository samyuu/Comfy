#ifndef OBJECTDATA_HLSL
#define OBJECTDATA_HLSL

#include "MaterialData.hlsl"
#include "../ShaderFlags.hlsl"

struct ObjectData
{
    matrix Model;
    matrix ModelInverse;
    matrix ModelView;
    matrix ModelViewProjection;
    
    MaterialData Material;
    float4 MorphWeight;
    
    uint ShaderFlags;
    uint DiffuseRGTC1;
    uint DiffuseScreenTexture;
    uint AmbientTextureType;
};

#endif /* OBJECTDATA_HLSL */
