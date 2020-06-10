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
    TEMP tangent;
    TEMP normal = (TEMP)0, eye, reflect;
    TEMP org_normal;
    TEMP lc, spec, diff, luce, env = (TEMP)0;
    TEMP spec_ratio;
    TEMP col0;
    TEMP ndote;
    TEMP tmp;

    TEX2D_00(col0, a_tex_color0);
    MUL(col0.xyz, col0.xyz, p_texcol_coef.x);
    
    if (FLAGS_TRANSPARENCY_TEX2D)
    {
        TEX2D_04(tmp, a_tex_parency);
        MOV(col0.w, tmp.w);
    }

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
    NRM(eye.xyz, a_eye);
    NRM(tangent.xyz, a_tangent);
    MOV(org_normal, normal);
    DP3(normal.x, nt_mtx[0], org_normal);
    DP3(normal.y, nt_mtx[1], org_normal);
    DP3(normal.z, nt_mtx[2], org_normal);
    MOV(tmp, eye);
    DP3(eye.x, nt_mtx[0], tmp);
    DP3(eye.y, nt_mtx[1], tmp);
    DP3(eye.z, nt_mtx[2], tmp);
    MOV(tmp, tangent);
    DP3(tangent.x, nt_mtx[0], tmp);
    DP3(tangent.y, nt_mtx[1], tmp);
    DP3(tangent.z, nt_mtx[2], tmp);
    
    if (FLAGS_SELF_SHADOW)
    {
        PS_SAMPLE_SELF_SHADOW_MAP;
    }
    else
    {
        MOV(lc, 1);
    }
    
    MAD(lc.w, lc.x, 0.6, 0.4);
    DP3(tangent.w, tangent, normal);
    MAD(tangent.xyz, -tangent.w, normal.xyz, tangent.xyz);
    NRM(tangent.xyz, tangent.xyz);
    NRM(tangent.xyz, tangent.xyz);
    DP3(tmp.w, tangent, eye);
    MUL(reflect.w, tmp.w, 2);
    MAD(reflect.xyz, -reflect.w, tangent.xyz, eye.xyz);
    MAD(diff.w, -tmp.w, tmp.w, 1.00001);
    RSQ(_tmp0.w, diff.w);
    MUL(diff.w, diff.w, _tmp0.w);
    DP3(tmp.w, tangent, -eye);
    MAD(tmp.x, tmp.w, 0.9, 0.1);
    MUL(tmp.x, tmp.x, tmp.x);
    MUL(tmp.x, tmp.x, tmp.x);
    MAD(spec_ratio.x, tmp.x, p_fres_coef.x, p_fres_coef.y);
    MUL(spec_ratio.w, p_fres_coef.x, 2.0);
    MAD(spec_ratio.w, spec_ratio.w, tmp.x, 1);
    MUL(spec_ratio, spec_ratio.xxxw, state_material_specular);
    DP3_SAT(tmp.w, normal, eye);
    MOV(ndote.x, tmp.w);
    MAD(tmp.x, tmp.w, -tmp.w, 1.0);
    MUL(tmp.x, tmp.x, tmp.x);
    MUL(tmp.x, tmp.x, tmp.x);
    MUL(tmp.x, tmp.x, lc.w);
    MUL(tmp.x, tmp.x, p_fres_coef.z);
    MUL(tmp.x, tmp.x, diff.w);
    MOV(eye.w, 0);
    //TEXCUBE_09(luce, -eye);
    TXLCUBE_09(luce, -eye);
    MUL(luce.xyz, luce.xyz, tmp.x);
    
    if (FLAGS_ENVIRONMENT_CUBE)
    {
        TEXCUBE_05(env, reflect);
    }
    
    MOV(normal.w, 0);
    TXLCUBE_09_XYZ(diff.xyz, normal);
    MOV(normal.w, 1);
    TXLCUBE_09_XYZ(tmp.xyz, normal);
    LRP(diff.xyz, lc.y, diff.xyz, tmp.xyz);
    MAD(diff.xyz, diff.xyz, state_light0_diffuse.xyz, a_color0.w);
    ADD(diff.xyz, diff.xyz, state_light0_ambient.xyz);
    MAD(diff.xyz, luce.xyz, 0.5, diff.xyz);
    MOV(diff.xyz, diff.xyz);
    MOV(tmp.w, diff.w);
    LRP(col0.xyz, tmp.w, col0.xyz, state_material_ambient.xyz);
    MUL(diff.xyz, diff.xyz, col0.xyz);
    TEXCUBE_10(spec, reflect);
    TEXCUBE_10(_tmp2, reflect);
    LRP(spec.xyz, state_material_shininess.x, spec.xyz, _tmp2.xyz);
    TEXCUBE_12(tmp, reflect);
    TEXCUBE_13(_tmp2, reflect);
    LRP(tmp.xyz, state_material_shininess.x, tmp.xyz, _tmp2.xyz);
    LRP(spec.xyz, lc.z, spec.xyz, tmp.xyz);
    MUL(spec.xyz, spec.xyz, state_light0_specular.xyz);
    
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
        MUL(env.xyz, env.xyz, env.w);
        MAD(diff.xyz, env.xyz, spec_ratio.w, diff.xyz);
    }
    
    MAD(diff.xyz, luce.xyz, 0.5, diff.xyz);
    MOV(diff.xyz, diff.xyz);
    
    if (FLAGS_LINEAR_FOG)
    {
        LRP(o_color.xyz, a_fogcoord.x, a_color1.xyz, diff.xyz);
    }
    else
    {
        MOV(o_color.xyz, diff.xyz);
    }
    
#endif
    
    return outputColor;
}
