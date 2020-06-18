#include "../Include/InputLayout.hlsl"
#include "../Include/ConstantInputs.hlsl"
#include "../Include/TextureInputs.hlsl"

float4 PS_main(VS_OUTPUT input) : SV_Target
{
    float4 textureColor = DiffuseTexture.Sample(DiffuseSampler, input.TexCoord);
    clip(textureColor.a - 0.5);

    return textureColor;
    
    // return float4(1.0, 0.0, 1.0, 1.0);
}
