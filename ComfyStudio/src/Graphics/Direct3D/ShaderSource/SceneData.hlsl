#ifndef SCENEDATA_HLSL
#define SCENEDATA_HLSL

struct ParallelLight
{
    float4 Ambient;
    float4 Diffuse;
    float4 Specular;
    float4 Position;
    float4 Direction;
};

struct SceneData
{
    matrix ViewProjection;
    float4 EyePosition;
    ParallelLight StageLight;
    float4 LightDiffuse;
};

#endif /* SCENEDATA_HLSL */
