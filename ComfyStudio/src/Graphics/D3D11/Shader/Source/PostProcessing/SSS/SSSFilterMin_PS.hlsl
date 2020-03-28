struct VS_OUTPUT
{
    float4 Position : SV_POSITION;
    float2 TexCoord : TEXCOORD;
};

SamplerState LinearTextureSampler
{
    Filter = MIN_MAG_MIP_LINEAR;
    AddressU = CLAMP;
    AddressV = CLAMP;
};

Texture2D SubsurfaceScatteringTexture : register(t0);

float4 PS_main(VS_OUTPUT input) : SV_Target
{
    float4 tmp, col0, col1;
    
    col0 = SubsurfaceScatteringTexture.Sample(LinearTextureSampler, input.TexCoord);
    float4 o_color = col0;
    
    if ((tmp.w = col0.w - 1.0) == 0.0)
        return o_color;
    
    col1 = SubsurfaceScatteringTexture.Sample(LinearTextureSampler, input.TexCoord, int2(-2, 0)); // a_tex1
    if ((tmp.w = col0.w - col1.w) < 0.0)
        col0 = col1;
    
    col1 = SubsurfaceScatteringTexture.Sample(LinearTextureSampler, input.TexCoord, int2(+2, 0)); // a_tex2
    if ((tmp.w = col0.w - col1.w) < 0.0)
        col0 = col1;
    
    col1 = SubsurfaceScatteringTexture.Sample(LinearTextureSampler, input.TexCoord, int2(0, -2)); // a_tex3
    if ((tmp.w = col0.w - col1.w) < 0.0)
        col0 = col1;
    
    col1 = SubsurfaceScatteringTexture.Sample(LinearTextureSampler, input.TexCoord, int2(0, +2)); // a_tex4
    if ((tmp.w = col0.w - col1.w) < 0.0)
        col0 = col1;
    
    return col0;
}
