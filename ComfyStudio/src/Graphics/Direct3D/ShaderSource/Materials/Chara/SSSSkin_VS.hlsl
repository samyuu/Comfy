#include "../Include/InputLayout.hlsl"
#include "../Include/ConstantInputs.hlsl"

#define COMFY_VS
#define ARB_PROGRAM_ACCURATE 1
#include "../Include/Assembly/DebugInterface.hlsl"

VS_OUTPUT VS_main(VS_INPUT input)
{
    VS_OUTPUT output = (VS_OUTPUT)0;
    
#if ARB_PROGRAM_ACCURATE
    
    TEMP pos_c = 0, pos_v = 0, pos_w = 0;
    TEMP normal_w, tangent_w, binormal_w = 0;
    TEMP normal_v, diff, luce;
    TEMP eye_w;
    DP4(pos_v.x, mv[0], a_position);
    DP4(pos_v.y, mv[1], a_position);
    DP4(pos_v.z, mv[2], a_position);
    DP4(pos_v.w, mv[3], a_position);
    DP3(normal_w.x, model_mtx[0], a_normal);
    DP3(normal_w.y, model_mtx[1], a_normal);
    DP3(normal_w.z, model_mtx[2], a_normal);
    DP3(tangent_w.x, model_mtx[0], a_tangent);
    DP3(tangent_w.y, model_mtx[1], a_tangent);
    DP3(tangent_w.z, model_mtx[2], a_tangent);
    DP4(pos_c.x, mvp[0], a_position);
    DP4(pos_c.y, mvp[1], a_position);
    DP4(pos_c.z, mvp[2], a_position);
    DP4(pos_c.w, mvp[3], a_position);
    DP3(normal_v.x, mv[0], normal_w);
    DP3(normal_v.y, mv[1], normal_w);
    DP3(normal_v.z, mv[2], normal_w);
    MOV(o_position, pos_c);
    DP4(o_tex0.x, state_matrix_texture0[0], a_tex0);
    DP4(o_tex0.y, state_matrix_texture0[1], a_tex0);
    DP4(o_tex_shadow0.x, state_matrix_texture6[0], pos_w);
    DP4(o_tex_shadow0.y, state_matrix_texture6[1], pos_w);
    DP4(o_tex_shadow0.z, state_matrix_texture6[2], pos_w);
    XPD(binormal_w, normal_w, tangent_w);
    MUL(binormal_w, binormal_w, a_tangent.w);
    MOV(o_tangent, tangent_w);
    MOV(o_binormal, binormal_w);
    MOV(o_normal, normal_w);
    DP3(eye_w.x, camera_mvi[0], -pos_v);
    DP3(eye_w.y, camera_mvi[1], -pos_v);
    DP3(eye_w.z, camera_mvi[2], -pos_v);
    MOV(o_eye, eye_w);
    DP3(eye_w.w, eye_w, eye_w);
    RSQ(eye_w.w, eye_w.w);
    MUL(eye_w, eye_w, eye_w.w);
    
    //DP3C(eye_w.w, eye_w, normal_w);
    DP3(eye_w.w, eye_w, normal_w);
    
    MUL(eye_w.w, eye_w.w, 1.02);
    //MAD(o_normal.xyz (LT.w), eye_w, -eye_w.w, normal_w);v
    
    DP3_SAT(luce.x, -eye_w, p_lit_dir);
    POW(luce.w, luce.x, 4.0);
    ADD(luce.x, luce.x, luce.w);
    DP3(luce.y, normal_w, p_lit_dir);
    MAD(luce.y, luce.y, 1.0, 1.0);
    MUL_SAT(luce.y, luce.y, luce.y);
    MUL(luce.x, luce.x, luce.y);
    MUL(o_color_f1.w, luce.x, 0.3);
    DP3_SAT(diff.x, normal_v, float3(0, 0, 1));
    POW(diff.x, diff.x, 0.4);
    DP3(diff.y, p_lit_dir, -eye_w);
    MAD_SAT(diff.y, diff.y, 0.5, 0.5);
    MUL(diff.y, diff.y, diff.x);
    MUL(diff.xy, diff.xy, program_env_17.xy);
    ADD(o_color_f0.w, diff.x, diff.y);

#endif
    
    return output;
}
