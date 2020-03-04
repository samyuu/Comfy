#include "../InputLayout.hlsl"
#include "../ConstantInputs.hlsl"

#ifndef COMFY_VS
#define COMFY_VS
#endif

#include "InputOutput.hlsl"
#include "ProgramEnvironment.hlsl"

// NOTE: (pos)_m = model space
// NOTE: (pos)_w = world space
// NOTE: (pos)_v = view space
// NOTE: (pos)_c = clip space

// NOTE: model_mtx
float3 ModelToWorldSpace(float3 modelSpace) { return mul(modelSpace, (float3x3)CB_Object.Model); }
float4 ModelToWorldSpace(float4 modelSpace) { return mul(modelSpace, CB_Object.Model); }
// NOTE: mv
float3 ModelToViewSpace(float3 modelSpace) { return mul(modelSpace, (float3x3)CB_Object.ModelView); }
float4 ModelToViewSpace(float4 modelSpace) { return mul(modelSpace, CB_Object.ModelView); }
// NOTE: mvp
float3 ModelToClipSpace(float3 modelSpace) { return mul(modelSpace, (float3x3)CB_Object.ModelViewProjection); }
float4 ModelToClipSpace(float4 modelSpace) { return mul(modelSpace, CB_Object.ModelViewProjection); }
// NOTE: camera_mvi, used for o_eye
float3 ViewToInverseViewSpace(float3 viewSpace) { return mul((float3x3)CB_Scene.View, viewSpace); }

float2 VS_MorphAttribute(float2 attribute, float2 morphAttribute) { return mad(attribute, p_morph_weight.y, (morphAttribute * p_morph_weight.x)); }
float4 VS_MorphAttribute(float4 attribute, float4 morphAttribute) { return mad(attribute, p_morph_weight.y, (morphAttribute * p_morph_weight.x)); }

void VS_SetModelSpaceAttributes(VS_INPUT input, out float4 modelSpacePosition)
{
    modelSpacePosition = a_position;
}

void VS_SetModelSpaceAttributes(VS_INPUT input, out float4 modelSpacePosition, out float4 modelSpaceNormal)
{
    modelSpacePosition = a_position;
    modelSpaceNormal = a_normal;
}

void VS_SetModelSpaceAttributes(VS_INPUT input, out float4 modelSpacePosition, out float4 modelSpaceNormal, out float4 modelSpaceTangent)
{
    modelSpacePosition = a_position;
    modelSpaceNormal = a_normal;
    modelSpaceTangent = a_tangent;
}

void VS_SetMorphModelSpaceAttributes(VS_INPUT input, out float4 modelSpacePosition)
{
    modelSpacePosition = VS_MorphAttribute(a_position, a_morph_position);
}

void VS_SetMorphModelSpaceAttributes(VS_INPUT input, out float4 modelSpacePosition, out float4 modelSpaceNormal)
{
    modelSpacePosition = VS_MorphAttribute(a_position, a_morph_position);
    modelSpaceNormal = VS_MorphAttribute(a_normal, a_morph_normal);
}

void VS_SetMorphModelSpaceAttributes(VS_INPUT input, out float4 modelSpacePosition, out float4 modelSpaceNormal, out float4 modelSpaceTangent)
{
    modelSpacePosition = VS_MorphAttribute(a_position, a_morph_position);
    modelSpaceNormal = VS_MorphAttribute(a_normal, a_morph_normal);
    modelSpaceTangent = VS_MorphAttribute(a_tangent, a_morph_tangent);
}

float2 VS_TransformTextureCoordinates(float2 texCoord, matrix transform)
{
    return float2(dot(transform[0], float4(texCoord, 0.0, 1.0)), dot(transform[1], float4(texCoord, 0.0, 1.0)));
}

void VS_SetTransformTextureCoordinates(VS_INPUT input, out float2 texCoord0)
{
    texCoord0 = VS_TransformTextureCoordinates(a_tex0.xy, state_matrix_texture0);
}

void VS_SetTransformTextureCoordinates(VS_INPUT input, out float2 texCoord0, out float2 texCoord1)
{
    texCoord0 = VS_TransformTextureCoordinates(a_tex0.xy, state_matrix_texture0);
    texCoord1 = VS_TransformTextureCoordinates(a_tex1.xy, state_matrix_texture1);
}

void VS_SetMorphTransformTextureCoordinates(VS_INPUT input, out float2 texCoord0)
{
    texCoord0 = VS_TransformTextureCoordinates(VS_MorphAttribute(a_tex0, a_morph_texcoord).xy, state_matrix_texture0);
}

void VS_SetMorphTransformTextureCoordinates(VS_INPUT input, out float2 texCoord0, out float2 texCoord1)
{
    texCoord0 = VS_TransformTextureCoordinates(VS_MorphAttribute(a_tex0, a_morph_texcoord).xy, state_matrix_texture0);
    texCoord1 = VS_TransformTextureCoordinates(VS_MorphAttribute(a_tex1, a_morph_texcoord1).xy, state_matrix_texture1);
}

// NOTE: Assigned to o_fog
float VS_GetFogFactor(float4 clipSpacePosition)
{
    return saturate((clipSpacePosition.z - state_fog_params.y) * state_fog_params.w) * state_fog_params.x;
}

float4 VS_GetShadowTextureCoordinates(float4 worldSpacePosition)
{
    // return mul(worldSpacePosition, CB_Scene.LightSpace);
    float4 position = mul(worldSpacePosition, CB_Scene.LightSpace);
    
    // return float4((float2(+position.x, -position.y) / 2.0f + 0.5f).xy, position.z, 1.0);
    return float4((float2(+position.x, -position.y) / 2.0f + 0.5f).xy, position.z / position.w, 1.0);
}

// NOTE: Assigned to o_eye / eye_w
float3 VS_GetWorldEye(float4 viewSpacePosition)
{
	return ViewToInverseViewSpace(-(viewSpacePosition * rsqrt(dot(viewSpacePosition.xyz, viewSpacePosition.xyz))).xyz);
}

float3 GetIrradiance(float4 worldSpaceNormal)
{
    return float3(
        dot(mul(worldSpaceNormal, irrad_r), worldSpaceNormal),
        dot(mul(worldSpaceNormal, irrad_g), worldSpaceNormal),
        dot(mul(worldSpaceNormal, irrad_b), worldSpaceNormal));
}

float3 GetIrradiance(float3 worldSpaceNormal)
{
    return GetIrradiance(float4(worldSpaceNormal, 1.0));
}
