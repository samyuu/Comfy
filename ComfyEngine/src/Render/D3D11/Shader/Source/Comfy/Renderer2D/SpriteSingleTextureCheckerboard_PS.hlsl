#include "../../Include/Comfy/Checkerboard.hlsl"

struct VS_OUTPUT
{
    float4 Position         : SV_POSITION;
    float2 TexCoord         : TEXCOORD0;
    float2 TexMaskCoord     : TEXCOORD1;
    float4 Color            : COLOR;
};

cbuffer SpriteConstantData : register(b0)
{
	int CB_BlendMode;
	int CB_TextureFormat;
	int CB_TextureMaskFormat;
	uint CB_TextureFormatFlags;
	float4 CB_CheckerboardSize;
};

float4 PS_main(VS_OUTPUT input) : SV_Target
{
    return input.Color * GetCheckerboardFactor(input.TexCoord, CB_CheckerboardSize.xy);
}
