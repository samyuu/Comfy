#include "InputLayouts.hlsl"
#include "ConstantInputs.hlsl"

float4 PS_main(VS_OUTPUT input) : SV_Target
{
    return float4(input.Normal, 1.0);
}
