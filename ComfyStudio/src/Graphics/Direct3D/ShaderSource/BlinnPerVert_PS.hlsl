#include "Include/InputLayouts.hlsl"
#include "Include/ConstantInputs.hlsl"
#include "Include/Common.hlsl"
#include "Include/TextureInputs.hlsl"

float4 PS_main(VS_OUTPUT input) : SV_Target
{
    // NOTE: state.lightprod[1] = CB_Material * CB_Scene.StageLight
    
    // NOTE: Diffuse texture else Diffuse material color
    const float4 diffuseTexColor = (CB_ShaderFlags & ShaderFlags_DiffuseTexture) ? 
        DiffuseTexture.Sample(DiffuseSampler, input.TexCoord) : float4(CB_Material.Diffuse.rgb, 1.0);
    
    // NOTE: Ambient texture else white
    const float4 ambientTexColor = (CB_ShaderFlags & ShaderFlags_AmbientTexture) ?
        SampleAmbientTexture(AmbientTexture, AmbientSampler, input.TexCoordAmbient, AmbientTextureType) : float4(1.0, 1.0, 1.0, 1.0);
    
    float3 spec;
    
    // TODO: Cube map + specular texture (?)
    if (CB_ShaderFlags & ShaderFlags_CubeMapReflection)
    {
        const float4 reflectionTexColor = ReflectionCubeMap.Sample(ReflectionSampler, input.Reflection.xyz);
        
        spec = reflectionTexColor.rgb * CB_Material.Reflectivity * CB_Scene.StageLight.Specular.w;
    }
    else if (CB_ShaderFlags & ShaderFlags_SpecularTexture)
    {
        const float4 specTexColor = SpecularTexture.Sample(SpecularSampler, input.TexCoord);
        
        spec = mad(specTexColor.rgb, input.Color.rgb, input.ColorSecondary.rgb * specTexColor.rgb);
        
        return float4(spec, 1.0);
    }
    else
    {
        const float4 sunLightMapColor = SunLightMap.Sample(LightMapSampler, input.Reflection.xyz);
        const float4 reflectLightMapColor = ReflectLightMap.Sample(LightMapSampler, input.Reflection.xyz);
    
        spec = lerp(sunLightMapColor.rgb, reflectLightMapColor.rgb, input.Reflection.w);
    }
    
    spec *= CB_Material.Specular * CB_Scene.StageLight.Specular.rgb;
    
    float4 outputColor;
    outputColor.rgb = mad(input.Color.rgb, (diffuseTexColor.rgb * ambientTexColor.rgb), spec.rgb);
    outputColor.a = diffuseTexColor.a;
    
    if (CB_ShaderFlags & ShaderFlags_AlphaTest)
        ClipAlphaThreshold(outputColor.a);
        
    return outputColor;
}
