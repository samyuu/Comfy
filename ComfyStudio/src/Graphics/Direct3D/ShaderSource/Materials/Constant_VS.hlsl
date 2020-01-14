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
    
    VS_SET_MODEL_POSITION;
    
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
    
    VS_SET_OUTPUT_TEX_COORDS;

    MUL(diff, state_material_diffuse, p_blend_color);
    MUL(diff.xyz, state_material_emission.xyz, diff.xyz);
    
    if (FLAGS_VERTEX_COLOR)
    {
        MUL(o_color_f0, diff, VS_A_COLOR_OR_MORPH);
    }
    else
    {
        MOV(o_color_f0, diff);
    }
    
    MOV(o_color_f1, p_offset_color);
    
#endif
    
    return output;
}
