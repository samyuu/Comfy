#include "../../Include/InputLayout.hlsl"
#include "../../Include/ConstantInputs.hlsl"

#define COMFY_VS
#include "../../Include/Assembly/DebugInterface.hlsl"
#include "../../Include/Assembly/TempRefactor.hlsl"

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
    float3 normal_v = ModelToViewSpace(normal_m.xyz);
    
    float4 pos_w = ModelToWorldSpace(pos_m);
    float4 pos_v = ModelToViewSpace(pos_m);
    float4 pos_c = ModelToClipSpace(pos_m);

    o_position = pos_c;
    
    if (FLAGS_LINEAR_FOG)
        o_fog = VS_GetFogFactor(pos_c);
        
    if (FLAGS_SELF_SHADOW)
        o_tex_shadow0 = VS_GetShadowTextureCoordinates(pos_w);
    
    float4 binormal_w = float4(cross(normal_w.xyz, tangent_w.xyz), 0.0) * a_tangent.w;
    
    o_tangent = tangent_w;
    o_aniso_tangent = binormal_w;
    o_binormal = binormal_w;
    o_normal = normal_w;
    
    float4 eye_w = float4(ViewToInverseViewSpace(-pos_v.xyz), 0.0);
    o_eye = eye_w;
    
    eye_w.w = rsqrt(dot(eye_w.xyz, eye_w.xyz));
    eye_w = (eye_w * eye_w.w);
    
    if ((eye_w.w = dot(eye_w.xyz, normal_w.xyz)) < 0.0)
        o_normal.xyz = mad(eye_w.xyz, (eye_w.w * -1.02), normal_w.xyz);
    
    float2 luce;
    luce.x = saturate(dot(-eye_w.xyz, p_lit_dir.xyz));
    luce.x = luce.x + pow(luce.x, 8.0);
    luce.y = mad(dot(normal_w.xyz, p_lit_dir.xyz), 1.0, 1.0);
    luce.y = saturate(luce.y * luce.y);
    luce.x *= luce.y;
    
    o_color_f1.xyz = p_fog_color.xyz;
    o_color_f1.w = luce.x * fres_coef.z;

    float2 diff;
    diff.x = pow(saturate(dot(normal_v.xyz, float3(0.0, 0.0, 1.0))), 0.4);
    diff.y = saturate(mad(dot(p_lit_dir.xyz, -eye_w.xyz), 0.5, 0.5)) * diff.x;
    diff.xy *= program_env_17.xy;
    
    o_color_f0.xyz = 0;
    o_color_f0.w = diff.x + diff.y;
    
    return output;
}
