#include "../Include/InputLayout.hlsl"
#include "../Include/ConstantInputs.hlsl"

#define COMFY_VS
#include "../Include/Assembly/DebugInterface.hlsl"
#include "../Include/Assembly/TempRefactor.hlsl"

VS_OUTPUT_SHADOW VS_main(VS_INPUT_SHADOW input)
{
    VS_OUTPUT_SHADOW output;
    
    float4 pos_m;
    if (FLAGS_MORPH)
    {
        VS_SetMorphModelSpaceAttributes(input, pos_m);
        VS_SetMorphTransformTextureCoordinates(input, o_tex0);
    }
    else
    {
        VS_SetModelSpaceAttributes(input, pos_m);
        VS_SetTransformTextureCoordinates(input, o_tex0);
    }
    
    float4 pos_c = ModelToClipSpace(pos_m);
    o_position = pos_c;
    
    return output;
}
