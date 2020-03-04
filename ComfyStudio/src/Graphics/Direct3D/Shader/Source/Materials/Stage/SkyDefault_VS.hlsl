#include "../Include/InputLayout.hlsl"
#include "../Include/ConstantInputs.hlsl"

#define COMFY_VS
#include "../Include/Assembly/DebugInterface.hlsl"
#include "../Include/Assembly/TempRefactor.hlsl"

VS_OUTPUT VS_main(VS_INPUT input)
{
    VS_OUTPUT output = (VS_OUTPUT)0;
    
    float4 pos_m, normal_m;
    if (FLAGS_MORPH)
    {
        VS_SetMorphModelSpaceAttributes(input, pos_m, normal_m);
        VS_SetMorphTransformTextureCoordinates(input, o_tex0, o_tex1);
    }
    else
    {
        VS_SetModelSpaceAttributes(input, pos_m, normal_m);
        VS_SetTransformTextureCoordinates(input, o_tex0, o_tex1);
    }

    float4 pos_c = ModelToClipSpace(pos_m);
    o_position = pos_c;
    
    if (FLAGS_VERTEX_COLOR)
        o_color_f0 = FLAGS_MORPH_COLOR ? VS_MorphAttribute(a_color, a_morph_color) : a_color;
    else
        o_color_f0 = float4(1.0, 1.0, 1.0, 1.0);
    
    return output;
}
