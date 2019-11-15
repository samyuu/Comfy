struct VS_INPUT
{
    float4 Position : POSITION;
    float4 Color    : COLOR;
    float2 TexCoord : TEXCOORD;
};

struct VS_OUTPUT
{
    float4 Position : SV_POSITION;
    float4 Color    : COLOR;
    float2 TexCoord : TEXCOORD;
};

cbuffer MatrixConstantBuffer : register(b0)
{
    matrix CB_Model, CB_View, CB_Projection;
};

VS_OUTPUT VS_main(VS_INPUT input)
{
    VS_OUTPUT output;
    output.Position = input.Position;
    output.Position = mul(output.Position, CB_Model);
    output.Position = mul(output.Position, CB_View);
    output.Position = mul(output.Position, CB_Projection);
    output.Color = input.Color;
    output.TexCoord = input.TexCoord;
    return output;
}
