#include "InputLayouts.hlsl"
#include "ConstantInputs.hlsl"
#include "Common.hlsl"

VS_OUTPUT VS_main(VS_INPUT input)
{
    VS_OUTPUT output = (VS_OUTPUT)0;
    
    output.Position = mul(input.Position, CB_Model);
    output.Position = mul(output.Position, CB_Scene.ViewProjection);
    
    if (CB_ShaderFlags & ShaderFlags_DiffuseTexture)
        output.TexCoord = TransformTextureCoordinates(input.TexCoord, CB_Material.DiffuseTextureTransform);
    
    if (CB_ShaderFlags & ShaderFlags_AmbientTexture)
        output.TexCoordAmbient = TransformTextureCoordinates(input.TexCoordAmbient, CB_Material.AmbientTextureTransform);
    
    output.Color.rgb = CB_Material.DiffuseColor * BlendColor * CB_Material.EmissionColor.rgb;
    output.Color.a = 1.0;
    
    if (CB_ShaderFlags & ShaderFlags_VertexColor)
        output.Color *= input.Color;
    
    return output;
}