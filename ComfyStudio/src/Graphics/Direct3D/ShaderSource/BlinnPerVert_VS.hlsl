#include "Include/InputLayouts.hlsl"
#include "Include/ConstantInputs.hlsl"
#include "Include/Common.hlsl"

VS_OUTPUT VS_main(VS_INPUT input)
{
    VS_OUTPUT output = (VS_OUTPUT)0;
    
    output.Position = mul(input.Position, CB_Model);
    output.Position = mul(output.Position, CB_Scene.ViewProjection);
    
    const float4 normal = float4(input.Normal, 1.0);
    output.Normal = mul(normal, CB_Model).xyz;

    if (CB_ShaderFlags & ShaderFlags_DiffuseTexture)
        output.TexCoord = TransformTextureCoordinates(input.TexCoord, CB_Material.DiffuseTextureTransform);
    
    if (CB_ShaderFlags & ShaderFlags_AmbientTexture)
        output.TexCoordAmbient = TransformTextureCoordinates(input.TexCoordAmbient, CB_Material.AmbientTextureTransform);
    
    float3 irradiance = GetIrradience(CB_Scene, normal);
    float3 diffuse = GetDiffuseLight(CB_Scene.StageLight, normal.xyz);
    
    output.Color = float4(mad(diffuse, CB_Scene.LightColor.rgb, irradiance * CB_Scene.StageLight.Diffuse.rgb), 1.0);
 
	float3 pos_v = mul(transpose(CB_Scene.View), input.Position).xyz;
   
	float3 eye_w = mul((float3x3)CB_Scene.View, -(rsqrt(dot(pos_v, pos_v)) * pos_v)).xyz;

	float3 lit_dir_w = CB_Scene.StageLight.Direction.xyz;

	float4 half_w = float4(lit_dir_w + eye_w, 0.0);
	half_w.w = rsqrt(dot(half_w, half_w));
	half_w.xyz *= half_w.w;

	float4 lc;
	lc.y = saturate(dot(output.Normal, lit_dir_w));
	lc.z = saturate(dot(output.Normal, half_w.xyz));
	lc.w = mad(CB_Material.Shininess.x, 120.0, 16.0);
	lc.z = pow(lc.z, lc.w);
	lc.xw = 1.0;
	
	float3 diff = mad(lc.y, CB_Scene.LightColor.rgb, diffuse.rgb);
	float3 spec = lc.z * CB_Scene.StageLight.Specular.rgb;

    output.ColorSecondary = float4(spec * CB_Material.Specular.rgb, 1.0);
    
    if (CB_ShaderFlags & ShaderFlags_VertexColor)
        output.Color *= input.Color;
    
    if (CB_ShaderFlags & ShaderFlags_CubeMapReflection)
    {
        float3 eyeDirection = normalize(input.Position.xyz - CB_Scene.EyePosition.xyz);
        output.Reflection.xyz = reflect(eyeDirection, output.Normal);
        output.Reflection.w = 1.0 - CB_Material.Shininess;
    }
    
    return output;
}
