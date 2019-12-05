#ifndef COMMON_HLSL
#define COMMON_HLSL
#include "SceneData.hlsl"

static const float AlphaTestThreshold = 0.5;

void ClipAlphaThreshold(const float alpha)
{
    clip(alpha < AlphaTestThreshold ? -1.0 : 1.0);
}

float2 TransformTextureCoordinates(const float2 texCoord, const matrix transform)
{
    float2 outputTexCoord;
    outputTexCoord.x = dot(transform[0], float4(texCoord, 0.0, 0.0));
    outputTexCoord.y = dot(transform[1], float4(texCoord, 0.0, 0.0));
    return outputTexCoord;
}

float4 SampleAmbientTexture(Texture2D inputTexture, SamplerState inputSampler, const float2 texCoord, const uint type)
{
    float4 ambientTextureColor = inputTexture.Sample(inputSampler, texCoord);
    
    if (type == 3)
        ambientTextureColor = 1.0 / (ambientTextureColor + 0.004);
    
    return ambientTextureColor;
}

float3 GetIrradience(const SceneData scene, const float4 normal)
{
    return float3(dot(mul(normal, scene.IrradianceRed), normal), dot(mul(normal, scene.IrradianceGreen), normal), dot(mul(normal, scene.IrradianceBlue), normal));
}

float3 GetDiffuseLight(const ParallelLight light, const float3 normal)
{
    return saturate(dot(normal, light.Direction.xyz));
}

float2 ScreenTexCoordFromVertexID(const uint vertexID)
{
    return float2(vertexID % 2, vertexID % 4 / 2);
}

float4 ScreenPositionFromTexCoord(const float2 texCoord)
{
    return float4((texCoord.x - 0.5) * 2.0, -(texCoord.y - 0.5) * 2.0, 0.0, 1.0);
}

#endif /* COMMON_HLSL */
