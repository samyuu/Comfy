#include "Include/InputLayouts.hlsl"
#include "Include/ConstantInputs.hlsl"
#include "Include/Common.hlsl"
#include "Include/TextureInputs.hlsl"

float4 PS_main(VS_OUTPUT input) : SV_Target
{
    // NOTE: Diffuse texture else Diffuse material color
    const float4 diffuseTexColor = (CB_ShaderFlags & ShaderFlags_DiffuseTexture) ?
        DiffuseTexture.Sample(DiffuseSampler, input.TexCoord) : float4(CB_Material.Diffuse.rgb, 1.0);
    
    // NOTE: Ambient texture else white
    const float4 ambientTexColor = (CB_ShaderFlags & ShaderFlags_AmbientTexture) ?
        SampleAmbientTexture(AmbientTexture, AmbientSampler, input.TexCoordAmbient, AmbientTextureType) : float4(1.0, 1.0, 1.0, 1.0);
    
    const float4 normalTexColor = NormalTexture.Sample(NormalSampler, input.TexCoord);
    
    return normalTexColor;
}
