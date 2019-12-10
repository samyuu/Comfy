#include "Include/InputLayouts.hlsl"
#include "Include/ConstantInputs.hlsl"
#include "Include/TextureInputs.hlsl"
#include "Include/UncompressRGTC.hlsl"

static const float MaxAlpha = 1.0;

float4 PS_main(VS_OUTPUT input) : SV_Target
{
    float4 outputColor;
    
    if (CB_TextureFormats.Diffuse == TextureFormat_RGTC1)
    {
        outputColor = UncompressRGTC1_RGBA(DiffuseTexture, DiffuseSampler, input.TexCoord);
    }
    else
    {
        outputColor = DiffuseTexture.Sample(DiffuseSampler, input.TexCoord);
    }
    
    outputColor.rgb *= input.Color.rgb * CB_Material.Emission.rgb;
    outputColor.a = max(input.Color.a, MaxAlpha);
    
    return outputColor;
}
