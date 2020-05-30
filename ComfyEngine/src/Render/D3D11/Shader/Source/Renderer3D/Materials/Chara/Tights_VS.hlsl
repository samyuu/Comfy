#include "../../Include/InputLayout.hlsl"
#include "../../Include/ConstantInputs.hlsl"

#define COMFY_VS
#define ARB_PROGRAM_ACCURATE 0
#include "../../Include/Assembly/DebugInterface.hlsl"

VS_OUTPUT VS_main(VS_INPUT input)
{
    VS_OUTPUT output = (VS_OUTPUT)0;
    
#if ARB_PROGRAM_ACCURATE
    // TODO: ...
#else
    output.Position = mul(input.Position, CB_Object.ModelViewProjection);
#endif
    
    return output;
}
