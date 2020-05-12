#ifndef SCENEDATA_HLSL
#define SCENEDATA_HLSL

struct IBLData
{
    matrix IrradianceRGB[3];
    float4 LightColors[4];
};

struct ParallelLight
{
    float4 Ambient;
    float4 Diffuse;
    float4 Specular;
    float4 Direction;
};

struct LinearFog
{
    // NOTE: { x = Density, y = Start, z = End, w = 1.0 / (End - Start) }
    float4 Parameters;
    float4 Color;
};

struct RenderTime
{
    // NOTE: Time scales { x = 0.5, y = 1.0, z = 2.0, w = 4.0 }
    float4 Time;
    float4 TimeSin;
    float4 TimeCos;
};

struct SceneData
{
    IBLData IBL;

    matrix View;
    matrix ViewProjection;
    matrix LightSpace;
    float4 EyePosition;
    
    ParallelLight CharaLight;
    ParallelLight StageLight;
    
    float2 TexelRenderResolution;
    float2 RenderResolution;

    RenderTime RenderTime;
    
    LinearFog DepthFog;
    
    float4 ShadowAmbient;
    float4 OneMinusShadowAmbient;
    float1 ShadowExponent;
    
    float1 SubsurfaceScatteringParameter;
    
    uint DebugFlags;
    uint Padding;
    float4 DebugValue;
};

#endif /* SCENEDATA_HLSL */
