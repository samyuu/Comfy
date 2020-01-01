#include "../Include/Common.hlsl"

struct VS_INPUT
{
    uint VertexID : SV_VERTEXID;
};

struct VS_OUTPUT
{
    float4 Position : SV_POSITION;
    float2 TexCoord : TEXCOORD;
    float2 Exposure : EXPOSURE;
};

cbuffer ToneMapConstantData : register(b0)
{
    float CB_Exposure;
    float CB_Gamma;
    float CB_SaturatePower;
    float CB_SaturateCoefficient;
};

VS_OUTPUT VS_main(VS_INPUT input)
{
    VS_OUTPUT output;

    output.TexCoord = ScreenTexCoordFromVertexID(input.VertexID);
    output.Position = ScreenPositionFromTexCoord(output.TexCoord);
    
    static const float p_exposure = 0.062500;
    output.Exposure = float2(CB_Exposure, CB_Exposure * p_exposure);
    
    return output;
}
