#include "../Include/InputLayout.hlsl"
#include "../Include/ConstantInputs.hlsl"
#include "../Include/TextureInputs.hlsl"

//static const float3 DebugColorStart = float4(1.0, 0.0, 1.0, 0.0);
static const float4 DebugColorStart = float4(175.0, 0.0, 175.0, 1.0);
static const float4 DebugColorEnd = float4(25.0, 0.0, 25.0, 1.0);

float4 PS_main(VS_OUTPUT input) : SV_Target
{
    return float4(1.0, 0.0, 1.0, 1.0);
    
    // TODO:
    const float timeFactor = CB_Scene.RenderTime.TimeCos.w;
    return lerp(DebugColorStart, DebugColorEnd, timeFactor);
}
