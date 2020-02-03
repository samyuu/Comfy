#include "../Include/InputLayoutShadow.hlsl"
#include "../Include/ConstantInputs.hlsl"

float2 VS_MorphAttribute(float2 attribute, float2 morphAttribute) { return mad(attribute, CB_MorphWeight.y, (morphAttribute * CB_MorphWeight.x)); }
float4 VS_MorphAttribute(float4 attribute, float4 morphAttribute) { return mad(attribute, CB_MorphWeight.y, (morphAttribute * CB_MorphWeight.x)); }
float2 VS_TransformTextureCoordinates(float2 texCoord, matrix transform) { return float2(dot(transform[0], float4(texCoord, 0.0, 1.0)), dot(transform[1], float4(texCoord, 0.0, 1.0))); }

VS_OUTPUT_SHADOW VS_main(VS_INPUT_SHADOW input)
{
    VS_OUTPUT_SHADOW output;
    
    if (FLAGS_MORPH)
    {
        output.Position = mul(VS_MorphAttribute(input.Position, input.MorphPosition), CB_ModelViewProjection);
        output.TexCoord = VS_TransformTextureCoordinates(VS_MorphAttribute(input.TexCoord, input.MorphTexCoord), CB_Material.DiffuseTextureTransform);
    }
    else
    {
        output.Position = mul(input.Position, CB_ModelViewProjection);
        output.TexCoord = VS_TransformTextureCoordinates(input.TexCoord, CB_Material.DiffuseTextureTransform);
    }
    
    return output;
}
