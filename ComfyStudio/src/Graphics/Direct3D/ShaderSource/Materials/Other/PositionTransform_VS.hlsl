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
    }
    else
    {
        VS_SetModelSpaceAttributes(input, pos_m);
    }
    
    float4 pos_c = ModelToClipSpace(pos_m);
    o_position = pos_c;
    
    return output;
}
