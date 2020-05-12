#define COMFY_PS
#include "../../Include/Assembly/DebugInterface.hlsl"

struct VS_OUTPUT
{
    float4 Position : SV_POSITION;
    float2 TexCoord : TEXCOORD;
};

cbuffer PPGaussTexConstantData : register(b7)
{
    float2 CB_TexelTextureSize;
    float2 CB_TextureSize;
    float4 CB_TextureOffsets;
    int CB_FinalPass;
};

cbuffer PPGaussCoefConstantData : register(b8)
{
    float4 CB_Coefficients[8];
};

SamplerState LinearTextureSampler
{
    Filter = MIN_MAG_MIP_LINEAR;
    AddressU = CLAMP;
    AddressV = CLAMP;
};

Texture2D ScreenTexture : register(t0);

float4 PS_main(VS_OUTPUT input) : SV_Target
{
    float4 outputColor;

    #define tex_wh CB_TexelTextureSize
    #define tex_ofs CB_TextureOffsets
    #define TEX2D_XX(result, texCoord) result = ScreenTexture.Sample(LinearTextureSampler, (texCoord).xy)
    #define coef CB_Coefficients
    
    if (!CB_FinalPass)
    {
        TEMP col0, col1;
        TEMP sum;
        TEMP step, stex0, stex1;
        TEX2D_XX(sum, a_tex0);
        MUL(sum.xyz, sum.xyz, coef[0].xyz);
        MUL(step.xy, tex_wh, tex_ofs.zw);
        MAD(stex0.xy, tex_wh.xy, tex_ofs.xy, a_tex0.xy);
        MAD(stex1.xy, tex_wh.xy, -tex_ofs.xy, a_tex0.xy);
        TEX2D_XX(col0, stex0);
        TEX2D_XX(col1, stex1);
        MAD(sum.xyz, col0.xyz, coef[1].xyz, sum.xyz);
        MAD(sum.xyz, col1.xyz, coef[1].xyz, sum.xyz);
        ADD(stex0.xy, stex0.xy, step.xy);
        ADD(stex1.xy, stex1.xy, -step.xy);
        TEX2D_XX(col0, stex0);
        TEX2D_XX(col1, stex1);
        MAD(sum.xyz, col0.xyz, coef[2].xyz, sum.xyz);
        MAD(sum.xyz, col1.xyz, coef[2].xyz, sum.xyz);
        ADD(stex0.xy, stex0.xy, step.xy);
        ADD(stex1.xy, stex1.xy, -step.xy);
        TEX2D_XX(col0, stex0);
        TEX2D_XX(col1, stex1);
        MAD(sum.xyz, col0.xyz, coef[3].xyz, sum.xyz);
        MAD(sum.xyz, col1.xyz, coef[3].xyz, sum.xyz);
        ADD(stex0.xy, stex0.xy, step.xy);
        ADD(stex1.xy, stex1.xy, -step.xy);
        TEX2D_XX(col0, stex0);
        TEX2D_XX(col1, stex1);
        MAD(sum.xyz, col0.xyz, coef[4].xyz, sum.xyz);
        MAD(sum.xyz, col1.xyz, coef[4].xyz, sum.xyz);
        ADD(stex0.xy, stex0.xy, step.xy);
        ADD(stex1.xy, stex1.xy, -step.xy);
        TEX2D_XX(col0, stex0);
        TEX2D_XX(col1, stex1);
        MAD(sum.xyz, col0.xyz, coef[5].xyz, sum.xyz);
        MAD(sum.xyz, col1.xyz, coef[5].xyz, sum.xyz);
        ADD(stex0.xy, stex0.xy, step.xy);
        ADD(stex1.xy, stex1.xy, -step.xy);
        TEX2D_XX(col0, stex0);
        TEX2D_XX(col1, stex1);
        MAD(sum.xyz, col0.xyz, coef[6].xyz, sum.xyz);
        MAD(sum.xyz, col1.xyz, coef[6].xyz, sum.xyz);
        MOV(o_color, sum);
    }
    else
    {
        const float4 p_color = FLOAT4_ONE;

        float2 a_tex1, a_tex2, a_tex3, a_tex4;
        MAD(a_tex1, float2(-0.5, -0.5), tex_wh.xy, input.TexCoord);
        MAD(a_tex2, float2(+0.5, -0.5), tex_wh.xy, input.TexCoord);
        MAD(a_tex3, float2(-0.5, +0.5), tex_wh.xy, input.TexCoord);
        MAD(a_tex4, float2(+0.5, +0.5), tex_wh.xy, input.TexCoord);
        
        TEMP col0, col1;
        TEMP sum;
        TEX2D_XX(sum, a_tex1);
        TEX2D_XX(col0, a_tex2);
        ADD(sum, sum, col0);
        TEX2D_XX(col0, a_tex3);
        ADD(sum, sum, col0);
        TEX2D_XX(col0, a_tex4);
        ADD(sum, sum, col0);
        MUL(sum, sum, 0.25);
        MUL(o_color, sum, p_color);
    }
    return outputColor;
}
