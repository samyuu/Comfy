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
    
    TEMP _tmp0, _tmp1, _tmp2;
    TEMP normal = (TEMP)0, eye, reflect;
    TEMP org_normal, org_eye;
    TEMP lc, spec, diff, luce, env = (TEMP)0;
    TEMP spec_ratio, tights_alpha;
    TEMP col0;
    TEMP ndote;
    TEMP tmp;
    TEX2D_00(col0, a_tex_color0);
    MUL(col0.xyz, col0.xyz, p_texcol_coef.x);
    MOV(tights_alpha.x, col0.w);
    MAD(tights_alpha.y, state_material_ambient.w, 2.0, 0.01);
    TEX2D_04(tmp, a_tex_parency);
    MOV(tights_alpha.z, tmp.w);
    MAD(tights_alpha.z, -tights_alpha.z, a_color0.x, a_color0.x);
    SUB(tights_alpha.z, 1, tights_alpha.z);
    MUL(tights_alpha.y, tights_alpha.y, tights_alpha.z);
    MAD(tights_alpha.z, tights_alpha.z, 0.5, 0.5);
    MUL(tights_alpha.x, tights_alpha.x, tights_alpha.z);
    MAX(o_color.w, col0.w, p_max_alpha.w);
    TEX2D_02(tmp, a_tex_normal0);
    MAD(tmp.xy, tmp.wy, 2.0, -1.0);
    MUL(tmp.zw, tmp.xy, tmp.xy);
    MUL(tmp.zw, tmp.zw, tmp.xy);
    MUL(tmp, tmp, float4(1.5, 1.5, 2.0, 2.0));
    ADD(tmp.xy, tmp.xy, tmp.zw);
    MUL(normal.xyz, a_tangent.xyz, tmp.x);
    MAD(normal.xyz, a_binormal.xyz, tmp.y, normal.xyz);
    ADD(normal.xyz, normal.xyz, a_normal.xyz);
    NRMH(normal, normal);
    NRM(eye, a_eye);
    MOV(org_normal, normal);
    MOV(org_eye, eye);
    DP3(normal.x, nt_mtx[0], org_normal);
    DP3(normal.y, nt_mtx[1], org_normal);
    DP3(normal.z, nt_mtx[2], org_normal);
    DP3(eye.x, nt_mtx[0], org_eye);
    DP3(eye.y, nt_mtx[1], org_eye);
    DP3(eye.z, nt_mtx[2], org_eye);
    DP3(tmp.x, org_eye, org_normal);
    MUL(tmp.x, tmp.x, 2.0);
    MAD(tmp.xyz, tmp.x, org_normal.xyz, -org_eye.xyz);
    DP3(reflect.x, nt_mtx[0], tmp);
    DP3(reflect.y, nt_mtx[1], tmp);
    DP3(reflect.z, nt_mtx[2], tmp);

    if (FLAGS_SELF_SHADOW)
    {
        PS_SAMPLE_SELF_SHADOW_MAP;
    }
    else
    {
        MOV(lc, 1);
    }
    
    DP3_SAT(tmp.w, normal, eye);
    MOV(ndote.x, tmp.w);
    MAD(tmp.x, tmp.w, -tmp.w, 1.0);
    POW(tmp.x, tmp.x, 8.0);
    MUL(tmp.x, tmp.x, a_color1.w);
    MUL(luce.xyz, p_lit_luce.xyz, tmp.x);
    MUL(luce.xyz, luce.xyz, lc.z);
    SUB(tmp.x, 1.0, tmp.w);
    POW(tmp.x, tmp.x, 5.0);
    MAD(tmp.y, lc.z, 0.7, 0.3);
    MUL(tmp.x, tmp.x, tmp.y);
    MAD(spec_ratio.x, tmp.x, p_fres_coef.x, p_fres_coef.y);
    MUL(spec_ratio.w, p_fres_coef.x, 10.0);
    MAD(spec_ratio.w, spec_ratio.w, tmp.x, 1);
    MUL(spec_ratio, spec_ratio.xxxw, state_material_specular);
    MAD(tmp.x, -tmp.w, tmp.w, 1.00001);
    RSQ(tmp.x, tmp.x);
    MAD(tmp.x, tmp.x, tmp.w, 0.000001);
    RCP(tmp.x, tmp.x);
    MUL_SAT(tights_alpha.w, tights_alpha.y, tmp.x);
    LRP(tights_alpha.w, tights_alpha.x, 1.0, tights_alpha.w);

    if (FLAGS_ENVIRONMENT_CUBE)
    {
        TEXCUBE_05(env, reflect);
    }
    
    MOV(normal.w, 0);
    TXLCUBE_09(diff, normal);
    MOV(normal.w, 1);
    TXLCUBE_09(tmp, normal);
    LRP(diff, lc.y, diff, tmp);
    MAD(diff.xyz, diff.xyz, state_light0_diffuse.xyz, a_color0.w);
    ADD(diff.xyz, diff.xyz, state_light0_ambient.xyz);
    MOV(diff.xyz, diff.xyz);
    MOV(tmp, diff);
    MUL(tmp.xyz, tmp.xyz, p_skin_color.xyz);
    MUL(diff.xyz, diff.xyz, col0.xyz);
    LRP(diff.xyz, tights_alpha.w, diff.xyz, tmp.xyz);
    TEXCUBE_10(spec, reflect);
    TEXCUBE_11(_tmp2, reflect);
    LRP(spec.xyz, state_material_shininess.x, spec.xyz, _tmp2.xyz);
    TEXCUBE_12(tmp, reflect);
    TEXCUBE_13(_tmp2, reflect);
    LRP(tmp.xyz, state_material_shininess.x, tmp.xyz, _tmp2.xyz);
    LRP(spec.xyz, lc.z, spec.xyz, tmp.xyz);
    MUL(spec.xyz, spec.xyz, state_light0_specular.xyz);
    MUL(spec.xyz, spec.xyz, tights_alpha.w);

    if (FLAGS_SPECULAR_TEX2D)
    {
        TEX2D_03(tmp, a_tex_specular);
        MUL(spec_ratio, spec_ratio, tmp);
    }
    
    ADD(spec_ratio.xyz, spec_ratio.xyz, p_texcol_coef.w);
    MUL(diff.xyz, diff.xyz, 0.96);
    MAD(diff.xyz, spec.xyz, spec_ratio.xyz, diff.xyz);
        
    if (FLAGS_ENVIRONMENT_CUBE)
    {
        MAD(env.w, lc.z, 0.5, 0.5);
        MUL(env.w, env.w, state_light0_specular.w);
        MUL(env.xyz, env.xyz, env.w);
        MAD(diff.xyz, env.xyz, spec_ratio.w, diff.xyz);
    }
    
    MAD(diff.xyz, luce.xyz, 0.5, diff.xyz);
    // MOV(diff.xyz, diff.xyz);
    
    if (FLAGS_LINEAR_FOG)
    {
        LRP(o_color.xyz, a_fogcoord.x, a_color1.xyz, diff.xyz);
    }
    else
    {
        MOV(o_color.xyz, diff.xyz);
    }
    
    PS_ALPHA_TEST;
    
#endif
    
    return outputColor;
}
