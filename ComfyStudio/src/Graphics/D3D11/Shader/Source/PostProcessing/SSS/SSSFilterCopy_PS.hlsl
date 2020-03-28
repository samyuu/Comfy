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
    return SubsurfaceScatteringTexture.Sample(LinearTextureSampler, input.TexCoord);
}
