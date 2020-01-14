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
    TEMP diff, tmp, pos_w, pos_c, pos_m, normal_w, normal_m;
    
    VS_SET_MODEL_POSITION_NORMAL;
    
    DP3(normal_w.x, model_mtx[0], normal_m);
    DP3(normal_w.y, model_mtx[1], normal_m);
    DP3(normal_w.z, model_mtx[2], normal_m);
    DP4(pos_w.x, model_mtx[0], pos_m);
    DP4(pos_w.y, model_mtx[1], pos_m);
    DP4(pos_w.z, model_mtx[2], pos_m);
    DP4(pos_w.w, model_mtx[3], pos_m);
    DP4(pos_c.x, mvp[0], pos_m);
    DP4(pos_c.y, mvp[1], pos_m);
    DP4(pos_c.z, mvp[2], pos_m);
    DP4(pos_c.w, mvp[3], pos_m);
    MOV(o_position, pos_c);
    SUB(_tmp0.w, pos_c.z, state_fog_params.y);
    MUL_SAT(_tmp0.w, _tmp0.w, state_fog_params.w);
    MUL(o_fog.x, _tmp0.w, state_fog_params.x);
    
    VS_SET_OUTPUT_TEX_COORDS;
    
    MOV(normal_w.w, 1.0);
    VS_SET_DIFFUSE_IRRADIANCE;
    
    MUL(diff, diff, state_light1_diffuse);
    DP3_SAT(tmp.x, normal_w, lit_dir_w);
    MAD(diff, tmp.x, lit_diff, diff);
    MOV(diff.w, 1.0);
    
    if (FLAGS_VERTEX_COLOR)
        MUL(diff, diff, VS_A_COLOR_OR_MORPH);
    
    MUL(o_color_f0, diff, p_blend_color);
    MOV(o_color_f1, p_offset_color);
    
#endif

    return output;
}
