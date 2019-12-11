#include "SceneData.hlsl"
#include "Material.hlsl"
#include "ShaderFlags.hlsl"
#include "TextureFormats.hlsl"

// TODO: program.env[24].x
static const uint AmbientTextureType = 1;

// TODO: program.env[3]
static const float3 BlendColor = float3(1.0, 1.0, 1.0);

cbuffer SceneConstantData : register(b0)
{
    SceneData CB_Scene;
    float1 SceneCB_Padding[8];
};

struct TextureFormats
{
    TextureFormat Diffuse;
    TextureFormat Ambient;
    TextureFormat Normal;
    TextureFormat Specular;
    TextureFormat ToonCurve;
    TextureFormat Reflection;
    TextureFormat Tangent;
    TextureFormat Reserved;
};

cbuffer ObjectConstantData : register(b1)
{
    matrix CB_Model;
    Material CB_Material;
    uint CB_ShaderFlags;
    TextureFormats CB_TextureFormats;
};
