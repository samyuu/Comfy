#include "../Include/InputLayoutShadow.hlsl"
#include "../Include/ConstantInputs.hlsl"
#include "../Include/Common.hlsl"
#include "../Include/TextureInputs.hlsl"

float4 PS_main(VS_OUTPUT_SHADOW input) : SV_Target
{
    float4 outputColor = float4(0.0, 0.0, 0.0, 1.0);
    
    if (FLAGS_DIFFUSE_TEX2D)
    {    
        outputColor.a = DiffuseTexture.Sample(DiffuseSampler, input.TexCoord).a;
        ClipAlphaThreshold(outputColor.a);
    }
    
    return outputColor;
}
