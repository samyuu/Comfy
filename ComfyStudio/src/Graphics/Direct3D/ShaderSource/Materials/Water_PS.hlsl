#include "../Include/InputLayouts.hlsl"
#include "../Include/ConstantInputs.hlsl"
#include "../Include/Common.hlsl"
#include "../Include/TextureInputs.hlsl"

#define COMFY_PS
#define ARB_PROGRAM_ACCURATE 1
#include "../Include/Assembly/DebugInterface.hlsl"

float4 PS_main(VS_OUTPUT input) : SV_Target
{
    float4 outputColor;
 
#if ARB_PROGRAM_ACCURATE
    
    TEMP normal_w, normal_t, eye;
    TEMP tmp, color;
    TEMP refract_map, reflect_map, cube_map, color_map, spec_map;
    TEMP screen_uv;
    TEMP fresnel, reflectivity;
    MOV(color_map, a_color0);
    TEX2D_02(normal_t, a_tex_normal0);
    MAD(normal_t.xy, normal_t.wy, 2.0, -1.0);
    MUL(normal_t.z, 0.15, a_color1.w);
    NRMH(normal_t, normal_t);
    MAD(normal_w, a_tangent, normal_t.x, a_normal);
    MAD(normal_w, a_binormal, normal_t.y, normal_w);
    NRMH(normal_w, normal_w);
    NRM(eye, a_eye);
    // MUL(screen_uv, fragment_position, p_fb_isize);
    // MAD(screen_uv, normal_t.xyxy, p_reflect_refract_uv_scale, screen_uv.xyxy);
    MUL(screen_uv.xy, fragment_position.xy, p_fb_isize.xy);
    MAD(screen_uv.xy, normal_t.xy, p_reflect_refract_uv_scale.xy, screen_uv.xy);
    TEX2D_15(reflect_map, screen_uv.xyww);
    MUL(reflect_map.rgb, reflect_map.xyz, state_material_specular.xyz);
    DP3(fresnel.w, normal_w, eye);
    SUB_SAT(fresnel.w, 1.0, fresnel.w);
    POW(fresnel.w, fresnel.w, 5.0);
    
    //MAD(fresnel.w, fresnel.w, p_fres_coef.x, p_fres_coef.y);
    const float2 p_test_fres_coef = float2(0.3936, 0.18);
    MAD(fresnel.w, fresnel.w, p_test_fres_coef.x, p_test_fres_coef.y);
    
    MUL(reflectivity.a, state_material_specular.a, fresnel.w);
    MUL(reflect_map, reflect_map, reflectivity.a);
    MOV(refract_map, color_map);
    ADD(color.rgb, reflect_map.xyz, refract_map.xyz);
    MOV(color.a, color_map.a);
    // LRP(o_color, a_fogcoord.xxxy, p_fog_color, color);
    MOV(o_color, color);
    
#endif

    return outputColor;
}
