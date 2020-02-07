#define COMFY_PS
#include "../Include/Assembly/DebugInterface.hlsl"

struct VS_OUTPUT
{
    float4 Position     : SV_POSITION;
    float2 TexCoord     : TEXCOORD0;
};

cbuffer ESMFilterConstantData : register(b3)
{
    float4 CB_Coefficient[2];
    float2 CB_TextureStep;
    float2 CB_FarTexelOffset;
    int CB_PassIndex;
};

SamplerState LinearTextureSampler
{
    Filter = MIN_MAG_MIP_LINEAR;
    AddressU = CLAMP;
    AddressV = CLAMP;
};

Texture2D<float> DepthTexture : register(t0);

float4 PS_main(VS_OUTPUT input) : SV_Target
{
    float4 outputColor;
    
    #define TEX(result, texCoord) result = DepthTexture.Sample(LinearTextureSampler, (texCoord).xy);
    
    TEMP sum;
    TEMP dep0, dep1;
    TEMP stex0, stex1;
    MOV(sum, float4(0.000011, 0.000010, 0, 0));
    TEX(dep0, a_tex0);
    MOV(o_color, dep0.x);
    MOV(sum.w, 0.999);
    ADD(sum.z, dep0.x, CB_FarTexelOffset.y);
    
    if ((dep0.w = dep0.x - sum.w) > 0.0)
    { 
        ADD(stex0.xy, CB_TextureStep, a_tex0);
        ADD(stex1.xy,-CB_TextureStep, a_tex0);
        TEX(dep0, stex0);
        TEX(dep1, stex1);
        
        if ((dep0.w = dep0.x - sum.w) < 0.0)
        {
            MAD(sum.x, dep0.x, 1, sum.x);
            ADD(sum.y, 1, sum.y);
        }
        
        if ((dep1.w = dep1.x - sum.w) < 0.0)
        {
            MAD(sum.x, dep1.x, 1, sum.x);
            ADD(sum.y, 1, sum.y);
        }
        
        RCP(sum.y, sum.y);
        MAD(o_color, sum.x, sum.y, CB_FarTexelOffset.x);
        RET;
    }
    
    MAD(sum.x, dep0.x, CB_Coefficient[0].x, sum.x);
    ADD(sum.y, CB_Coefficient[0].x, sum.y);
    ADD(stex0.xy, CB_TextureStep, a_tex0);
    ADD(stex1.xy,-CB_TextureStep, a_tex0);
    TEX(dep0, stex0);
    TEX(dep1, stex1);
    
    if ((dep0.w = dep0.x - sum.w) > 0.0)
        MOV(dep0.x, sum.z);
    
    MAD(sum.x, dep0.x, CB_Coefficient[0].y, sum.x);
    ADD(sum.y, CB_Coefficient[0].y, sum.y);
    
    if ((dep1.w = dep1.x - sum.w) > 0.0)
        MOV(dep1.x, sum.z);
    
    MAD(sum.x, dep1.x, CB_Coefficient[0].y, sum.x);
    ADD(sum.y, CB_Coefficient[0].y, sum.y);
    ADD(stex0.xy, CB_TextureStep.xy, stex0.xy);
    ADD(stex1.xy,-CB_TextureStep.xy, stex1.xy);
    TEX(dep0, stex0);
    TEX(dep1, stex1);
    
    if ((dep0.w = dep0.x - sum.w) > 0.0)
        MOV(dep0.x, sum.z);
    
    MAD(sum.x, dep0.x, CB_Coefficient[0].z, sum.x);
    ADD(sum.y, CB_Coefficient[0].z, sum.y);
    
    if ((dep1.w = dep1.x - sum.w) > 0.0)
        MOV(dep1.x, sum.z);
    
    MAD(sum.x, dep1.x, CB_Coefficient[0].z, sum.x);
    ADD(sum.y, CB_Coefficient[0].z, sum.y);
    ADD(stex0.xy, CB_TextureStep.xy, stex0.xy);
    ADD(stex1.xy,-CB_TextureStep.xy, stex1.xy);
    TEX(dep0, stex0);
    TEX(dep1, stex1);
    
    if ((dep0.w = dep0.x - sum.w) > 0.0)
        MOV(dep0.x, sum.z);
    
    MAD(sum.x, dep0.x, CB_Coefficient[0].w, sum.x);
    ADD(sum.y, CB_Coefficient[0].w, sum.y);
    
    if ((dep1.w = dep1.x - sum.w) > 0.0)
        MOV(dep1.x, sum.z);
    
    MAD(sum.x, dep1.x, CB_Coefficient[0].w, sum.x);
    ADD(sum.y, CB_Coefficient[0].w, sum.y);
    ADD(stex0.xy, CB_TextureStep.xy, stex0.xy);
    ADD(stex1.xy,-CB_TextureStep.xy, stex1.xy);
    TEX(dep0, stex0);
    TEX(dep1, stex1);
    
    if ((dep0.w = dep0.x - sum.w) > 0.0)
        MOV(dep0.x, sum.z);
    
    MAD(sum.x, dep0.x, CB_Coefficient[1].x, sum.x);
    ADD(sum.y, CB_Coefficient[1].x, sum.y);
    
    if ((dep1.w = dep1.x - sum.w) > 0.0)
        MOV(dep1.x, sum.z);
    
    MAD(sum.x, dep1.x, CB_Coefficient[1].x, sum.x);
    ADD(sum.y, CB_Coefficient[1].x, sum.y);
    ADD(stex0.xy, CB_TextureStep.xy, stex0.xy);
    ADD(stex1.xy,-CB_TextureStep.xy, stex1.xy);
    TEX(dep0, stex0);
    TEX(dep1, stex1);
    
    if ((dep0.w = dep0.x - sum.w) > 0.0)
        MOV(dep0.x, sum.z);
    
    MAD(sum.x, dep0.x, CB_Coefficient[1].y, sum.x);
    ADD(sum.y, CB_Coefficient[1].y, sum.y);
    
    if ((dep1.w = dep1.x - sum.w) > 0.0)
        MOV(dep1.x, sum.z);
    
    MAD(sum.x, dep1.x, CB_Coefficient[1].y, sum.x);
    ADD(sum.y, CB_Coefficient[1].y, sum.y);
    ADD(stex0.xy, CB_TextureStep.xy, stex0.xy);
    ADD(stex1.xy,-CB_TextureStep.xy, stex1.xy);
    TEX(dep0, stex0);
    TEX(dep1, stex1);
    
    if ((dep0.w = dep0.x - sum.w) > 0.0)
        MOV(dep0.x, sum.z);
    
    MAD(sum.x, dep0.x, CB_Coefficient[1].z, sum.x);
    ADD(sum.y, CB_Coefficient[1].z, sum.y);
    
    if ((dep1.w = dep1.x - sum.w) > 0.0)
        MOV(dep1.x, sum.z);
    
    MAD(sum.x, dep1.x, CB_Coefficient[1].z, sum.x);
    ADD(sum.y, CB_Coefficient[1].z, sum.y);
    ADD(stex0.xy, CB_TextureStep.xy, stex0.xy);
    ADD(stex1.xy,-CB_TextureStep.xy, stex1.xy);
    TEX(dep0, stex0);
    TEX(dep1, stex1);
    
    if ((dep0.w = dep0.x - sum.w) > 0.0)
        MOV(dep0.x, sum.z);
    
    MAD(sum.x, dep0.x, CB_Coefficient[1].w, sum.x);
    ADD(sum.y, CB_Coefficient[1].w, sum.y);
    
    if ((dep1.w = dep1.x - sum.w) > 0.0)
        MOV(dep1.x, sum.z);
    
    MAD(sum.x, dep1.x, CB_Coefficient[1].w, sum.x);
    ADD(sum.y, CB_Coefficient[1].w, sum.y);
    RCP(sum.y, sum.y);
    MUL(o_color, sum.x, sum.y);
    
    return outputColor;
}
