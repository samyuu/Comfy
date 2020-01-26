#include "../Include/InputLayouts.hlsl"
#include "../Include/ConstantInputs.hlsl"
#include "../Include/Common.hlsl"
#include "../Include/TextureInputs.hlsl"

#define COMFY_PS
#define ARB_PROGRAM_ACCURATE 1
#include "../Include/Assembly/DebugInterface.hlsl"

float4 PS_main(VS_OUTPUT input) : SV_Target
{
    float4 outputColor;
    
#if ARB_PROGRAM_ACCURATE
    
    TEMP _tmp0, _tmp1, _tmp2, diff, spec, tex_col;
    
    if (FLAGS_DIFFUSE_TEX2D)
    {
        TEX2D_00(tex_col, a_tex_color0);
        
        if (FLAGS_AMBIENT_TEX2D)
        {
            PS_APPLY_SAMPLE_AMBIENT_TEX_COL;
        }
    }
    else
    {
        MOV(tex_col, state_material_diffuse);
    }
    
    if (FLAGS_SPECULAR_TEX2D)
    {
        if (FLAGS_REFLECTION_CUBE)
        {
            TEXCUBE_05(spec, a_reflect);
            TEX2D_03(_tmp0, a_tex_specular);
            MUL(spec.w, state_material_specular.w, state_light1_specular.w);
            MUL(spec.xyz, spec.xyz, spec.w);
            MUL(spec.xyz, spec.xyz, _tmp0.xyz);
            MUL(spec.xyz, spec.xyz, state_lightprod1_specular.xyz);
        }
        else
        {
            TEX2D_03(_tmp0, a_tex_specular);
            MUL(spec.xyz, a_color1.xyz, _tmp0.xyz);
        }
    }
    else
    {
        if (FLAGS_REFLECTION_CUBE)
        {
            TEXCUBE_05(spec, a_reflect);
            MUL(spec.w, state_material_specular.w, state_light1_specular.w);
            MUL(spec.xyz, spec.xyz, spec.w);
        }
        else
        {
            TEXCUBE_10(spec, a_reflect);
            TEXCUBE_11(_tmp2, a_reflect);
            LRP(spec.xyz, a_reflect.w, spec.xyz, _tmp2.xyz);
        }
        MUL(spec.xyz, spec.xyz, state_lightprod1_specular.xyz);
    }

    // MAD(o_color.xyz, a_color0.xyz, tex_col.xyz, spec.xyz);
    // MUL(o_color.w, a_color0.w, tex_col.w);
    MAD(_tmp0, a_color0, tex_col, spec);
    LRP(o_color.xyz, a_fogcoord.x, p_fog_color.xyz, _tmp0.xyz);
    MUL(o_color.w, a_color0.w, tex_col.w);
    
    PS_ALPHA_TEST;
    
#endif

    return outputColor;
}
