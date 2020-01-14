#include "../Include/InputLayouts.hlsl"
#include "../Include/ConstantInputs.hlsl"
#include "../Include/Common.hlsl"

#define COMFY_VS
#define ARB_PROGRAM_ACCURATE 1
#include "../Include/DebugInterface.hlsl"

VS_OUTPUT VS_main(VS_INPUT input)
{
    VS_OUTPUT output = (VS_OUTPUT)0;
    
#if ARB_PROGRAM_ACCURATE
    
    TEMP _tmp0;
    TEMP eye_w, diff, spec, spec_ratio, lc, half_w, tmp, pos_v, pos_w, pos_c, pos_m, normal_w = (TEMP)0, normal_m;
    
    MOV(normal_m, a_normal);
    MOV(pos_m, a_position);
    DP4(pos_w.x, model_mtx[0], pos_m);
    DP4(pos_w.y, model_mtx[1], pos_m);
    DP4(pos_w.z, model_mtx[2], pos_m);
    DP4(pos_w.w, model_mtx[3], pos_m);
    DP4(pos_c.x, mvp[0], pos_m);
    DP4(pos_c.y, mvp[1], pos_m);
    DP4(pos_c.z, mvp[2], pos_m);
    DP4(pos_c.w, mvp[3], pos_m);
    MOV(o_position, pos_c);
    DP3(normal_w.x, model_mtx[0], normal_m);
    DP3(normal_w.y, model_mtx[1], normal_m);
    DP3(normal_w.z, model_mtx[2], normal_m);
    MOV(o_normal, normal_w);
    SUB(_tmp0.w, pos_c.z, state_fog_params.y);
    MUL_SAT(_tmp0.w, _tmp0.w, state_fog_params.w);
    MUL(o_fog.x, _tmp0.w, state_fog_params.x);
    
    VS_SET_OUTPUT_TEX_COORDS;
    
    DP4(pos_v.x, mv[0], a_position);
    DP4(pos_v.y, mv[1], a_position);
    DP4(pos_v.z, mv[2], a_position);
    DP4(pos_v.w, mv[3], a_position);
    DP3(tmp.w, pos_v, pos_v);
    RSQ(tmp.w, tmp.w);
    MUL(tmp, pos_v, tmp.w);
    DP3(eye_w.x, camera_mvi[0], -tmp);
    DP3(eye_w.y, camera_mvi[1], -tmp);
    DP3(eye_w.z, camera_mvi[2], -tmp);
    DP3(tmp.w, normal_w, eye_w);
    SUB_SAT(tmp.w, 1.0, tmp.w);
    POW(tmp.w, tmp.w, 5.0);
    MAD(spec_ratio.xyz, tmp.w, p_fres_coef.x, p_fres_coef.y);
    MUL(tmp.z, p_fres_coef.x, 10.0);
    MAD(spec_ratio.w, tmp.w, tmp.z, 1);
    MUL(spec_ratio, spec_ratio, state_material_specular);
    MOV(normal_w.w, 1.0);
    
    VS_SET_DIFFUSE_IRRADIANCE;
    
    MUL(diff, diff, state_light1_diffuse);
    ADD(half_w, lit_dir_w, eye_w);
    DP3(half_w.w, half_w, half_w);
    RSQ(half_w.w, half_w.w);
    MUL(half_w, half_w, half_w.w);
    DP3_SAT(lc.y, normal_w, lit_dir_w);
    DP3_SAT(lc.z, normal_w, half_w);
    MAD(lc.w, state_material_shininess, 112.0, 16.0);
    POW(lc.z, lc.z, lc.w);
    MOV(lc.xw, 1);
    MAD(diff, lc.y, lit_diff, diff);
    MUL(spec.xyz, lc.z, lit_spec);
    MOV(spec.w, 1);
    MUL(o_color_f1, spec, spec_ratio);
    MOV(diff.w, 1.0);
    MUL(diff, diff, state_material_diffuse);
    
    if (FLAGS_VERTEX_COLOR)
        MUL(diff, diff, VS_A_COLOR_OR_MORPH);
    
    MUL(o_color_f0, diff, p_blend_color);
    
#endif

    return output;
}
