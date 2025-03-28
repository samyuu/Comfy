#include "../../Include/InputLayout.hlsl"
#include "../../Include/ConstantInputs.hlsl"
#include "../../Include/TextureInputs.hlsl"

#define COMFY_PS
#define ARB_PROGRAM_ACCURATE 1
#include "../../Include/Assembly/DebugInterface.hlsl"

float4 PS_main(VS_OUTPUT input) : SV_Target
{
    float4 outputColor;
    
#if ARB_PROGRAM_ACCURATE

    TEMP _tmp0, _tmp2, tex_col;
    
    if (FLAGS_DIFFUSE_TEX2D)
    {
        TEX2D_00(tex_col, a_tex_color0);
    }
    else
    {
        MOV(tex_col, state_material_diffuse);
    }
    
    if (FLAGS_AMBIENT_TEX2D)
    {
        PS_APPLY_SAMPLE_AMBIENT_TEX_COL;
    }
    
    if (FLAGS_SHADOW)
    {
        PS_SAMPLE_SHADOW_MAP;
        tex_col *= _tmp0;
    }
    
    if (FLAGS_LINEAR_FOG)
    {
        MUL(_tmp0, tex_col, a_color0);
        LRP(o_color, a_fogcoord.xxxy, p_fog_color, _tmp0);
    }
    else
    {
        MUL(o_color, tex_col, a_color0);
    }
    
    PS_ALPHA_TEST;
    
#endif

    return outputColor;
}
