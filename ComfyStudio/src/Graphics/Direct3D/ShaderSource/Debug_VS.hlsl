#include "Include/InputLayouts.hlsl"
#include "Include/ConstantInputs.hlsl"

VS_OUTPUT VS_main(VS_INPUT input)
{
    VS_OUTPUT output = (VS_OUTPUT) 0;
    
    output.Position = mul(input.Position, CB_Model);
    output.Position = mul(output.Position, CB_Scene.ViewProjection);
    
    output.Normal = input.Normal;
    output.TexCoord = input.TexCoord;
    output.TexCoordAmbient = input.TexCoordAmbient;
    output.Color = input.Color;

    return output;
}
