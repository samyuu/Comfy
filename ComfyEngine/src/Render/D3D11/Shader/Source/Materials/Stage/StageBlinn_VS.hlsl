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
    
    float4 tangent_w = float4(ModelToWorldSpace(tangent_m.xyz), 1.0);
    float4 normal_w = float4(ModelToWorldSpace(normal_m.xyz), 1.0);

    float4 pos_w = ModelToWorldSpace(pos_m);
    float4 pos_v = ModelToViewSpace(pos_m);
    float4 pos_c = ModelToClipSpace(pos_m);

    o_position = pos_c;
    
    if (FLAGS_LINEAR_FOG)
        o_fog = VS_GetFogFactor(pos_c);
        
    if (FLAGS_SHADOW)
        o_tex_shadow0 = VS_GetShadowTextureCoordinates(pos_w);
    
    normal_w.w = rsqrt(dot(normal_w.xyz, normal_w.xyz));
    normal_w = (normal_w * normal_w.w);

    tangent_w.w = rsqrt(dot(tangent_w.xyz, tangent_w.xyz));
    tangent_w = (tangent_w * tangent_w.w);

    float4 binormal_w = float4(cross(normal_w.xyz, tangent_w.xyz), 0.0) * a_tangent.w;

    o_tangent = tangent_w;
    o_binormal = binormal_w;
    o_normal = normal_w;
    
	float4 tmp = float4(ViewToInverseViewSpace(-pos_v.xyz), 0.0);
    o_eye = tmp;
    
    float3 eyevec = (tmp.xyz * rsqrt(dot(tmp.xyz, tmp.xyz)));
    tmp.w = pow(saturate(1.0 - dot(normal_w.xyz, eyevec.xyz)), 5.0);

	float4 spec = float4(mad(tmp.w, p_fres_coef.x, p_fres_coef.y).xxx, mad(tmp.w, (p_fres_coef.x * 10.0), 1));
	o_color_f1 = spec * state_material_specular;
    
    float4 diff = state_light1_diffuse;
    
    if (FLAGS_VERTEX_COLOR)
        diff *= FLAGS_MORPH_COLOR ? VS_MorphAttribute(a_color, a_morph_color) : a_color;

    o_color_f0 = diff;
    
    return output;
}
