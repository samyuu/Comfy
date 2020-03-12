#include "../Include/InputLayout.hlsl"
#include "../Include/ConstantInputs.hlsl"
#include "../Include/TextureInputs.hlsl"

#define COMFY_PS
#define ARB_PROGRAM_ACCURATE 1
#include "../Include/Assembly/DebugInterface.hlsl"

float4 PS_main(VS_OUTPUT input) : SV_Target
{
    float4 outputColor;

#if ARB_PROGRAM_ACCURATE

    TEMP tmp, _tmp0, _tmp1, _tmp2;
    TEX2D_02(tmp, a_tex_normal0);
    MAD(tmp.xy, tmp.wy, 2.0, -1.0);
  
    MUL(tmp.zw, tmp.xy, tmp.xy);
    MUL(tmp.zw, tmp.zw, tmp.xy);
    MUL(tmp, tmp, float4(1.5, 1.5, 2.0, 2.0));
    ADD(tmp.xy, tmp.xy, tmp.zw);
    
    TEMP normal = float4(0.0, 0.0, 0.0, 0.0);
    if (FLAGS_NORMAL_TEX2D)
    {
        MUL(normal.xyz, a_tangent.xyz, tmp.x);
        MAD(normal.xyz, a_binormal.xyz, tmp.y, normal.xyz);
        ADD(normal.xyz, normal.xyz, a_normal.xyz);
        NRMH(normal, normal);
    }
    else
    {
        NRMH(normal, a_normal);
    }
    
    TEMP eye;
    NRM(eye, a_eye);
    
    TEMP reflect;
    DP3(reflect.x, eye, normal);
    MUL(reflect.x, reflect.x, 2.0);
    MAD(reflect.xyz, reflect.x, normal.xyz, -eye.xyz);
    
    TEMP half_;
    ADD(half_, lit_dir, eye);
    NRM(half_, half_);
    DP3_SAT(tmp.y, normal, lit_dir);
    
    DP3_SAT(tmp.z, normal, half_);
    MAD(tmp.w, state_material_shininess, 112.0, 16.0);
    POW(tmp.z, tmp.z, tmp.w);
    MOV(tmp.xw, 1.0);
    
    TEMP lc = float4(tmp.y, 1.0, 1.0, tmp.z);
    
    TEMP diff;
    MOV(normal.w, 1);
    DP4(tmp.x, irrad_r[0], normal); DP4(tmp.y, irrad_r[1], normal); DP4(tmp.z, irrad_r[2], normal); DP4(tmp.w, irrad_r[3], normal);
    DP4(diff.x, normal, tmp);
    DP4(tmp.x, irrad_g[0], normal); DP4(tmp.y, irrad_g[1], normal); DP4(tmp.z, irrad_g[2], normal); DP4(tmp.w, irrad_g[3], normal);
    DP4(diff.y, normal, tmp);
    DP4(tmp.x, irrad_b[0], normal); DP4(tmp.y, irrad_b[1], normal); DP4(tmp.z, irrad_b[2], normal); DP4(tmp.w, irrad_b[3], normal);
    DP4(diff.z, normal, tmp);
    
    MAD(diff.xyz, lc.x, lit_diff.xyz, diff.xyz);
    MAD(diff.xyz, diff.xyz, a_color0.xyz, state_material_emission.xyz);
    
    TEMP col0;
    if (FLAGS_DIFFUSE_TEX2D)
    {
        TEX2D_00(col0, a_tex_color0);
    }
    else
    {
        MOV(col0, state_material_diffuse);
    }
    
    if (FLAGS_AMBIENT_TEX2D)
    {
        TEMP tex_col = diff;
        PS_APPLY_SAMPLE_AMBIENT_TEX_COL;
        diff = tex_col;
    }
    
    MUL(diff.xyz, diff.xyz, col0.xyz);
    MUL(diff.xyz, diff.xyz, lc.y);
    
    TEMP spec;
    MUL(spec.xyz, lc.w, lit_spec.xyz);
    MUL(spec.xyz, lc.z, spec.xyz);
    MUL(spec.xyz, spec.xyz, state_light1_specular.xyz);

    TEMP spec_ratio;
    TEX2D_03(spec_ratio, a_tex_specular);
    MUL(spec_ratio, spec_ratio, a_color1);
    MUL(diff.xyz, diff.xyz, 0.96);
    MAD(diff.xyz, spec_ratio.xyz, spec.xyz, diff.xyz);
    
    TEMP env;
    TEXCUBE_05(env, reflect);
    MUL(env.w, lc.z, state_light1_specular.w);
    MUL(env.xyz, env.xyz, env.w);
    MAD(tmp.xyz, env.xyz, spec_ratio.w, diff.xyz);
    
    if (FLAGS_SHADOW)
    {
        PS_SAMPLE_SHADOW_MAP;
        tmp *= _tmp0;
    }
    
    if (FLAGS_LINEAR_FOG)
    {
        LRP(o_color.rgb, a_fogcoord.x, p_fog_color.xyz, tmp.xyz);
    }
    else
    {
        MOV(o_color.rgb, tmp.xyz);
    }
    
    MUL(diff.w, col0.w, a_color0.w);
    
    // TODO:
    // MAX(o_color.w, diff.w, p_max_alpha.w);
    MOV(o_color.w, diff.w);
    
    PS_ALPHA_TEST;
    
#endif
    
    return outputColor;
}
