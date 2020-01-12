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
    TEMP pos_m, pos_v, pos_c, pos_w;
    SET_VERTEX_POSITIONS;
    
    TEMP normal_m = a_normal;
    TEMP tangent_m = a_tangent;
    
    MOV(o_position, pos_c);
    
	TEMP normal_w;
	DP3(normal_w.x, model_mtx_it[0], normal_m);
	DP3(normal_w.y, model_mtx_it[1], normal_m);
	DP3(normal_w.z, model_mtx_it[2], normal_m);
    
	TEMP tangent_w;
	DP3(tangent_w.x, model_mtx_it[0], tangent_m);
	DP3(tangent_w.y, model_mtx_it[1], tangent_m);
	DP3(tangent_w.z, model_mtx_it[2], tangent_m);
    
    DP3(normal_w.w, normal_w, normal_w);
    RSQ(normal_w.w, normal_w.w);
    MUL(normal_w, normal_w, normal_w.wwww);

    DP3(tangent_w.w, tangent_w, tangent_w);
    RSQ(tangent_w.w, tangent_w.w);
    MUL(tangent_w, tangent_w, tangent_w.w);

    TEMP binormal_w = float4(0.0, 0.0, 0.0, 0.0);
    XPD(binormal_w.xyz, normal_w.xyz, tangent_w.xyz);
    MUL(binormal_w, binormal_w, a_tangent.w);
    
    MOV(o_tangent, tangent_w);
    MOV(o_binormal, binormal_w);
    MOV(o_normal, normal_w);
    
    DP4(o_tex0.x, state_matrix_texture0[0], a_tex0);
	DP4(o_tex0.y, state_matrix_texture0[1], a_tex0);
    DP4(o_tex1.x, state_matrix_texture1[0], a_tex1);
	DP4(o_tex1.y, state_matrix_texture1[1], a_tex1);
    
	TEMP tmp = float4(0.0, 0.0, 0.0, 0.0);
    DP3(tmp.x, camera_mvi[0], -pos_v);
	DP3(tmp.y, camera_mvi[1], -pos_v);
	DP3(tmp.z, camera_mvi[2], -pos_v);
    
    MOV(o_eye, tmp);
    
    TEMP eyevec;
    DP3(eyevec.w, tmp, tmp);
    RSQ(eyevec.w, eyevec.w);
    MUL(eyevec, tmp, eyevec.w);
    
    DP3(tmp.w, normal_w, eyevec);
    SUB_SAT(tmp.w, 1.0, tmp.w);
    POW(tmp.w, tmp.w, 5.0);
    
    TEMP spec;
    MAD(spec.xyz, tmp.w, p_fres_coef.x, p_fres_coef.y);
    MUL(tmp.z, p_fres_coef.x, 10.0);
    MAD(spec.w, tmp.w, tmp.z, 1);
    MUL(o_color_f1, spec, state_material_specular);
    
    TEMP diff;
    MOV(diff, state_light1_diffuse);
    if (FLAGS_VERTEX_COLOR)
        MUL(diff, diff, a_color);
    MOV(o_color_f0, diff);
    
    SUB(_tmp0.w, pos_c.z, state_fog_params.y);
    MUL_SAT(_tmp0.w, _tmp0.w, state_fog_params.w);
    MUL(o_fog.x, _tmp0.w, state_fog_params.x);
    
#endif
    
    return output;
}
