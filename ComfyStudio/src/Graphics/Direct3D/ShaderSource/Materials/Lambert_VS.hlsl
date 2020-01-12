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
    
    if (FLAGS_MORPH)
    {
        MUL(_tmp0, a_morph_normal, p_morph_weight.x);
        MAD(normal_m, a_normal, p_morph_weight.y, _tmp0);
        MUL(_tmp0, a_morph_position, p_morph_weight.x);
        MAD(pos_m, a_position, p_morph_weight.y, _tmp0);
    }
    else
    {
        MOV(normal_m, a_normal);
        MOV(pos_m, a_position);
    }
    
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
    DP4(o_tex0.x, state_matrix_texture0[0], a_tex0);
    DP4(o_tex0.y, state_matrix_texture0[1], a_tex0);
    DP4(o_tex1.x, state_matrix_texture1[0], a_tex1);
    DP4(o_tex1.y, state_matrix_texture1[1], a_tex1);
    MOV(normal_w.w, 1.0);
    DP4(tmp.x, irrad_r[0], normal_w);
    DP4(tmp.y, irrad_r[1], normal_w);
    DP4(tmp.z, irrad_r[2], normal_w);
    DP4(tmp.w, irrad_r[3], normal_w);
    DP4(diff.x, normal_w, tmp);
    DP4(tmp.x, irrad_g[0], normal_w);
    DP4(tmp.y, irrad_g[1], normal_w);
    DP4(tmp.z, irrad_g[2], normal_w);
    DP4(tmp.w, irrad_g[3], normal_w);
    DP4(diff.y, normal_w, tmp);
    DP4(tmp.x, irrad_b[0], normal_w);
    DP4(tmp.y, irrad_b[1], normal_w);
    DP4(tmp.z, irrad_b[2], normal_w);
    DP4(tmp.w, irrad_b[3], normal_w);
    DP4(diff.z, normal_w, tmp);
    MUL(diff, diff, state_light1_diffuse);
    DP3_SAT(tmp.x, normal_w, lit_dir_w);
    MAD(diff, tmp.x, lit_diff, diff);
    MOV(diff.w, 1.0);
    
    if (FLAGS_VERTEX_COLOR)
    {
        if (FLAGS_MORPH)
        {
            MUL(_tmp0, a_morph_color, p_morph_weight.x);
            MAD(tmp, a_color, p_morph_weight.y, _tmp0);
            MUL(diff, diff, tmp);
        }
        else
        {
            MUL(diff, diff, a_color);
        }
    }
    
    MUL(o_color_f0, diff, p_blend_color);
    MOV(o_color_f1, p_offset_color);
    
#endif

    return output;
}
