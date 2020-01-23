#include "../Include/InputLayouts.hlsl"
#include "../Include/ConstantInputs.hlsl"
#include "../Include/Common.hlsl"

#define COMFY_VS
#define ARB_PROGRAM_ACCURATE 1
#include "../Include/Assembly/DebugInterface.hlsl"

VS_OUTPUT VS_main(VS_INPUT input)
{
    VS_OUTPUT output = (VS_OUTPUT)0;
    
#if ARB_PROGRAM_ACCURATE

    TEMP _tmp0, _tmp1, _tmp2;
    TEMP tex_eb;
    TEMP diff, tmp;
    TEMP pos_m, pos_w, pos_v, pos_c;
    TEMP normal_w = FLOAT4_ZERO, normal_v;
    TEMP eye_w = FLOAT4_ZERO;
    TEMP pos_eb;
    DP4(pos_v.x, mv[0], a_position);
    DP4(pos_v.y, mv[1], a_position);
    DP4(pos_v.z, mv[2], a_position);
    DP4(pos_v.w, mv[3], a_position);
    DP3(normal_w.x, model_mtx[0], a_normal);
    DP3(normal_w.y, model_mtx[1], a_normal);
    DP3(normal_w.z, model_mtx[2], a_normal);
    DP4(pos_w.x, model_mtx[0], a_position);
    DP4(pos_w.y, model_mtx[1], a_position);
    DP4(pos_w.z, model_mtx[2], a_position);
    DP4(pos_w.w, model_mtx[3], a_position);
    DP3(normal_v.x, mv[0], normal_w);
    DP3(normal_v.y, mv[1], normal_w);
    DP3(normal_v.z, mv[2], normal_w);
    DP4(pos_c.x, mvp[0], a_position);
    DP4(pos_c.y, mvp[1], a_position);
    DP4(pos_c.z, mvp[2], a_position);
    DP4(pos_c.w, mvp[3], a_position);
    MOV(o_position, pos_c);
    MOV(o_normal_diff, normal_w);
    MAD(tex_eb, p_tex_offset, float4(1, 1, 0, 0), a_tex0);
    MOV(o_tex0, tex_eb.xy);
    //MAD(tmp, p_tex_offset.zw, float2(1,1), a_tex0);
    MAD(tmp.xy, p_tex_offset.zw, float2(1, 1), a_tex0.xy);
    MAD(o_tex1, normal_v.xy, float2(-0.1, 0.06), tmp.xy);
    DP4(o_tex_shadow0.x, state_matrix_texture6[0], pos_w);
    DP4(o_tex_shadow0.y, state_matrix_texture6[1], pos_w);
    DP4(o_tex_shadow0.z, state_matrix_texture6[2], pos_w);
    MAD(pos_m, tex_eb, float4(-1, 1, 0, 0), float4(1, 0, 0, 0));
    SUB(pos_m.xy, pos_m.xy, p_tex_model_param.zw);
    MUL(pos_m.xy, pos_m.xy, p_tex_model_param.xy);
    MOV(o_cornea_coord, pos_m);
    MUL(pos_m.xy, pos_m.xy, p_ellipsoid_scale.xy);
    MOV(pos_m.z, 0);
    MOV(pos_m.w, 1);
    MOV(o_model_pos, pos_m);
    MAD(pos_eb, tex_eb, float4(-1, 1, 0, 0), float4(1, 0, 0, 0));
    SUB(pos_eb.xy, pos_eb.xy, p_eb_tex_model_param.zw);
    MUL(pos_eb.xy, pos_eb.xy, p_eb_tex_model_param.xy);
    MUL(tmp, pos_eb, p_eb_radius);
    DP3(tmp.w, tmp, pos_eb);
    MAD(tmp.w, -tmp.w, p_eb_radius.w, p_eb_radius.w);
    //SUBC( tmp.z, tmp.w, 1.0e-6);
    SUB(tmp.z, tmp.w, 1.0e-6);
    
    RSQ(tmp.x, tmp.w);
    //MUL(pos_eb.z (GT.z), tmp.x, tmp.w);
    
    if (tmp.z > 0.0)
        MUL(pos_eb.z, tmp.x, tmp.w);
    
    MUL(tmp, 2, pos_eb);
    MUL(tmp, tmp, p_eb_radius);
    DP3(tmp.w, tmp, tmp);
    RSQ(tmp.w, tmp.w);
    MUL(tmp, tmp, tmp.w);
    DP3(normal_w.x, model_mtx[0], tmp);
    DP3(normal_w.y, model_mtx[1], tmp);
    DP3(normal_w.z, model_mtx[2], tmp);
    MOV(o_normal_spec, normal_w);
    DP3(eye_w.x, camera_mvi[0], -pos_v);
    DP3(eye_w.y, camera_mvi[1], -pos_v);
    DP3(eye_w.z, camera_mvi[2], -pos_v);
    MOV(o_eye, eye_w);
    DP3(eye_w.w, eye_w, eye_w);
    RSQ(eye_w.w, eye_w.w);
    MUL(eye_w, eye_w, eye_w.w);
    MOV(o_color_f0.xyz, 0);
    DP3_SAT(diff.x, normal_v, float3(0, 0, 1));
    POW(diff.x, diff.x, 0.4);
    DP3(diff.y, p_lit_dir, -eye_w);
    MAD_SAT(diff.y, diff.y, 0.5, 0.5);
    MUL(diff.y, diff.y, diff.x);
    MUL(diff.xy, diff.xy, program_env_17.xy);
    ADD(o_color_f0.w, diff.x, diff.y);
    SUB(_tmp0.w, pos_c.z, state_fog_params.y);
    MUL_SAT(_tmp0.w, _tmp0.w, state_fog_params.w);
    MUL(o_fog.x, _tmp0.w, state_fog_params.x);
    MOV(o_color_f1.xyz, p_fog_color.xyz);
    
#endif
    
    return output;
}
