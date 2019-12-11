#include "Include/InputLayouts.hlsl"
#include "Include/ConstantInputs.hlsl"
#include "Include/Common.hlsl"

VS_OUTPUT VS_main(VS_INPUT input)
{
    VS_OUTPUT output = (VS_OUTPUT)0;
    
    output.Position = mul(input.Position, CB_Model);
    output.Position = mul(output.Position, CB_Scene.ViewProjection);
    
    const float4 normal = float4(input.Normal, 1.0);
    output.Normal = mul(normal, CB_Model);

    if (CB_ShaderFlags & ShaderFlags_DiffuseTexture)
        output.TexCoord = TransformTextureCoordinates(input.TexCoord, CB_Material.DiffuseTextureTransform);
    
    if (CB_ShaderFlags & ShaderFlags_AmbientTexture)
        output.TexCoordAmbient = TransformTextureCoordinates(input.TexCoordAmbient, CB_Material.AmbientTextureTransform);
    
    float3 irradiance = GetIrradience(CB_Scene, normal);
    float3 diffuse = GetDiffuseLight(CB_Scene.StageLight, normal.xyz);
    
    output.Color = float4(mad(diffuse, CB_Scene.LightColor.rgb, irradiance * CB_Scene.StageLight.Diffuse.rgb), 1.0);
    
    if (CB_ShaderFlags & ShaderFlags_VertexColor)
        output.Color *= input.Color;
    
    return output;
}
