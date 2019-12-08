#include "Include/InputLayouts.hlsl"
#include "Include/ConstantInputs.hlsl"
#include "Include/Common.hlsl"

VS_OUTPUT VS_main(VS_INPUT input)
{
    VS_OUTPUT output = (VS_OUTPUT)0;
    
    output.Position = mul(input.Position, CB_Model);
    output.Position = mul(output.Position, CB_Scene.ViewProjection);
    
    output.TexCoord = TransformTextureCoordinates(input.TexCoord, CB_Material.DiffuseTextureTransform);
    output.Color = (CB_ShaderFlags & ShaderFlags_VertexColor) ? input.Color : float4(1.0, 1.0, 1.0, 1.0);
    
    return output;
}
