#include "../Include/InputLayout.hlsl"
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

    float4 pos_w = ModelToWorldSpace(pos_m);
    float4 pos_v = ModelToViewSpace(pos_m);
	float4 pos_c = ModelToClipSpace(pos_m);

    o_position = pos_c;
    o_normal = float4(normal_w, 1.0);
    o_fog = VS_GetFogFactor(pos_c);
        
    if (FLAGS_SHADOW)
        o_tex_shadow0 = VS_GetShadowTextureCoordinates(pos_w);
    
	//float3 eye_w = VS_GetWorldEye(pos_v * rsqrt(dot(pos_v.xyz, pos_v.xyz)));
	float3 eye_w = VS_GetWorldEye(pos_v);

	float tmp = pow(saturate(1.0 - dot(normal_w, eye_w)), 5.0);
	float4 spec_ratio = float4(mad(tmp, p_fres_coef.x, p_fres_coef.y).xxx, mad(tmp, (p_fres_coef.x * 10.0), 1.0)) * state_material_specular;
	
    float3 half_w = (lit_dir_w.xyz + eye_w);
    half_w = (half_w * rsqrt(dot(half_w, half_w)));

	float4 diff = float4(GetIrradiance(float4(normal_w, 1.0)), 1.0) * state_light1_diffuse;
	diff = float4(mad(saturate(dot(normal_w, lit_dir_w.xyz)), lit_diff, diff).xyz, 1.0) * state_material_diffuse;

    if (FLAGS_VERTEX_COLOR)
        diff *= FLAGS_MORPH ? VS_MorphAttribute(a_color, a_morph_color) : a_color;

    o_color_f0 = diff * p_blend_color;
    o_color_f1 = (float4(pow(saturate(dot(normal_w, half_w.xyz)), mad(state_material_shininess, 112.0, 16.0)) * lit_spec.xyz, 1.0) * spec_ratio);

    return output;
}
