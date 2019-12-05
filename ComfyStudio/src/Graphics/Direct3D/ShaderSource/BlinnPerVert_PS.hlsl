#include "Include/InputLayouts.hlsl"
#include "Include/ConstantInputs.hlsl"
#include "Include/Common.hlsl"
#include "Include/TextureInputs.hlsl"

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
        float4 spec = ReflectionCubeMap.Sample(ReflectionSampler, input.Reflection.xyz);
        spec.w = CB_Material.Reflectivity, CB_Scene.StageLight.Specular.w;
        spec.rgb *= spec.w;
        spec.rgb *= lerp(CB_Material.Diffuse.rgb, CB_Scene.StageLight.Specular.rgb, 0.5);
		
        outputColor.rgb *= mad(input.Color.rgb, CB_Material.Diffuse.rgb, spec.rgb);
    }
    
    return outputColor;
}
