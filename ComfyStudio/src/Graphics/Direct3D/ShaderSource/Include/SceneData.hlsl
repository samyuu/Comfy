#ifndef SCENEDATA_HLSL
#define SCENEDATA_HLSL

struct ParallelLight
{
    float4 Ambient;
    float4 Diffuse;
    float4 Specular;
    float4 Direction;
};

struct LinearFog
{
    // NOTE: x: Density, y: Start, z: End, w: 1.0 / (End - Start)
    float4 Parameters;
    float4 Color;
};

struct SceneData
{
    matrix IrradianceRed;
    matrix IrradianceGreen;
    matrix IrradianceBlue;
    
    matrix View;
    matrix ViewProjection;
    float4 EyePosition;
    
    ParallelLight CharacterLight;
    ParallelLight StageLight;
    
    float4 StageLightColor;
    float4 CharacterLightColor;
    
    float2 TexelRenderResolution;
    float2 RenderResolution;
    
    LinearFog DepthFog;
    
    float4 SubsurfaceScatteringParameter;
};

#endif /* SCENEDATA_HLSL */
