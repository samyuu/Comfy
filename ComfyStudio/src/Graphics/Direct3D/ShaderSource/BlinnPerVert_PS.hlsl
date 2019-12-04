#include "Include/InputLayouts.hlsl"
#include "Include/ConstantInputs.hlsl"
#include "Include/Common.hlsl"

SamplerState DiffuseSampler : register(s0);
SamplerState AmbientSampler : register(s1);
SamplerState ReflectionSampler : register(s5);

Texture2D DiffuseTexture : register(t0);
Texture2D AmbientTexture : register(t1);
TextureCube ReflectionCubeMap : register(t5);

float4 PS_main(VS_OUTPUT input) : SV_Target
{
    float4 outputColor = input.Color;
    
    if (CB_ShaderFlags & ShaderFlags_DiffuseTexture)
        outputColor = DiffuseTexture.Sample(DiffuseSampler, input.TexCoord);
    
    if (CB_ShaderFlags & ShaderFlags_AmbientTexture)
        outputColor *= SampleAmbientTexture(AmbientTexture, AmbientSampler, input.TexCoordAmbient, AmbientTextureType);
    
    // TODO: Implement the rest
    if (CB_ShaderFlags & ShaderFlags_CubeMapReflection)
    {
        float4 specular = ReflectionTexture.Sample(ReflectionSampler, input.Reflection);
        specular.a = CB_Material.Reflectivity * CB_Scene.StageLight.Specular.a;
        specular.rgb *= CB_Scene.StageLight.Specular.rgb * specular.a;
        outputColor = mad(input.Color, outputColor, specular);
    }
    
    return outputColor;
}
