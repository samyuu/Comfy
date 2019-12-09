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

    const float4 tangent = input.Tangent;
    output.Tangent = mul(tangent, CB_Model).xyzw;
    
    output.Binormal.xyz = cross(tangent.xyz, normal.xyz);

    float3 pos_v = mul(input.Position, CB_Scene.View).xyz;
    output.EyeDirection.xyz = mul((float3x3)CB_Scene.View, -(rsqrt(dot(pos_v, pos_v)) * pos_v)).xyz;
    output.EyeDirection.w = 1.0;
    
    output.TexCoord = TransformTextureCoordinates(input.TexCoord, CB_Material.DiffuseTextureTransform);
    output.TexCoordAmbient = TransformTextureCoordinates(input.TexCoordAmbient, CB_Material.AmbientTextureTransform);
    
    output.Color = input.Color * float4(0.5, 0.5, 0.5, 1.0);
    
    return output;
}
