#ifndef INPUTLAYOUTS_HLSL
#define INPUTLAYOUTS_HLSL

struct VS_INPUT_SHADOW
{
    float4 Position                 : POSITION0;
    float2 TexCoord                 : TEXCOORD0;
    float4 BoneWeight               : BONE_WEIGHT0;
    float4 BoneIndex                : BONE_INDEX0;
    float4 MorphPosition            : POSITION1;
    float2 MorphTexCoord            : TEXCOORD4;
};

struct VS_OUTPUT_SHADOW
{
    float4 Position                 : SV_POSITION;
    float2 TexCoord                 : TEXCOORD0;
};

#endif /* INPUTLAYOUTS_HLSL */
