#include "Include/InputLayouts.hlsl"
#include "Include/ConstantInputs.hlsl"
#include "Include/Common.hlsl"
#include "Include/TextureInputs.hlsl"

#define COMFY_PS
#define ARB_PROGRAM_ACCURATE 1
#include "Include/DebugInterface.hlsl"

float4 PS_main(VS_OUTPUT input) : SV_Target
{
    float4 outputColor;
    
#if ARB_PROGRAM_ACCURATE

    TEMP _tmp0, _tmp1, _tmp2;
    TEMP dev_pos;
    TEMP normal = (TEMP)0;
    TEMP eye, reflect;
    TEMP org_normal, org_eye;
    TEMP lc, spec, diff, luce, env;
    TEMP spec_ratio, fres;
    TEMP col0;
    TEMP tmp;
    TEMP ndote;
    TEX2D_00(col0, a_tex_color0);
    MUL(col0.xyz, col0.xyz, p_texcol_coef.x);
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
    MOV(lc, 1);
    DP3_SAT(fres.w, normal, eye);
    MOV(ndote.x, fres.w);
    MAD(fres.x, fres.w, -fres.w, 1.0);
    MUL(fres.x, fres.x, fres.x);
    MUL(fres.x, fres.x, fres.x);
    SUB(fres.w, 1.0, fres.w);
    POW(fres.w, fres.w, 5.0);
    MUL(tmp.x, fres.x, a_color1.w);
    MUL(tmp.x, tmp.x, 0.75);
    MUL(tmp.x, tmp.x, lc.x);
    MUL(luce.xyz, p_lit_luce.xyz, tmp.x);
    DP3(tmp.w, -org_eye, p_lit_dir);
    MAD(tmp.w, tmp.w, 0.2, 0.4);
    MUL(tmp.w, tmp.w, fres.w);
    MUL(luce.w, tmp.w, 0.5);
    MOV(spec_ratio, state_material_specular);
    TEX2D_03(tmp, a_tex_specular);
    MUL(spec_ratio, spec_ratio, tmp);
    DP3(tmp.w, spec_ratio, float3(1, 1, 1));
    MAD_SAT(tmp.w, tmp.w, -3.0, 1.3);
    MUL(luce, luce, tmp.w);
    MAD(tmp.y, lc.z, 0.7, 0.3);
    MUL(tmp.y, tmp.y, fres.w);
    MAD(tmp.x, tmp.y, p_fres_coef.x, p_fres_coef.y);
    MUL(tmp.w, p_fres_coef.x, 10.0);
    MAD(tmp.w, tmp.w, tmp.y, 1);
    MAD(spec_ratio, spec_ratio, tmp.xxxw, p_texcol_coef.w);
    TEXCUBE_05(env, reflect);
    MOV(normal.w, 0);
    TEXCUBE_09(diff, normal);
    MOV(normal.w, 1);
    TEXCUBE_09(tmp, normal);
    LRP(diff, lc.y, diff, tmp);
    MAD(diff.xyz, diff.xyz, state_light0_diffuse.xyz, a_color0.w);
    ADD(diff.xyz, diff.xyz, state_light0_ambient.xyz);
    ADD(diff.xyz, diff.xyz, luce.xyz);
    MOV(diff.xyz, diff.xyz);
    MUL(dev_pos, fragment_position, program_env_00);
    TEX2D_16(tmp, dev_pos);
    LRP(diff, state_material_ambient.w, diff, tmp);
    MUL(diff.xyz, diff.xyz, col0.xyz);
    TEXCUBE_10(spec, reflect);
    TEXCUBE_11(_tmp2, reflect);
    // LRP(spec.xyz, state_material_shininess, spec.xyz, _tmp2.xyz);
    LRP(spec.xyz, state_material_shininess.xxx, spec.xyz, _tmp2.xyz);
    TEXCUBE_12(tmp, reflect);
    TEXCUBE_13(_tmp2, reflect);
    // LRP(tmp.xyz, state_material_shininess, tmp.xyz, _tmp2.xyz);
    LRP(tmp.xyz, state_material_shininess.xxx, tmp.xyz, _tmp2.xyz);
    LRP(spec.xyz, lc.z, spec.xyz, tmp.xyz);
    MUL(spec.xyz, spec.xyz, state_light0_specular.xyz);
    MUL(diff.xyz, diff.xyz, 0.96);
    MAD(diff.xyz, spec.xyz, spec_ratio.xyz, diff.xyz);
    MAD(env.w, lc.z, 0.5, 0.5);
    MUL(env.w, env.w, state_light0_specular.w);
    MUL(env.xyz, env.xyz, env.w);
    MAD(diff.xyz, env.xyz, spec_ratio.w, diff.xyz);
    ADD(diff.xyz, diff.xyz, luce.w);
    MOV(diff.xyz, diff.xyz);
    // LRP(o_color.xyz, a_fogcoord.x, a_color1, diff);
    MOV(o_color.xyz, diff.xyz);
    
    CHECK_CLIP_ALPHA_TEST;

#endif

    return outputColor;
}
