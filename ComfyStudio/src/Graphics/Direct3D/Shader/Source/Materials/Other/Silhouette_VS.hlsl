#include "../Include/InputLayout.hlsl"
#include "../Include/ConstantInputs.hlsl"

#define COMFY_VS
#include "../Include/Assembly/DebugInterface.hlsl"
#include "../Include/Assembly/TempRefactor.hlsl"

VS_OUTPUT_SHADOW VS_main(VS_INPUT_SHADOW input)
{
    VS_OUTPUT_SHADOW output;
    
    if (FLAGS_MORPH)
    {
        output.Position = mul(VS_MorphAttribute(input.Position, input.MorphPosition), CB_Object.ModelViewProjection);
        output.TexCoord = VS_TransformTextureCoordinates(VS_MorphAttribute(input.TexCoord, input.MorphTexCoord), CB_Object.Material.DiffuseTextureTransform);
    }
    else
    {
        output.Position = mul(input.Position, CB_Object.ModelViewProjection);
        output.TexCoord = VS_TransformTextureCoordinates(input.TexCoord, CB_Object.Material.DiffuseTextureTransform);
    }
    
    return output;
}
