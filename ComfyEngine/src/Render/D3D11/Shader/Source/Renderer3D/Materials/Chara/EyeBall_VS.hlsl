#include "../../Include/InputLayout.hlsl"
#include "../../Include/ConstantInputs.hlsl"

#define COMFY_VS
#include "../../Include/Assembly/DebugInterface.hlsl"
#include "../../Include/Assembly/TempRefactor.hlsl"

VS_OUTPUT VS_main(VS_INPUT input)
{
    VS_OUTPUT output = (VS_OUTPUT)0;
    
    output.Position = ModelToClipSpace(input.Position);
    
    return output;
}
