// NOTE: No include guard on purpose, for different kind of layouts #defines could be used

struct VS_INPUT
{
    float4 Position                 : POSITION0;
    float3 Normal                   : NORMAL0;
    float4 Tangent                  : TANGENT0;
    float2 TexCoord                 : TEXCOORD0;
    float2 TexCoordAmbient          : TEXCOORD1;
    float4 Color                    : COLOR0;
    float4 ColorSecondary           : COLOR1;
    float4 BoneWeight               : BONE_WEIGHT0;
    float4 BoneIndex                : BONE_INDEX0;
    
    float4 MorphPosition            : POSITION1;
    float3 MorphNormal              : NORMAL1;
    float4 MorphTangent             : TANGENT1;
    float2 MorphTexCoord            : TEXCOORD4;
    float2 MorphTexCoordAmbient     : TEXCOORD5;
    float4 MorphColor               : COLOR2;
    float4 MorphColorSecondary      : COLOR3;
};

struct VS_OUTPUT
{
    float4 Position                 : SV_POSITION;
    float4 Normal                   : NORMAL0;
    float4 Tangent                  : TANGENT0;
    float4 AnisoTangent             : TANGENT1;
    float4 Binormal                 : BINORMAL;
    float4 EyeDirection             : EYE_DIRECTION;
    float2 TexCoord                 : TEXCOORD0;
    float2 TexCoordAmbient          : TEXCOORD1;
    float4 TexCoordShadow           : TEXCOORD2;
    float4 Color                    : COLOR0;
    float4 ColorSecondary           : COLOR1;
    float4 Reflection               : REFLECT;
    float1 FogFactor                : FOG;
    float4 WorldPosition            : POSITION;
};
