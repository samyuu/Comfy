#include "../Include/InputLayout.hlsl"
#include "../Include/ConstantInputs.hlsl"
#include "../Include/TextureInputs.hlsl"

float4 PS_main(VS_OUTPUT input) : SV_Target
{
    return float4(input.Normal.xyz, 1.0);
}
