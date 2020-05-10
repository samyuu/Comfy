#include "../Include/InputLayout.hlsl"
#include "../Include/ConstantInputs.hlsl"

#define COMFY_VS
#include "../Include/Assembly/DebugInterface.hlsl"
#include "../Include/Assembly/TempRefactor.hlsl"

VS_OUTPUT VS_main(VS_INPUT input)
{
    VS_OUTPUT output = (VS_OUTPUT)0;
    
    float4 pos_m;
    if (FLAGS_MORPH)
    {
        VS_SetMorphModelSpaceAttributes(input, pos_m);
        VS_SetMorphTransformTextureCoordinates(input, o_tex0, o_tex1);
    }
    else
    {
        VS_SetModelSpaceAttributes(input, pos_m);
        VS_SetTransformTextureCoordinates(input, o_tex0, o_tex1);
    }
    
    float4 pos_w = ModelToWorldSpace(pos_m);
    float4 pos_c = ModelToClipSpace(pos_m);

    o_position = pos_c;
    
    if (FLAGS_LINEAR_FOG)
        o_fog = VS_GetFogFactor(pos_c);
        
    if (FLAGS_SHADOW)
        o_tex_shadow0 = VS_GetShadowTextureCoordinates(pos_w);
    
	float4 diff = state_material_diffuse * float4(state_material_emission.xyz, 1.0) * p_blend_color;
    
    if (FLAGS_VERTEX_COLOR)
        diff *= FLAGS_MORPH_COLOR ? VS_MorphAttribute(a_color, a_morph_color) : a_color;

    o_color_f0 = diff;
    o_color_f1 = p_offset_color;
    
    return output;
}
