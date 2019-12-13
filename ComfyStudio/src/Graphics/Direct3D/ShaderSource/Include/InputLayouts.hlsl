// NOTE: No include guard on purpose, for different kind of layouts #defines could be used

struct VS_INPUT
{
    float4 Position         : POSITION;
    float3 Normal           : NORMAL;
    float4 Tangent          : TANGENT;
    float2 TexCoord         : TEXCOORD0;
    float2 TexCoordAmbient  : TEXCOORD1;
    float4 Color            : COLOR;
};

struct VS_OUTPUT
{
    float4 Position         : SV_POSITION;
    float4 Normal           : NORMAL;
    float4 Tangent          : TANGENT;
    float4 Binormal         : BINORMAL;
    float4 EyeDirection     : EYE_DIRECTION;
    float2 TexCoord         : TEXCOORD0;
    float2 TexCoordAmbient  : TEXCOORD1;
    float4 TexCoordShadow   : TEXCOORD2;
    float4 Color            : COLOR0;
    float4 ColorSecondary   : COLOR1;
    float4 Reflection       : REFLECT;
};
