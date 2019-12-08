#include "Include/InputLayouts.hlsl"
#include "Include/ConstantInputs.hlsl"

float4 PS_main(VS_OUTPUT input) : SV_Target
{
    return float4(1.0, 0.0, 1.0, 1.0);
    return float4(input.Normal, 1.0);
}
