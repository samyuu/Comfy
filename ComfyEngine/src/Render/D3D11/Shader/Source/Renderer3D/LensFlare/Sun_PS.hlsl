#include "../Include/InputLayout.hlsl"
#include "../Include/ConstantInputs.hlsl"
#include "../Include/TextureInputs.hlsl"

float4 PS_main(VS_OUTPUT input) : SV_Target
{
    float4 textureColor = DiffuseTexture.Sample(DiffuseSampler, input.TexCoord);
    clip(textureColor.a - 0.5);

    return textureColor * CB_Object.Material.Diffuse;
}
