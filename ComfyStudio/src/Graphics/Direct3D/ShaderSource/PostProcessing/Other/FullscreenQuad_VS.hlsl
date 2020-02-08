struct VS_INPUT
{
    uint VertexID : SV_VERTEXID;
};

struct VS_OUTPUT
{
    float4 Position : SV_POSITION;
    float2 TexCoord : TEXCOORD;
};

float2 ScreenTexCoordFromVertexID(const uint vertexID)
{
    return float2(vertexID % 2, vertexID % 4 / 2);
}

float4 ScreenPositionFromTexCoord(const float2 texCoord)
{
    return float4((texCoord.x - 0.5) * 2.0, -(texCoord.y - 0.5) * 2.0, 0.0, 1.0);
}

VS_OUTPUT VS_main(VS_INPUT input)
{
    VS_OUTPUT output;
    output.TexCoord = ScreenTexCoordFromVertexID(input.VertexID);
    output.Position = ScreenPositionFromTexCoord(output.TexCoord);
    return output;
}
