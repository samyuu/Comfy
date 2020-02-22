#include "../Include/InputLayout.hlsl"
#include "../Include/ConstantInputs.hlsl"
#include "../Include/TextureInputs.hlsl"

#define COMFY_PS
#include "../Include/Assembly/DebugInterface.hlsl"

float4 PS_main(VS_OUTPUT input) : SV_Target
{
    float4 outputColor;
    
    TEMP _tmp0, _tmp1, _tmp2;
    TEMP normal, eye, reflect;
    TEMP org_normal, org_eye;
    TEMP lc, spec, diff, env;
    TEMP spec_ratio;
    TEMP col0;
    TEMP tmp;
    TEMP ndote;
    
    if (FLAGS_DIFFUSE_TEX2D)
    {
        TEX2D_00(col0, a_tex_color0);
    }
    else
    {
        MOV(col0, state_material_diffuse);
    }
    
    if (FLAGS_NORMAL_TEX2D)
    {
        TEX2D_02(tmp, a_tex_normal0);
        MAD(tmp.xy, tmp.wyyy, 2.0, -1.0);
        MUL(tmp.zw, tmp.xyyy, tmp.xyyy);
        MUL(tmp.zw, tmp.zwww, tmp.xyyy);
        MUL(tmp, tmp, float4(1.5, 1.5, 2.0, 2.0));
        ADD(tmp.xy, tmp.xyyy, tmp.zwww);
        MUL(normal.xyz, a_tangent, tmp.x);
        MAD(normal.xyz, a_binormal, tmp.y, normal);
        ADD(normal.xyz, normal, a_normal);
        //NRMH(normal, normal);
        NRM(normal, normal);
    }
    else
    {
        //NRMH(normal, a_normal);
        NRM(normal, a_normal);
    }
    
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
    MAD(tmp.xyz, tmp.x, org_normal, -org_eye);
    DP3(reflect.x, nt_mtx[0], tmp);
    DP3(reflect.y, nt_mtx[1], tmp);
    DP3(reflect.z, nt_mtx[2], tmp);
    
    if (FLAGS_SHADOW)
    {
        PS_SAMPLE_CHARA_SHADOW_MAP;
    }
    else
    {
        MOV(lc, 1);
    }
    
    DP3_SAT(tmp.w, normal, eye);
    MOV(ndote.x, tmp.w);
    SUB(tmp.x, 1.0, tmp.w);
    POW(tmp.x, tmp.x, 5.0);
    MAD(tmp.y, lc.z, 0.7, 0.3);
    MUL(tmp.x, tmp.x, tmp.y);
    MAD(spec_ratio.x, tmp.x, p_fres_coef.x, p_fres_coef.y);
    MUL(spec_ratio.w, p_fres_coef.x, 10.0);
    MAD(spec_ratio.w, spec_ratio.w, tmp.x, 1);
    MUL(spec_ratio, spec_ratio.xxxw, state_material_specular);
    
    MOV(normal.w, 0);
    TXLCUBE_09(diff, normal);
    MOV(normal.w, 1);
    TXLCUBE_09(tmp, normal);
    LRP(diff, lc.y, diff, tmp);
    MAD(diff.xyz, diff, state_light0_diffuse, a_color0.w);
    ADD(diff.xyz, diff, state_light0_ambient);
    ADD(diff.xyz, diff, state_material_emission);
    MOV(diff.xyz, diff);
    MUL(diff.xyz, diff, col0);

    if (FLAGS_SPECULAR_TEX2D)
    {
        TEXCUBE_10(spec, reflect);
        TEXCUBE_11(_tmp2, reflect);
        LRP(spec.xyz, state_material_shininess.x, spec, _tmp2);
        MIN(tmp, spec, 3.0);
        LRP(spec.xyz, lc.z, spec, tmp);
        MUL(spec.xyz, spec, state_light0_specular);
        TEX2D_03(tmp, a_tex_specular);
        MUL(spec_ratio, spec_ratio, tmp);
        
        MUL(diff.xyz, diff, 0.96);
        MAD(diff.xyz, spec, spec_ratio, diff);
    }
    else
    {
        MUL(diff.xyz, diff, 0.96);
    }

    if (FLAGS_REFLECTION_CUBE)
    {
        TEXCUBE_05(env, reflect);
        MAD(env.w, lc.z, 0.5, 0.5);
        MUL(env.w, env.w, state_light0_specular.w);
        MUL(env.xyz, env, env.w);
        MAD(diff.xyz, env, spec_ratio.w, diff);
    }
    
    MOV(diff.xyz, diff);
    
    LRP(o_color.xyz, a_fogcoord.x, a_color1, diff);
    //MAX(o_color.w, col0.w, p_max_alpha.w);
    MOV(o_color.w, col0.w);
    
    PS_ALPHA_TEST;

    return outputColor;
}
