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

    output.TexCoord = TransformTextureCoordinates(input.TexCoord, CB_Material.DiffuseTextureTransform);
    output.TexCoordAmbient = TransformTextureCoordinates(input.TexCoordAmbient, CB_Material.AmbientTextureTransform);
    
    output.Color = input.Color;
    
    return output;
}
