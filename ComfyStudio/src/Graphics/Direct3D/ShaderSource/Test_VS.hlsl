#include "InputLayouts.hlsl"
#include "ConstantInputs.hlsl"

VS_OUTPUT VS_main(VS_INPUT input)
{
    VS_OUTPUT output;
    
    output.Position = mul(input.Position, CB_Model);
    output.Position = mul(output.Position, CB_Scene.ViewProjection);
    
    output.Normal = input.Normal;
    output.TexCoord = input.TexCoord;
    output.TexCoordAmbient = input.TexCoordAmbient;
    output.Color = input.Color;

    float3 eyeDirection = normalize(input.Position.xyz - CB_Scene.EyePosition.xyz);
    output.Reflection.xyz = reflect(eyeDirection, input.Normal);
    output.Reflection.w = 1.0;

    return output;
}
