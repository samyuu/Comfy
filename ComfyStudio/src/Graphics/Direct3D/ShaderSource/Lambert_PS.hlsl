#include "Include/InputLayouts.hlsl"
#include "Include/ConstantInputs.hlsl"
#include "Include/Common.hlsl"

SamplerState DiffuseSampler : register(s0);
SamplerState AmbientSampler : register(s1);

Texture2D DiffuseTexture : register(t0);
Texture2D AmbientTexture : register(t1);

float4 PS_main(VS_OUTPUT input) : SV_Target
{
    float4 outputColor = float4(CB_Material.DiffuseColor, 1.0);
    
    if (CB_ShaderFlags & ShaderFlags_DiffuseTexture)
        outputColor = DiffuseTexture.Sample(DiffuseSampler, input.TexCoord);
    
    if (CB_ShaderFlags & ShaderFlags_AmbientTexture)
        outputColor *= SampleAmbientTexture(AmbientTexture, AmbientSampler, input.TexCoordAmbient, AmbientTextureType);
    
    if (CB_ShaderFlags & ShaderFlags_VertexColor)
        outputColor *= input.Color;

    if (CB_ShaderFlags & ShaderFlags_AlphaTest)
        ClipAlphaThreshold(outputColor.a);
        
    return outputColor * input.Color;
}
