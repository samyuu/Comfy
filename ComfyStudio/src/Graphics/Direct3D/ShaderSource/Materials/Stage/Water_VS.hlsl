#include "../Include/InputLayout.hlsl"
#include "../Include/ConstantInputs.hlsl"

#define COMFY_VS
#define ARB_PROGRAM_ACCURATE 1
#include "../Include/Assembly/DebugInterface.hlsl"

VS_OUTPUT VS_main(VS_INPUT input)
{
    VS_OUTPUT output = (VS_OUTPUT)0;
    
#if ARB_PROGRAM_ACCURATE
    
    TEMP _tmp0, _tmp1, _tmp2;
    TEMP eye_w = FLOAT4_ZERO;
    TEMP diff;
    TEMP tmp = FLOAT4_ZERO;
    TEMP pos_v;
    TEMP pos_w;
    TEMP pos_c;
    TEMP pos_m;
    TEMP normal_w = FLOAT4_ZERO;
    TEMP normal_m = FLOAT4_ZERO;
    TEMP tangent_w = FLOAT4_ZERO;
    TEMP tangent_m = FLOAT4_ZERO;
    TEMP binormal_w = FLOAT4_ZERO;
    // MUL(_tmp0, a_morph_position, p_morph_weight.x);
    // MAD(pos_m, a_position, p_morph_weight.y, _tmp0);
    // MUL(_tmp0, a_morph_normal, p_morph_weight.x);
    // MAD(normal_m, a_normal, p_morph_weight.y, _tmp0);
    // MUL(_tmp0, a_morph_tangent, p_morph_weight.x);
    // MAD(tangent_m, a_tangent, p_morph_weight.y, _tmp0);
    
    //MOV(pos_m, a_position);
    //MOV(tangent_m, a_tangent);
    
    VS_SET_MODEL_POSITION_NORMAL_TANGENT;

    DP4(pos_w.x, model_mtx[0], pos_m);
    DP4(pos_w.y, model_mtx[1], pos_m);
    DP4(pos_w.z, model_mtx[2], pos_m);
    DP4(pos_w.w, model_mtx[3], pos_m);
    DP4(pos_v.x, mv[0], pos_m);
    DP4(pos_v.y, mv[1], pos_m);
    DP4(pos_v.z, mv[2], pos_m);
    DP4(pos_v.w, mv[3], pos_m);
    DP4(pos_c.x, mvp[0], pos_m);
    DP4(pos_c.y, mvp[1], pos_m);
    DP4(pos_c.z, mvp[2], pos_m);
    DP4(pos_c.w, mvp[3], pos_m);
    MOV(o_position, pos_c);
    DP3(normal_w.x, model_mtx[0], normal_m);
    DP3(normal_w.y, model_mtx[1], normal_m);
    DP3(normal_w.z, model_mtx[2], normal_m);
    DP3(tangent_w.x, model_mtx[0], tangent_m);
    DP3(tangent_w.y, model_mtx[1], tangent_m);
    DP3(tangent_w.z, model_mtx[2], tangent_m);
    XPD(binormal_w, normal_w, tangent_w);
    MUL(binormal_w, binormal_w, a_tangent.w);
    MOV(o_tangent, tangent_w);
    MOV(o_binormal, binormal_w);
    MOV(o_normal, normal_w);
    
    SUB(_tmp0.w, pos_c.z, state_fog_params.y);
    MUL_SAT(_tmp0.w, _tmp0.w, state_fog_params.w);
    MUL(o_fog.x, _tmp0.w, state_fog_params.x);
    SUB(tmp.w, pos_c.z, program_env_19.y);
    MUL_SAT(tmp.w, tmp.w, program_env_19.w);
    MUL(tmp.w, tmp.w, program_env_19.x);
    
    MAD(o_color_f1.w, tmp.w, p_bump_depth.x, 1);
    
    //VS_SET_OUTPUT_TEX_COORDS;
    VS_SET_OUTPUT_TEX_COORDS_NO_TRANSFORM;
    
    DP3(eye_w.x, camera_mvi[0], -pos_v);
    DP3(eye_w.y, camera_mvi[1], -pos_v);
    DP3(eye_w.z, camera_mvi[2], -pos_v);
    MOV(o_eye, eye_w);
    MOV(diff, 1.0);
    
    if (FLAGS_VERTEX_COLOR)
        MUL(diff, diff, VS_A_COLOR_OR_MORPH);
    
    MUL(diff, diff, state_material_diffuse);
    MUL(o_color_f0, diff, p_blend_color);
    
#endif
    
    return output;
}
