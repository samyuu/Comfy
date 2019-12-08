#include "Include/InputLayouts.hlsl"
#include "Include/ConstantInputs.hlsl"
#include "Include/TextureInputs.hlsl"
#include "Include/UncompressRGTC.hlsl"

static const float MaxAlpha = 1.0;

float4 PS_main(VS_OUTPUT input) : SV_Target
{
    float3 texColor;
    texColor.y = DiffuseTexture.SampleLevel(DiffuseSampler, input.TexCoord, 0).r;
    texColor.x = DiffuseTexture.SampleLevel(DiffuseSampler, input.TexCoord, 1).r;
    texColor.z = DiffuseTexture.SampleLevel(DiffuseSampler, input.TexCoord, 2).r;
    texColor.xz = mad(texColor.xz, LUMINANCE_FACTOR, -LUMINANCE_OFFSET);
    
    float4 outputColor;
    outputColor.r = dot(texColor, RED_COEF);
    outputColor.g = dot(texColor, GRN_COEF);
    outputColor.b = dot(texColor, BLU_COEF);
    outputColor.a = max(input.Color.a, MaxAlpha);
    
    outputColor.rgb *= input.Color.rgb * CB_Material.Emission.rgb;
    
    return outputColor;
}
