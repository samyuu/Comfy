#include "Include/InputLayouts.hlsl"
#include "Include/ConstantInputs.hlsl"
#include "Include/Common.hlsl"

VS_OUTPUT VS_main(VS_INPUT input)
{
    VS_OUTPUT output = (VS_OUTPUT)0;
    
    output.Position = mul(input.Position, CB_Model);
    output.Position = mul(output.Position, CB_Scene.ViewProjection);
    
    const float4 normal = float4(input.Normal, 1.0);
    output.Normal = mul(normal, CB_Model).xyz;

    if (CB_ShaderFlags & ShaderFlags_DiffuseTexture)
        output.TexCoord = TransformTextureCoordinates(input.TexCoord, CB_Material.DiffuseTextureTransform);
    
    if (CB_ShaderFlags & ShaderFlags_AmbientTexture)
        output.TexCoordAmbient = TransformTextureCoordinates(input.TexCoordAmbient, CB_Material.AmbientTextureTransform);
    
    float3 irradiance = float3(dot(mul(normal, CB_Scene.IrradianceRed), normal), dot(mul(normal, CB_Scene.IrradianceGreen), normal), dot(mul(normal, CB_Scene.IrradianceBlue), normal));
    float3 diffuse = saturate(dot(normal.xyz, CB_Scene.StageLight.Direction.xyz));
    
    output.Color = float4(mad(diffuse, CB_Scene.LightColor.rgb, irradiance * CB_Scene.StageLight.Diffuse.rgb), 1.0);
    
    if (CB_ShaderFlags & ShaderFlags_VertexColor)
        output.Color *= input.Color;
    
    if (CB_ShaderFlags & ShaderFlags_CubeMapReflection)
    {
        float3 eyeDirection = normalize(input.Position.xyz - CB_Scene.EyePosition.xyz);
        output.Reflection = reflect(eyeDirection, input.Normal);
    }
    
    return output;
}
