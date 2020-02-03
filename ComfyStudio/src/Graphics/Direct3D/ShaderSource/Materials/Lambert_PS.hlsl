#include "../Include/InputLayouts.hlsl"
#include "../Include/ConstantInputs.hlsl"
#include "../Include/Common.hlsl"
#include "../Include/TextureInputs.hlsl"

#define COMFY_PS
#define ARB_PROGRAM_ACCURATE 1
#include "../Include/Assembly/DebugInterface.hlsl"

float4 PS_main(VS_OUTPUT input) : SV_Target
{
    float4 outputColor;
    
#if ARB_PROGRAM_ACCURATE

    TEMP _tmp0, _tmp2, tex_col;
    
    if (FLAGS_DIFFUSE_TEX2D)
    {
        TEX2D_00(tex_col, a_tex_color0);
        
        if (FLAGS_AMBIENT_TEX2D)
        {
            PS_APPLY_SAMPLE_AMBIENT_TEX_COL;
        }
    }
    else
    {
        MOV(tex_col, state_material_diffuse);
    }

    if (FLAGS_STAGE_SHADOW)
    {
        PS_SAMPLE_STAGE_SHADOW_MAP;
        tex_col *= _tmp0;
    }

    // MUL(o_color, a_color0, tex_col);
    MUL(_tmp0, a_color0, tex_col);
    LRP(o_color, a_fogcoord.xxxy, p_fog_color, _tmp0);
    
    PS_ALPHA_TEST;
    
#endif

    return outputColor;
}
