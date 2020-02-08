#include "../Include/InputLayout.hlsl"
#include "../Include/ConstantInputs.hlsl"
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
    }
    else
    {
        MOV(tex_col, state_material_diffuse);
    }
    
    if (FLAGS_AMBIENT_TEX2D)
    {
        PS_APPLY_SAMPLE_AMBIENT_TEX_COL;
    }
    
    if (FLAGS_NORMAL_TEX2D)
    {
        TEX2D_02(_tmp2, a_tex_normal0);
        MAD(_tmp2.xy, _tmp2.wy, 2.0, -1.0);
        MOV(_tmp2.z, 0.8);
    }
    else
    {
        MOV(_tmp2, float4(0.0, 0.0, 1.0, 0.0));
    }
    
    MAD(_tmp0, a_color0.w, 2.0, -1.0);
    MUL(_tmp0, _tmp0, _tmp2);
    DP3(_tmp2.x, a_tangent, _tmp0);
    DP3(_tmp2.y, a_binormal, _tmp0);
    DP3(_tmp2.z, a_normal, _tmp0);
    MOV(_tmp2.w, 0.0);
    TEXCUBE_09(diff, _tmp2);
    DP3(_tmp1.x, a_eye, _tmp2);
    MUL(_tmp1.x, _tmp1.x, 2.0);
    MAD(_tmp1.xyz, _tmp1.x, _tmp2.xyz, -a_eye.xyz);
    
    if (FLAGS_SPECULAR_TEX2D)
    {
        if (FLAGS_REFLECTION_CUBE)
        {
            TEXCUBE_05(spec, _tmp1);
            TEX2D_03(_tmp0, a_tex_specular);
            MUL(spec.w, state_material_specular.w, state_light1_specular.w);
            MUL(spec.xyz, spec.xyz, spec.w);
            MUL(spec.xyz, spec.xyz, _tmp0.xyz);
        }
        else
        {
            MOV(_tmp0.xyz, _tmp1.xyz);
            SUB(_tmp0.w, 1.0, state_material_shininess);
            TEXCUBE_10(spec, _tmp0);
            TEXCUBE_11(_tmp2, _tmp0);
            LRP(spec.xyz, _tmp0.w, spec.xyz, _tmp2.xyz);
            TEX2D_03(_tmp0, a_tex_specular);
            MUL(spec.xyz, spec.xyz, _tmp0.xyz);
        }
    }
    else
    {
        if (FLAGS_REFLECTION_CUBE)
        {
            TEXCUBE_05(spec, _tmp1);
            MUL(spec.w, state_material_specular.w, state_light1_specular.w);
            MUL(spec.xyz, spec.xyz, spec.w);
        }
        else
        {
            MOV(_tmp0.xyz, _tmp1.xyz);
            SUB(_tmp0.w, 1.0, state_material_shininess);
            TEXCUBE_10(spec, _tmp0);
            TEXCUBE_11(_tmp2, _tmp0);
            LRP(spec.xyz, _tmp0.w, spec.xyz, _tmp2.xyz);
        }
    }
    
    MUL(spec.xyz, spec.xyz, state_lightprod1_specular.xyz);
    MAD(diff, diff, state_light1_diffuse, a_color0);
    
    if (FLAGS_SHADOW)
    {
        PS_SAMPLE_STAGE_SHADOW_MAP;
        diff *= _tmp0;
    }
    
    // MAD(o_color.xyz, diff.xyz, tex_col.xyz, spec.xyz);
    // MUL(o_color.w, diff.w, tex_col.w);
    MAD(_tmp0, diff, tex_col, spec);
    LRP(o_color.xyz, a_fogcoord.x, p_fog_color.xyz, _tmp0.xyz);
    MUL(o_color.w, diff.w, tex_col.w);
    
    PS_ALPHA_TEST;
    
#endif

    return outputColor;
}
