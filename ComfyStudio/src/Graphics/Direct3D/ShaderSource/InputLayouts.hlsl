// NOTE: No include guard on purpose, for different kind of layouts #defines could be used

struct VS_INPUT
{
    float3 Position         : POSITION;
    float3 Normal           : NORMAL;
    float2 TexCoord         : TEXCOORD0;
    float2 ShadowTexCoord   : TEXCOORD1;
    float4 Color            : COLOR;
};

struct VS_OUTPUT
{
    float4 Position         : SV_POSITION;
    float3 Normal           : NORMAL;
    float2 TexCoord         : TEXCOORD0;
    float2 ShadowTexCoord   : TEXCOORD1;
    float4 Color            : COLOR0;
    float4 Reflection       : REFLECT;
};
