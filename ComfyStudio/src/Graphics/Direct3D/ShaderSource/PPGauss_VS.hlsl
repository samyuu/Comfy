#include "Include/Common.hlsl"

struct VS_INPUT
{
    uint VertexID : SV_VERTEXID;
};

struct VS_OUTPUT
{
    float4 Position : SV_POSITION;
    float2 TexCoord : TEXCOORD;
};

VS_OUTPUT VS_main(VS_INPUT input)
{
    VS_OUTPUT output;
    output.TexCoord = ScreenTexCoordFromVertexID(input.VertexID);
    output.Position = ScreenPositionFromTexCoord(output.TexCoord);
    return output;
}
