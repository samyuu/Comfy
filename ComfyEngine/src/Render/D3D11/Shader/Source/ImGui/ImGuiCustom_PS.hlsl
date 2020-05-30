#include "../Common/CubeMapNet.hlsl"
#include "../Common/YCbCr.hlsl"

struct VS_OUTPUT
{
    float4 Position : SV_POSITION;
    float4 Color    : COLOR;
    float2 TexCoord : TEXCOORD;
};

cbuffer CustomConstantData : register(b0)
{
    int CB_DecompressRGTC;
    int CB_RenderCubeMap;
    int CB_CubeMapFace;
    int CB_CubeMapUnwrapNet;
    int CB_CubeMapMipLevel;
    int CB_Padding[3];
};

SamplerState TextureSampler : register(s0);

Texture2D SpriteTexture : register(t0);
TextureCube SpriteCubeTexture : register(t1);

float3 CB_GetCubeMapTextureCoordinates(const float2 texCoord)
{
    if (CB_CubeMapUnwrapNet)
        return GetCubeMapNetTextureCoordinates(texCoord);
    else if (CB_CubeMapFace >= 0)
        return GetCubeMapFaceTextureCoordinates(texCoord, CB_CubeMapFace);
    else
        return GetCubeMapTextureCoordinates(texCoord);
}

float4 PS_main(VS_OUTPUT input) : SV_Target
{
    float4 textureColor;
    
    if (CB_RenderCubeMap)
    {
        textureColor = SpriteCubeTexture.SampleLevel(TextureSampler, CB_GetCubeMapTextureCoordinates(input.TexCoord), CB_CubeMapMipLevel);
    }
    else if (CB_DecompressRGTC)
    {
        textureColor = RGTC2_ConvertYACbCrToRGBA(SpriteTexture, TextureSampler, input.TexCoord);
    }
    else
    {
        textureColor = SpriteTexture.Sample(TextureSampler, input.TexCoord);
    }
    
    return textureColor * input.Color;
}
