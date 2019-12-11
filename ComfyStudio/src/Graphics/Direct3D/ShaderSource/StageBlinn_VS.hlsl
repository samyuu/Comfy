#include "Include/InputLayouts.hlsl"
#include "Include/ConstantInputs.hlsl"
#include "Include/Common.hlsl"

#define COMFY_VS
#define ARB_PROGRAM_ACCURATE 1
#include "Include/DebugInterface.hlsl"

VS_OUTPUT VS_main(VS_INPUT input)
{
    VS_OUTPUT output = (VS_OUTPUT)0;
    
#if ARB_PROGRAM_ACCURATE
    
    float4 normal_m = float4(a_normal, 1.0);
    float4 pos_m = a_position;
    float4 tangent_m = a_tangent;
    
    // DP4 pos_v.x, mv[0], pos_m; DP4 pos_v.y, mv[1], pos_m; DP4 pos_v.z, mv[2], pos_m; DP4 pos_v.w, mv[3], pos_m;
    float4 pos_v = mul(mul(pos_m, CB_Model), CB_Scene.View);
    
    // DP4 pos_w.x, model_mtx[0], pos_m; DP4 pos_w.y, model_mtx[1], pos_m; DP4 pos_w.z, model_mtx[2], pos_m; DP4 pos_w.w, model_mtx[3], pos_m;
    // TODO: float4 pos_w = mul(mul(pos_m, CB_Model), CB_Scene.ViewProjection);
    
    // DP4 pos_c.x, mvp[0], pos_m; DP4 pos_c.y, mvp[1], pos_m; DP4 pos_c.z, mvp[2], pos_m; DP4 pos_c.w, mvp[3], pos_m;
    float4 pos_c = mul(mul(pos_m, CB_Model), CB_Scene.ViewProjection);
    
    MOV(o_position, pos_c);
    
    // DP3 normal_w.x, model_mtx_it[0], normal_m; DP3 normal_w.y, model_mtx_it[1], normal_m; DP3 normal_w.z, model_mtx_it[2], normal_m;
    float4 normal_w = mul(normal_m, CB_Model);
    
    // DP3 tangent_w.x, model_mtx_it[0], tangent_m; DP3 tangent_w.y, model_mtx_it[1], tangent_m; DP3 tangent_w.z, model_mtx_it[2], tangent_m;
    float4 tangent_w = mul(tangent_m, CB_Model);
    
    DP3(normal_w.w, normal_w, normal_w);
    RSQ(normal_w.w, normal_w.w);
    MUL(normal_w, normal_w, normal_w.wwww);

    DP3(tangent_w.w, tangent_w, tangent_w);
    RSQ(tangent_w.w, tangent_w.w);
    MUL(tangent_w, tangent_w, tangent_w.w);

    float4 binormal_w = float4(0.0, 0.0, 0.0, 0.0);
    XPD(binormal_w.xyz, normal_w.xyz, tangent_w.xyz);
    MUL(binormal_w, binormal_w, a_tangent.w);
    
    MOV(o_tangent, tangent_w);
    MOV(o_binormal, binormal_w);
    MOV(o_normal, normal_w);
    
    DP4(o_tex0.x, state_matrix_texture0[0], float4(a_tex0, 0.0, 0.0)); DP4(o_tex0.y, state_matrix_texture0[1], float4(a_tex0, 0.0, 0.0));
    DP4(o_tex1.x, state_matrix_texture1[0], float4(a_tex1, 0.0, 0.0)); DP4(o_tex1.y, state_matrix_texture1[1], float4(a_tex1, 0.0, 0.0));
    
    // DP3 tmp.x, camera_mvi[0], -pos_v; DP3 tmp.y, camera_mvi[1], -pos_v; DP3 tmp.z, camera_mvi[2], -pos_v;
    float4 tmp = float4(mul((float3x3) CB_Scene.View, -pos_v.xyz), 1.0);
    
    MOV(o_eye, tmp);
    
    float4 eyevec;
    DP3(eyevec.w, tmp, tmp);
    RSQ(eyevec.w, eyevec.w);
    MUL(eyevec, tmp, eyevec.w);
    
    DP3(tmp.w, normal_w, eyevec);
    SUB_SAT(tmp.w, 1.0, tmp.w);
    POW(tmp.w, tmp.w, 5.0);
    
    float4 spec;
    MAD(spec.xyz, tmp.w, p_fres_coef.x, p_fres_coef.y);
    MUL(tmp.z, p_fres_coef.x, 10.0);
    MAD(spec.w, tmp.w, tmp.z, 1);
    MUL(o_color_f1, spec, state_material_specular);
    
    float4 diff;
    MOV(diff, state_light1_diffuse);
    if (FLAGS_VERTEX_COLOR)
    {
        MUL(diff, diff, a_color);
    }
    MOV(o_color_f0, diff);
    
#endif
    
    return output;
}
