#include "../Include/InputLayout.hlsl"
#include "../Include/ConstantInputs.hlsl"
#include "../Include/TextureInputs.hlsl"

static const float3 DebugColorStart = float3(1.0, 0.0, 1.0);
static const float3 DebugColorEnd = float3(0.0, 0.0, 0.0);

float4 PS_main(VS_OUTPUT input) : SV_Target
{
    // TODO:
    const float timeFactor = CB_Scene.RenderTime.TimeCos.w;
    return float4(lerp(DebugColorStart, DebugColorEnd, timeFactor), 1.0);
}
