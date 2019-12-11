#include "Include/InputLayouts.hlsl"
#include "Include/ConstantInputs.hlsl"
#include "Include/Common.hlsl"
#include "Include/TextureInputs.hlsl"

float4 PS_main(VS_OUTPUT input) : SV_Target
{
    const float4 diffuseTexColor = (CB_ShaderFlags & ShaderFlags_DiffuseTexture) ?
        DiffuseTexture.Sample(DiffuseSampler, input.TexCoord) : float4(CB_Material.Diffuse.rgb, 1.0);
    
    const float4 ambientTexColor = (CB_ShaderFlags & ShaderFlags_AmbientTexture) ?
        SampleAmbientTexture(AmbientTexture, AmbientSampler, input.TexCoordAmbient, AmbientTextureType) : float4(1.0, 1.0, 1.0, 1.0);
    
    const float4 texColor = (diffuseTexColor * ambientTexColor);
    const float4 normalTexColor = NormalTexture.Sample(NormalSampler, input.TexCoord);
    
    float3 normal = float3(mad(normalTexColor.xy, 2.0, -1.0), 0.8);
    normal *= mad(input.Color.w, 2.0, -1.0);
    
    float3x3 tangentToWorldSpace = float3x3(input.Tangent.xyz, input.Binormal.xyz, input.Normal.xyz);
    float3 worldSpaceNormal = mul(normal, tangentToWorldSpace);
    
    float3 diff = CharacterLightMap.Sample(LightMapSampler, worldSpaceNormal).rgb;
    diff = mad(diff, CB_Scene.StageLight.Diffuse.rgb, input.Color.rgb);
    
    const float3 reflection = reflect(-input.EyeDirection.xyz, worldSpaceNormal);
    float3 spec = float3(1.0, 1.0, 1.0);
    
    if (CB_ShaderFlags & ShaderFlags_CubeMapReflection)
    {
        spec = ReflectionCubeMap.Sample(ReflectionSampler, reflection).rgb;
    }
    else
    {
        const float4 sunLightMapColor = SunLightMap.Sample(LightMapSampler, reflection);
        const float4 reflectLightMapColor = ReflectLightMap.Sample(LightMapSampler, reflection);
    
        spec = lerp(sunLightMapColor.rgb, reflectLightMapColor.rgb, 1.0 - CB_Material.Shininess);
    }
    
    if (CB_ShaderFlags & ShaderFlags_SpecularTexture)
        spec *= SpecularTexture.Sample(SpecularSampler, input.TexCoord).rgb;
        
    spec *= CB_Material.Specular.a * CB_Scene.StageLight.Specular.a;
    spec *= CB_Material.Specular.rgb * CB_Scene.StageLight.Specular.rgb;
    
    float4 outputColor;
    outputColor.rgb = mad(diff, texColor.rgb, spec);
    outputColor.a = texColor.a; // * input.Color.a
    
    if (CB_ShaderFlags & ShaderFlags_AlphaTest)
        ClipAlphaThreshold(outputColor.a);
        
    return outputColor;
}
