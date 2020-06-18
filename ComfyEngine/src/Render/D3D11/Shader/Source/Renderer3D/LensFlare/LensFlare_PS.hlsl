#include "../Include/InputLayout.hlsl"
#include "../Include/ConstantInputs.hlsl"
#include "../Include/TextureInputs.hlsl"

static const float4 DEBUG_TINT = float4(1.0f, /*0.0f*/1.0, 1.0f, 1.0f);

float4 PS_main(VS_OUTPUT input) : SV_Target
{
    return DiffuseTexture.Sample(DiffuseSampler, input.TexCoord) * CB_Object.Material.Diffuse.a * DEBUG_TINT;
}
