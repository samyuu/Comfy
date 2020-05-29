struct VS_INPUT
{
    float2 Position         : POSITION;
    float2 TexCoord         : TEXCOORD0;
    float2 TexMaskCoord     : TEXCOORD1;
    float4 Color            : COLOR;
    uint TextureIndex      : TEXINDEX;
};

struct VS_OUTPUT
{
    float4 Position         : SV_POSITION;
    float2 TexCoord         : TEXCOORD0;
    float2 TexMaskCoord     : TEXCOORD1;
    float4 Color            : COLOR;
    uint TextureIndex       : TEXINDEX;
};

cbuffer CameraConstantData : register(b0)
{
    matrix CB_ViewProjection;
};

float2 FlipTextureCoordinates(float2 texCoord)
{
    return float2(texCoord.x, 1.0 - texCoord.y);
}

VS_OUTPUT VS_main(VS_INPUT input)
{
	VS_OUTPUT output;
	
	output.Position = mul(float4(input.Position, 0.0, 1.0), CB_ViewProjection);
	output.Color = input.Color;
	
	output.TexCoord = FlipTextureCoordinates(input.TexCoord);
	output.TexMaskCoord = FlipTextureCoordinates(input.TexMaskCoord);
	
	output.TextureIndex = input.TextureIndex;
	
	return output;
}
