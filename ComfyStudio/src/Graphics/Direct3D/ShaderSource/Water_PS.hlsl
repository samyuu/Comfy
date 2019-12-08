#include "Include/InputLayouts.hlsl"
#include "Include/ConstantInputs.hlsl"
#include "Include/Common.hlsl"
#include "Include/TextureInputs.hlsl"

float4 PS_main(VS_OUTPUT input) : SV_Target
{
    const float4 normalTex = NormalTexture.Sample(NormalSampler, input.TexCoord);
    
    const float4 debugWaterColor = float4(0.22, 0.68, 0.91, 0.01);
    return float4(debugWaterColor.rgb * 0.15, debugWaterColor.a);
}
