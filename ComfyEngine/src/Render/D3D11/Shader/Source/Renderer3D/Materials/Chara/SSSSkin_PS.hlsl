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
    
    TEMP normal, eye;
    TEMP org_normal, org_eye;
    TEMP lc, diff, luce;
    TEMP col0, tmp;
    TEX2D_00(col0, a_tex_color0);
    NRM(normal, a_normal);
    MOV(org_normal, normal);
    DP3(normal.x, nt_mtx[0], org_normal);
    DP3(normal.y, nt_mtx[1], org_normal);
    DP3(normal.z, nt_mtx[2], org_normal);
    
    if (FLAGS_SELF_SHADOW)
    {
        PS_SAMPLE_SELF_SHADOW_MAP;
    }
    else
    {
        MOV(lc, 1);
    }
    
    TEX2D_03(tmp, a_tex_color0);
    //MUL(lc.z, lc, tmp.w);
    MUL(lc.z, lc.x, tmp.w);
    MOV(lc.w, tmp.w);
    MAD(tmp.x, lc.z, 0.7, 0.3);
    MUL(tmp.x, tmp.x, a_color1.w);
    MUL(luce.xyz, p_lit_luce.xyz, tmp.x);
    MUL(luce.xyz, luce.xyz, float3(1, 0.90, 1.0));
    MOV(normal.w, 0);
    TXLCUBE_09(diff, normal);
    MOV(normal.w, 1);
    TXLCUBE_09(tmp, normal);
    LRP(diff, lc.y, diff, tmp);
    MUL(diff.xyz, diff.xyz, state_light0_diffuse.xyz);
    ADD(diff.xyz, diff.xyz, state_light0_ambient.xyz);
    ADD(diff.xyz, diff.xyz, luce.xyz);
    ADD(diff.xyz, diff.xyz, a_color0.w);
    MOV(tmp, 0);
    MOV(tmp.xyz, tmp.xyz);
    SGT(tmp.w, lc.w, 0.99);
    MAD(diff.xyz, tmp.xyz, tmp.w, diff.xyz);
    MOV(o_color.xyz, diff.xyz);
    MOV(o_color.w, 1);

#endif
    
    return outputColor;
}
