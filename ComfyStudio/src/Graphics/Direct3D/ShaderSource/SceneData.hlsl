#ifndef SCENEDATA_HLSL
#define SCENEDATA_HLSL

struct ParallelLight
{
    float4 Ambient;
    float4 Diffuse;
    float4 Specular;
    float4 Direction;
};

struct SceneData
{
    matrix ViewProjection;
    float4 EyePosition;
    ParallelLight StageLight;
    float4 LightColor;
};

#endif /* SCENEDATA_HLSL */
