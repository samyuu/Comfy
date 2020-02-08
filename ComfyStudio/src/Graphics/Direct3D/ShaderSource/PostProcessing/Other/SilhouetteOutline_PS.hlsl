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

Texture2D<float> SilhouetteTexture : register(t0);

static const float4 NoColor = float4(0.0, 0.0, 0.0, 0.0);
static const float4 OutlineColor = float4(0.95, 0.60, 0.16, 1.0);

float4 PS_main(VS_OUTPUT input) : SV_Target
{
    if (SilhouetteTexture.Sample(LinearTextureSampler, input.TexCoord) < 0.5)
    {
        [unroll]
        for (int x = -1; x <= 1; x++)
        {
            [unroll]
            for (int y = -1; y <= 1; y++)
            {
                if (x == 0 && y == 0)
                    continue;
        
                if (SilhouetteTexture.Sample(LinearTextureSampler, input.TexCoord, int2(x, y)) >= 0.5)
                    return OutlineColor;
            }
        }
    }
    
    clip(-1.0);
    return NoColor;
}
