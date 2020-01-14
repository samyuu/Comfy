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

    TEMP _tmp0, _tmp1, _tmp2;
    TEMP diff, tmp, pos_v, pos_w, pos_c, pos_m, normal_w = FLOAT4_ZERO, normal_m, tangent_w = FLOAT4_ZERO, tangent_m, binormal_w = FLOAT4_ZERO;
    
    VS_SET_MODEL_POSITION_NORMAL_TANGENT;
    
    DP3(tangent_w.x, model_mtx[0], tangent_m);
    DP3(tangent_w.y, model_mtx[1], tangent_m);
    DP3(tangent_w.z, model_mtx[2], tangent_m);
    DP3(normal_w.x, model_mtx[0], normal_m);
    DP3(normal_w.y, model_mtx[1], normal_m);
    DP3(normal_w.z, model_mtx[2], normal_m);
    DP4(pos_v.x, mv[0], pos_m);
    DP4(pos_v.y, mv[1], pos_m);
    DP4(pos_v.z, mv[2], pos_m);
    DP4(pos_v.w, mv[3], pos_m);
    DP4(pos_w.x, model_mtx[0], pos_m);
    DP4(pos_w.y, model_mtx[1], pos_m);
    DP4(pos_w.z, model_mtx[2], pos_m);
    DP4(pos_w.w, model_mtx[3], pos_m);
    DP4(pos_c.x, mvp[0], pos_m);
    DP4(pos_c.y, mvp[1], pos_m);
    DP4(pos_c.z, mvp[2], pos_m);
    DP4(pos_c.w, mvp[3], pos_m);
    MOV(o_position, pos_c);
    XPD(binormal_w, normal_w, tangent_w);
    MUL(binormal_w, binormal_w, a_tangent.w);
    MOV(_tmp0.y, binormal_w.x);
    MOV(_tmp0.z, normal_w.x);
    MOV(binormal_w.x, tangent_w.y);
    MOV(normal_w.x, tangent_w.z);
    MOV(tangent_w.yz, _tmp0.yz);
    MOV(_tmp0.z, normal_w.y);
    MOV(normal_w.y, binormal_w.z);
    MOV(binormal_w.z, _tmp0.z);
    MOV(o_tangent, tangent_w);
    MOV(o_binormal, binormal_w);
    MOV(o_normal, normal_w);
    SUB(_tmp0.w, pos_c.z, state_fog_params.y);
    MUL_SAT(_tmp0.w, _tmp0.w, state_fog_params.w);
    MUL(o_fog.x, _tmp0.w, state_fog_params.x);
    
    VS_SET_OUTPUT_TEX_COORDS;
    
    DP3(tmp.w, pos_v, pos_v);
    RSQ(tmp.w, tmp.w);
    MUL(tmp, pos_v, tmp.w);
    DP3(o_eye.x, camera_mvi[0], -tmp);
    DP3(o_eye.y, camera_mvi[1], -tmp);
    DP3(o_eye.z, camera_mvi[2], -tmp);
    
    MOV(diff, float4(0.5, 0.5, 0.5, 1.0));
    
    if (FLAGS_VERTEX_COLOR)
        MUL(diff, diff, VS_A_COLOR_OR_MORPH);
    
    MUL(o_color_f0, diff, p_blend_color);
    MOV(o_color_f1, p_offset_color);
    
#endif
    
    return output;
}
