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

    TEMP _tmp0 = 0, _tmp1 = 0, _tmp2 = 0;
    TEMP dev_pos;
    TEMP normal = FLOAT4_ZERO, reflect, eye, _half;
    TEMP org_normal, org_eye;
    TEMP lc = 0, spec, diff, luce, env;
    TEMP spec_ratio;
    TEMP col0;
    TEMP tmp = FLOAT4_ZERO;
    TEMP ndote;
        
    TEX2D_00(col0, a_tex_color0);
    
    // TODO: p_texcol_coef should be 1
    //MAD(col0.xyz, col0.xyz, p_texcol_coef.xyz, p_texcol_offset.xyz);
    
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
    NRMH(eye, a_eye);
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
    LRP(_half, 1.25, normal, eye);
    NRM(_half, _half);
    DP3(tmp.x, _half, eye);
    SUB_SAT(tmp.x, 1, tmp.x);
    POW(tmp.x, tmp.x, 5);
    MAD(spec_ratio.x, tmp.x, 0.20, 0.022);
    DP3_SAT(spec_ratio.w, normal, eye);
    MOV(ndote.x, spec_ratio.w);
    POW(spec_ratio.w, spec_ratio.w, 0.5);
    MAD(spec_ratio.w, spec_ratio.w, 0.75, 0.25);
    
    if (FLAGS_SHADOW)
    {
        //float esm = saturate(exp2((ESMGauss.Sample(ScreenReflectionSampler, a_tex_shadow0.xy) - a_tex_shadow0.z) * p_esm_k.x));
        //_tmp0 = StageShadowMap.Sample(ScreenReflectionSampler, a_tex_shadow0.xy);
        //_tmp0 = mad(max(_tmp0, esm), program_env_13, program_env_12);
        
        #define TEX2D_19(result, texCoord) result = ESMFull.Sample(ScreenReflectionSampler, (texCoord).xy)
        //return ESMFull.Sample(ScreenReflectionSampler, (a_tex_shadow0).xy);

        TEX2D_19(_tmp0, a_tex_shadow0);
        SUB(_tmp0.x, _tmp0.x, a_tex_shadow0.z);
        MUL(_tmp0.x, _tmp0.x, p_esm_k.x);
        MUL(_tmp0.x, _tmp0.x, state_material_emission.w);
        EX2_SAT(_tmp0.x, _tmp0.x);
        DP3(_tmp0.y, p_lit_dir, org_normal);
        ADD_SAT(_tmp0.y, _tmp0.y, 1);
        MUL(_tmp0.y, _tmp0.y, _tmp0.y);
        MUL(_tmp0.y, _tmp0.y, _tmp0.y);
        MIN(lc.yz, _tmp0.x, _tmp0.y);
        MOV(lc.x, _tmp0.x);

        //return float4(lc.xyz, 1.0);
        //return float4(lc.x, 0.0, 0.0, 1.0);
    }
    else
    {
        MOV(lc, 1);
    }
    
    TEX2D_03(tmp, a_tex_color0);
    MUL(lc.z, lc.z, tmp.w);
    MOV(lc.w, tmp.w);
    MOV(normal.w, 0);
    TXLCUBE_09(diff, normal);
    MOV(normal.w, 1);
    TXLCUBE_09(tmp, normal);
    LRP(diff, lc.y, diff, tmp);
    MUL(diff.xyz, diff.xyz, state_light0_diffuse.xyz);
    ADD(diff.xyz, diff.xyz, a_color0.w);
    ADD(diff.xyz, diff.xyz, state_light0_ambient.xyz);
    MOV(tmp, 0);
    MOV(tmp.xyz, tmp.xyz);
    SGT(tmp.w, lc.w, 0.99);
    MAD(diff.xyz, tmp.xyz, tmp.w, diff.xyz);
    
    MUL(dev_pos, fragment_position, program_env_00);

    TEX2D_16(luce, dev_pos);
    
    MUL(tmp.x, p_sss_param.x, spec_ratio.w);
    LRP(diff, tmp.x, luce, diff);
    MUL(diff.xyz, diff.xyz, col0.xyz);
    MAD(lc.w, p_texcol_coef.w, 15, 0.1);
    TEXCUBE_10(spec, reflect);
    TEXCUBE_11(_tmp2, reflect);
    LRP(spec.xyz, lc.w, spec.xyz, _tmp2.xyz);
    TEXCUBE_12(tmp, reflect);
    TEXCUBE_13(_tmp2, reflect);
    LRP(tmp.xyz, lc.w, tmp.xyz, _tmp2.xyz);
    LRP(spec.xyz, lc.z, spec.xyz, tmp.xyz);
    MUL(spec.xyz, spec.xyz, state_light0_specular.xyz);
    MAD(tmp.x, lc.z, 0.7, 0.3);
    MUL(spec.xyz, spec.xyz, tmp.x);
    ADD(spec_ratio.xyz, spec_ratio.x, p_texcol_coef.w);
    MAD(spec_ratio.xyz, spec_ratio.xyz, p_texspc_coef.xyz, p_texspc_offset.xyz);
    MUL(diff.xyz, diff.xyz, 0.98);
    MAD(diff.xyz, spec.xyz, spec_ratio.xyz, diff.xyz);
    MOV(diff.xyz, diff.xyz);
    LRP(o_color.xyz, a_fogcoord.x, a_color1.xyz, diff.xyz);
    
#endif
    
    return outputColor;
}
