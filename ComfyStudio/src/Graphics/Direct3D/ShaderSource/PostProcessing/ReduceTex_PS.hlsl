#define COMFY_PS
#include "../Include/Assembly/DebugInterface.hlsl"

struct VS_OUTPUT
{
    float4 Position     : SV_POSITION;
    float2 TexCoord     : TEXCOORD0;
    float4x2 TexCoords  : TEXCOORD1;
};

cbuffer ReduceTexConstantData : register(b0)
{
    float2 CB_TexelTextureSize;
    float2 CB_TextureSize;
    bool CB_ExtractBrightness;
    bool CB_CombineBlurred;
};

SamplerState LinearTextureSampler
{
    Filter = MIN_MAG_MIP_LINEAR;
    AddressU = CLAMP;
    AddressV = CLAMP;
};

Texture2D ScreenTexture : register(t0);
Texture2D BlurredTexture1 : register(t1);
Texture2D BlurredTexture2 : register(t2);
Texture2D BlurredTexture3 : register(t3);

static const float3 YBR_COEF = { 0.35, 0.45, 0.20 };
static const float3 BloomThreshold = { 1.1, 1.1, 1.1 };

static const float3 ext_col = BloomThreshold;

float4 PS_main(VS_OUTPUT input) : SV_Target
{
    float4 outputColor;
    
    if (CB_ExtractBrightness)
    {
        TEMP col = ScreenTexture.Sample(LinearTextureSampler, input.TexCoords[0]);
        TEMP col1 = ScreenTexture.Sample(LinearTextureSampler, input.TexCoords[1]);
        TEMP col2 = ScreenTexture.Sample(LinearTextureSampler, input.TexCoords[2]);
        TEMP tmp = ScreenTexture.Sample(LinearTextureSampler, input.TexCoords[3]);
        TEMP sum;
        ADD(sum.xyz, col.xyz, col1.xyz);
        ADD(sum.xyz, sum.xyz, col2.xyz);
        ADD(sum.xyz, sum.xyz, tmp.xyz);
        MUL(sum.xyz, sum.xyz, 0.25);
        DP3(o_color.w, sum, YBR_COEF);
        MAX(sum.xyz, col.xyz, col1.xyz);
        MAX(sum.xyz, sum.xyz, col2.xyz);
        MAX(sum.xyz, sum.xyz, tmp.xyz);
        SUB(sum.xyz, sum.xyz, ext_col.xyz);
        MAX(o_color.xyz, sum.xyz, 0);
    }
    else if (CB_CombineBlurred)
    {
        float4 p_color = float4(0.15, 0.25, 0.25, 0.25);
        
        #undef a_tex0
        #undef a_tex1
        #undef a_tex2
        #undef a_tex3
        #undef a_tex4
        #define a_tex0 input.TexCoord
        #define a_tex1 input.TexCoords[0]
        #define a_tex2 input.TexCoords[1]
        #define a_tex3 input.TexCoords[2]
        #define a_tex4 input.TexCoords[3]
        
        #undef TEX2D_00
        #undef TEX2D_01
        #undef TEX2D_02
        #undef TEX2D_03
        #define TEX2D_00(result, texCoord) result = ScreenTexture.Sample(LinearTextureSampler, (texCoord).xy)
        #define TEX2D_01(result, texCoord) result = BlurredTexture1.Sample(LinearTextureSampler, (texCoord).xy)
        #define TEX2D_02(result, texCoord) result = BlurredTexture2.Sample(LinearTextureSampler, (texCoord).xy)
        #define TEX2D_03(result, texCoord) result = BlurredTexture3.Sample(LinearTextureSampler, (texCoord).xy)
        
        TEMP sum, col;
        TEMP col1, col2;
        TEX2D_03(sum, a_tex1);
        TEX2D_03(col, a_tex2);
        ADD(sum, sum, col);
        TEX2D_03(col, a_tex3);
        ADD(sum, sum, col);
        TEX2D_03(col, a_tex4);
        ADD(sum, sum, col);
        MUL(sum, sum, 0.25);
        MUL(sum, sum, p_color.w);
        TEX2D_00(col, a_tex0);
        TEX2D_01(col1, a_tex0);
        TEX2D_02(col2, a_tex0);
        MAD(sum, col, p_color.x, sum);
        MAD(sum, col1, p_color.y, sum);
        MAD(sum, col2, p_color.z, sum);
        MOV(o_color.xyz, sum.xyz);
        MOV(o_color.w, col.w);
    }
    else
    {
        float4x4 texColors;
        texColors[0] = ScreenTexture.Sample(LinearTextureSampler, input.TexCoords[0]);
        texColors[1] = ScreenTexture.Sample(LinearTextureSampler, input.TexCoords[1]);
        texColors[2] = ScreenTexture.Sample(LinearTextureSampler, input.TexCoords[2]);
        texColors[3] = ScreenTexture.Sample(LinearTextureSampler, input.TexCoords[3]);

        float4 totalColor = (texColors[0] + texColors[1] + texColors[2] + texColors[3]);
        outputColor = totalColor / 4.0;
    }
    
    return outputColor;
}
