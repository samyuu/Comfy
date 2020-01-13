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
    TEMP diff;
    TEMP tmp;
    TEMP pos_w, pos_c, pos_m;
    
    if (FLAGS_MORPH)
    {
        MUL(_tmp0, a_morph_position, p_morph_weight.x);
        MAD(pos_m, a_position, p_morph_weight.y, _tmp0);
    }
    else
    {
        MOV(pos_m, a_position);
    }
    
    DP4(pos_w.x, model_mtx[0], pos_m);
    DP4(pos_w.y, model_mtx[1], pos_m);
    DP4(pos_w.z, model_mtx[2], pos_m);
    DP4(pos_w.w, model_mtx[3], pos_m);
    DP4(pos_c.x, mvp[0], pos_m);
    DP4(pos_c.y, mvp[1], pos_m);
    DP4(pos_c.z, mvp[2], pos_m);
    DP4(pos_c.w, mvp[3], pos_m);
    MOV(o_position, pos_c);
    MOV(o_fog.x, 0);
    DP4(o_tex0.x, state_matrix_texture0[0], a_tex0);
    DP4(o_tex0.y, state_matrix_texture0[1], a_tex0);
    DP4(o_tex1.x, state_matrix_texture1[0], a_tex1);
    DP4(o_tex1.y, state_matrix_texture1[1], a_tex1);
    MUL(diff, state_material_diffuse, p_blend_color);
    MUL(diff.xyz, state_material_emission.xyz, diff.xyz);
    
    if (FLAGS_VERTEX_COLOR)
    {
        if (FLAGS_MORPH)
        {
            MUL(_tmp0, a_morph_color, p_morph_weight.x);
            MAD(tmp, a_color, p_morph_weight.y, _tmp0);
            MUL(o_color_f0, diff, tmp);
        }
        else
        {
            MUL(o_color_f0, diff, a_color);
        }
    }
    else
    {
        MOV(o_color_f0, diff);
    }
    
    MOV(o_color_f1, p_offset_color);
    
#endif
    
    return output;
}
