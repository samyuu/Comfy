#include "../Include/InputLayout.hlsl"
#include "../Include/ConstantInputs.hlsl"

#define COMFY_VS
#define ARB_PROGRAM_ACCURATE 1
#include "../Include/Assembly/DebugInterface.hlsl"
#include "../Include/Assembly/TempRefactor.hlsl"

VS_OUTPUT VS_main(VS_INPUT input)
{
    VS_OUTPUT output = (VS_OUTPUT)0;
    
#if ARB_PROGRAM_ACCURATE
    
    TEMP diff, spec, luce;
    TEMP pos_v;
    TEMP pos_w;
    TEMP pos_c;
    TEMP normal_w = (TEMP)0;
    TEMP tangent_w = (TEMP)0;
    TEMP binormal_w = (TEMP)0;
    TEMP normal_v;
    TEMP eye_w = (TEMP)0;
    DP4(pos_v.x, mv[0], a_position);
    DP4(pos_v.y, mv[1], a_position);
    DP4(pos_v.z, mv[2], a_position);
    DP4(pos_v.w, mv[3], a_position);
    DP4(pos_w.x, model_mtx[0], a_position);
    DP4(pos_w.y, model_mtx[1], a_position);
    DP4(pos_w.z, model_mtx[2], a_position);
    DP4(pos_w.w, model_mtx[3], a_position);
    DP3(normal_w.x, model_mtx[0], a_normal);
    DP3(normal_w.y, model_mtx[1], a_normal);
    DP3(normal_w.z, model_mtx[2], a_normal);
    DP3(tangent_w.x, model_mtx[0], a_tangent);
    DP3(tangent_w.y, model_mtx[1], a_tangent);
    DP3(tangent_w.z, model_mtx[2], a_tangent);
    DP3(normal_v.x, mv[0], a_normal);
    DP3(normal_v.y, mv[1], a_normal);
    DP3(normal_v.z, mv[2], a_normal);
    DP4(pos_c.x, mvp[0], a_position);
    DP4(pos_c.y, mvp[1], a_position);
    DP4(pos_c.z, mvp[2], a_position);
    DP4(pos_c.w, mvp[3], a_position);
    MOV(o_position, pos_c);
    XPD(binormal_w, normal_w, tangent_w);
    MUL(binormal_w.xyz, binormal_w.xyz, a_tangent.w);
    MOV(o_tangent, tangent_w);
    MOV(o_binormal, binormal_w);
    MOV(o_normal, normal_w);
    
    VS_SET_OUTPUT_TEX_COORDS;
    
    if (FLAGS_LINEAR_FOG)
        o_fog = VS_GetFogFactor(pos_c);
        
    if (FLAGS_SELF_SHADOW)
        o_tex_shadow0 = VS_GetShadowTextureCoordinates(pos_w);
    
    DP3(eye_w.x, camera_mvi[0], -pos_v);
    DP3(eye_w.y, camera_mvi[1], -pos_v);
    DP3(eye_w.z, camera_mvi[2], -pos_v);
    MOV(o_eye, eye_w);
    DP3(eye_w.w, eye_w, eye_w);
    RSQ(eye_w.w, eye_w.w);
    MUL(eye_w, eye_w, eye_w.w);
    // DP3C(eye_w.w, eye_w, normal_w);
    MUL(eye_w.w, eye_w.w, 1.02);
    // MAD(o_normal.xyz (LT.w), eye_w, -eye_w.w, normal_w);
    DP3_SAT(luce.x, -eye_w, p_lit_dir);
    POW(luce.w, luce.x, 8.0);
    ADD(luce.x, luce.x, luce.w);
    DP3(luce.y, normal_w, p_lit_dir);
    MAD(luce.y, luce.y, 1.0, 1.0);
    MUL_SAT(luce.y, luce.y, luce.y);
    MUL(luce.x, luce.x, luce.y);
    MUL(o_color_f1.w, luce.x, fres_coef.z);
    MOV(o_color_f0.xyz, 0);
    DP3_SAT(diff.x, normal_v, float3(0, 0, 1));
    POW(diff.x, diff.x, 0.4);
    DP3(diff.y, p_lit_dir, -eye_w);
    MAD_SAT(diff.y, diff.y, 0.5, 0.5);
    MUL(diff.y, diff.y, diff.x);
    MUL(diff.xy, diff.xy, program_env_17.xy);
    ADD(o_color_f0.w, diff.x, diff.y);
    // SUB(_tmp0.w, pos_c.z, state.fog.params.y);
    // MUL_SAT(_tmp0.w, _tmp0.w, state.fog.params.w);
    // MUL(o_fog.x, _tmp0.w, state.fog.params.x);
    MOV(o_color_f1.xyz, p_fog_color);
    
#endif

    return output;
}
