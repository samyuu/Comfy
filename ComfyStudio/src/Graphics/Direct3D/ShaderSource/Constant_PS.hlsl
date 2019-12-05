#include "Include/InputLayouts.hlsl"
#include "Include/ConstantInputs.hlsl"
#include "Include/Common.hlsl"
#include "Include/TextureInputs.hlsl"

float4 PS_main(VS_OUTPUT input) : SV_Target
{
    float4 outputColor = float4(CB_Material.Diffuse, 1.0);
    
    if (CB_ShaderFlags & ShaderFlags_DiffuseTexture)
        outputColor = DiffuseTexture.Sample(DiffuseSampler, input.TexCoord);
    
    if (CB_ShaderFlags & ShaderFlags_AmbientTexture)
        outputColor *= SampleAmbientTexture(AmbientTexture, AmbientSampler, input.TexCoordAmbient, AmbientTextureType);
    
    return outputColor * input.Color;
}
