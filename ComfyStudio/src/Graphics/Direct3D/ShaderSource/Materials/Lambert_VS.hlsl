#include "../Include/InputLayouts.hlsl"
#include "../Include/ConstantInputs.hlsl"

#define COMFY_VS
#include "../Include/Assembly/DebugInterface.hlsl"
#include "../Include/Assembly/TempRefactor.hlsl"

VS_OUTPUT VS_main(VS_INPUT input)
{
    VS_OUTPUT output = (VS_OUTPUT)0;
    
    float4 pos_m, normal_m;
    if (FLAGS_MORPH)
    {
        VS_SetMorphModelSpaceAttributes(input, pos_m, normal_m);
        VS_SetMorphTransformTextureCoordinates(input, o_tex0, o_tex1);
    }
    else
    {
        VS_SetModelSpaceAttributes(input, pos_m, normal_m);
        VS_SetTransformTextureCoordinates(input, o_tex0, o_tex1);
    }
    
    float3 normal_w = ModelToWorldSpace(normal_m.xyz);
	float4 pos_c = ModelToClipSpace(pos_m);
    
    o_position = pos_c;
    o_fog = VS_GetFogFactor(pos_c);
    
	float4 diff = float4(mad(saturate(dot(normal_w, lit_dir_w.xyz)), lit_diff.xyz, GetIrradiance(normal_w) * state_light1_diffuse.xyz), 1.0);
    
    if (FLAGS_VERTEX_COLOR)
        diff *= FLAGS_MORPH ? VS_MorphAttribute(a_color, a_morph_color) : a_color;

    o_color_f0 = diff * p_blend_color;
    o_color_f1 = p_offset_color;

    return output;
}
