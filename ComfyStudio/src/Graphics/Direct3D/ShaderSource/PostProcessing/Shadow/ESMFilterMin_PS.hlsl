#define COMFY_PS

struct VS_OUTPUT
{
    float4 Position     : SV_POSITION;
    float2 TexCoord     : TEXCOORD0;
};

cbuffer ESMFilterConstantData : register(b4)
{
    float4 CB_Coefficients[2];
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
    float dep0, dep1;
    dep0 = DepthTexture.Sample(LinearTextureSampler, mad(CB_TextureStep, float2(+1, +1), input.TexCoord));
    dep1 = DepthTexture.Sample(LinearTextureSampler, mad(CB_TextureStep, float2(-1, +1), input.TexCoord));
    dep0 = min(dep0, dep1);
    dep1 = DepthTexture.Sample(LinearTextureSampler, mad(CB_TextureStep, float2(+1, -1), input.TexCoord));
    dep0 = min(dep0, dep1);
    dep1 = DepthTexture.Sample(LinearTextureSampler, mad(CB_TextureStep, float2(-1, -1), input.TexCoord));
    dep0 = min(dep0, dep1);
    return dep0;
}
