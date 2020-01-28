#include "../Include/Common.hlsl"

struct VS_INPUT
{
    uint VertexID : SV_VERTEXID;
};

struct VS_OUTPUT
{
    float4 Position     : SV_POSITION;
    float2 TexCoord     : TEXCOORD0;
    float4x2 TexCoords  : TEXCOORD1;
};

cbuffer ReduceTexConstantData : register(b6)
{
    float2 CB_TexelTextureSize;
    float2 CB_TextureSize;
    bool CB_ExtractBrightness;
    bool CB_CombineBlurred;
};

VS_OUTPUT VS_main(VS_INPUT input)
{
    VS_OUTPUT output;

    float2 texCoord = ScreenTexCoordFromVertexID(input.VertexID);
    output.Position = ScreenPositionFromTexCoord(texCoord);
    
    output.TexCoord = texCoord;
    output.TexCoords[0] = mad(float2(-1.0, -1.0), CB_TexelTextureSize, texCoord);
    output.TexCoords[1] = mad(float2(+1.0, -1.0), CB_TexelTextureSize, texCoord);
    output.TexCoords[2] = mad(float2(-1.0, +1.0), CB_TexelTextureSize, texCoord);
    output.TexCoords[3] = mad(float2(+1.0, +1.0), CB_TexelTextureSize, texCoord);
    
    return output;
}
