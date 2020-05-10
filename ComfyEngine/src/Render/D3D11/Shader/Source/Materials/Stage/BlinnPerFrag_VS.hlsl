#include "../Include/InputLayout.hlsl"
#include "../Include/ConstantInputs.hlsl"

#define COMFY_VS
#include "../Include/Assembly/DebugInterface.hlsl"
#include "../Include/Assembly/TempRefactor.hlsl"

VS_OUTPUT VS_main(VS_INPUT input)
{
    VS_OUTPUT output = (VS_OUTPUT)0;
    
	float4 pos_m, normal_m, tangent_m;
    if (FLAGS_MORPH)
    {
        VS_SetMorphModelSpaceAttributes(input, pos_m, normal_m, tangent_m);
        VS_SetMorphTransformTextureCoordinates(input, o_tex0, o_tex1);
    }
    else
    {
        VS_SetModelSpaceAttributes(input, pos_m, normal_m, tangent_m);
        VS_SetTransformTextureCoordinates(input, o_tex0, o_tex1);
    }
    
    float3 tangent_w = ModelToWorldSpace(tangent_m.xyz);
    float3 normal_w = ModelToWorldSpace(normal_m.xyz);
    
    float4 pos_w = ModelToWorldSpace(pos_m);
    float4 pos_v = ModelToViewSpace(pos_m);
    float4 pos_c = ModelToClipSpace(pos_m);

    o_position = pos_c;
    
    if (FLAGS_LINEAR_FOG)
        o_fog = VS_GetFogFactor(pos_c);
        
    if (FLAGS_SHADOW)
        o_tex_shadow0 = VS_GetShadowTextureCoordinates(pos_w);
    
    float3 binormal_w = (cross(normal_w.xyz, tangent_w.xyz) * a_tangent.w);
	float3 _tmp0 = float3(0, binormal_w.x, normal_w.x);
	
	binormal_w.x = tangent_w.y;
    normal_w.x = tangent_w.z;
    tangent_w.yz = _tmp0.yz;

    _tmp0.z = normal_w.y;
    normal_w.y = binormal_w.z;
    binormal_w.z = _tmp0.z;

    o_tangent = float4(tangent_w, 1.0);
    o_binormal = float4(binormal_w, 1.0);
    o_normal = float4(normal_w, 1.0);
    
	o_eye.xyz = VS_GetWorldEye(pos_v);

    float4 diff = float4(0.5, 0.5, 0.5, 1.0);
    
    if (FLAGS_VERTEX_COLOR)
        diff *= FLAGS_MORPH_COLOR ? VS_MorphAttribute(a_color, a_morph_color) : a_color;
    
    o_color_f0 = diff * p_blend_color;
    o_color_f1 = p_offset_color;
    
    return output;
}
