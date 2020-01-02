#include "../Include/UncompressRGTC.hlsl"
#include "../Include/CubeMapCommon.hlsl"

struct VS_OUTPUT
{
    float4 Position : SV_POSITION;
    float4 Color    : COLOR;
    float2 TexCoord : TEXCOORD;
};

cbuffer CustomConstantData : register(b0)
{
    int CB_RenderCubeMap;
    int CB_CubeMapFace;
    int CB_CubeMapUnwrapNet;
    int CB_DecompressRGTC;
};

SamplerState TextureSampler     : register(s0);

Texture2D SpriteTexture         : register(t0);
TextureCube SpriteCubeTexture   : register(t1);

float4 PS_main(VS_OUTPUT input) : SV_Target
{
    float4 textureColor;
    
    if (CB_RenderCubeMap)
    {
        if (CB_CubeMapUnwrapNet)
            textureColor = SpriteCubeTexture.Sample(TextureSampler, GetCubeMapNetTextureCoordinates(input.TexCoord));
        else if (CB_CubeMapFace >= 0)
            textureColor = SpriteCubeTexture.Sample(TextureSampler, GetCubeMapFaceTextureCoordinates(input.TexCoord, CB_CubeMapFace));
        else
            textureColor = SpriteCubeTexture.Sample(TextureSampler, GetCubeMapTextureCoordinates(input.TexCoord));
    }
    else if (CB_DecompressRGTC)
    {
        textureColor = UncompressRGTC_RGBA(SpriteTexture, TextureSampler, input.TexCoord);
    }
    else
    {
        textureColor = SpriteTexture.Sample(TextureSampler, input.TexCoord);
    }
    
    return textureColor * input.Color;
}
