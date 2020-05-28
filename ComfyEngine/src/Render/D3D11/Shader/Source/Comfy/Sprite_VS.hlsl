struct VS_INPUT
{
    float2 Position         : POSITION;
    float2 TexCoord         : TEXCOORD0;
    float2 TexMaskCoord     : TEXCOORD1;
    float4 Color            : COLOR;

#if !defined(COMFY_ENGINE_RENDERER2D_IMPL_FIXED_TEX)
    uint2 TextureIndices    : TEXINDEX;
#endif
};

struct VS_OUTPUT
{
    float4 Position         : SV_POSITION;
    float2 TexCoord         : TEXCOORD0;
    float2 TexMaskCoord     : TEXCOORD1;
    float4 Color            : COLOR;
    
#if !defined(COMFY_ENGINE_RENDERER2D_IMPL_FIXED_TEX)
    uint2 TextureIndices    : TEXINDEX;
#endif
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

#if !defined(COMFY_ENGINE_RENDERER2D_IMPL_FIXED_TEX)
    output.TextureIndices = input.TextureIndices;
#endif
    
    return output;
}
