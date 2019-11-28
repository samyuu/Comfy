#include "InputLayouts.hlsl"
#include "ConstantInputs.hlsl"
#include "Common.hlsl"

VS_OUTPUT VS_main(VS_INPUT input)
{
    VS_OUTPUT output = (VS_OUTPUT)0;
    
    output.Position = mul(input.Position, CB_Model);
    output.Position = mul(output.Position, CB_Scene.ViewProjection);
    
    output.Normal = mul(float4(input.Normal, 1.0), CB_Model).xyz;

    if (CB_ShaderFlags & ShaderFlags_DiffuseTexture)
        output.TexCoord = TransformTextureCoordinates(input.TexCoord, CB_Material.DiffuseTextureTransform);
    
    if (CB_ShaderFlags & ShaderFlags_AmbientTexture)
        output.TexCoordAmbient = TransformTextureCoordinates(input.TexCoordAmbient, CB_Material.AmbientTextureTransform);
    
    float3 diffuse = saturate(dot(input.Normal, CB_Scene.StageLight.Direction.xyz));
    output.Color.rgb = mad(diffuse, CB_Scene.LightColor.rgb, Irradiance * CB_Scene.StageLight.Diffuse.rgb);
    output.Color.a = 1.0;

    if (CB_ShaderFlags & ShaderFlags_VertexColor)
        output.Color *= input.Color;
    
    return output;
}
