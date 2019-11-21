struct VS_INPUT
{
    float3 Position : POSITION;
    float3 Normal   : NORMAL;
    float2 TexCoord : TEXCOORD;
    float4 Color    : COLOR;
};

struct VS_OUTPUT
{
    float4 Position : SV_POSITION;
    float3 Normal   : NORMAL;
    float2 TexCoord : TEXCOORD0;
    float4 Color    : COLOR0;
};

cbuffer CameraConstantData : register(b0)
{
    matrix CB_ViewProjection;
};

VS_OUTPUT VS_main(VS_INPUT input)
{
    VS_OUTPUT output;
    output.Position = mul(float4(input.Position, 1.0), CB_ViewProjection);
    output.Normal = input.Normal;
    output.TexCoord = input.TexCoord;
    output.Color = input.Color;
    return output;
}
