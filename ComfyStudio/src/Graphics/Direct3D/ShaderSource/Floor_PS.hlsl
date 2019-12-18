#include "Include/InputLayouts.hlsl"
#include "Include/ConstantInputs.hlsl"
#include "Include/Common.hlsl"
#include "Include/TextureInputs.hlsl"

#define COMFY_PS
#define ARB_PROGRAM_ACCURATE 1
#include "Include/DebugInterface.hlsl"

float4 PS_main(VS_OUTPUT input) : SV_Target
{
    float4 outputColor;
 
#if ARB_PROGRAM_ACCURATE

    TEMP color_map, normal_w, spec, reflect, reflect_high, screen_uv, diff, tmp;
    
    TEX2D_00(color_map, a_tex_color0);
    MOV(o_color.w, color_map.w);
    TEX2D_02(normal_w, a_tex_normal0);
    MAD(normal_w, normal_w.wyyy, 2.0, -1.0);
    MUL(screen_uv.xy, fragment_position.xy, p_fb_isize.xy);
    //MAD(screen_uv.xy, normal_w.xz, p_bump_depth.yy, screen_uv.xy);
    MAD(screen_uv.xy, normal_w.xz, p_bump_depth.xx, screen_uv.xy);
    TEX2D_15(reflect, screen_uv);
    TEX2D_03(tmp, a_tex_specular);
    MOV(color_map.w, tmp.w);
    MUL(spec.xyz, a_color1.xyz, tmp.xyz);
    MAD(spec.xyz, reflect.xyz, a_color1.w, spec.xyz);
    MUL(diff, color_map, a_color0);
    MAD(o_color.rgb, spec.xyz, color_map.w, diff.xyz);
    
    CHECK_CLIP_ALPHA_TEST;
    
#endif

    return outputColor;
}
