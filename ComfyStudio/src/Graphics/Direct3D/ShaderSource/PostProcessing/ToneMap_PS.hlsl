struct VS_OUTPUT
{
    float4 Position : SV_POSITION;
    float2 TexCoord : TEXCOORD;
    float2 Exposure : EXPOSURE;
};

SamplerState LinearTextureSampler
{
    Filter = MIN_MAG_MIP_LINEAR;
    AddressU = CLAMP;
    AddressV = CLAMP;
};

Texture2D ScreenTexture : register(t0);
Texture2D BloomTexture : register(t1);
Texture1D ToneMapLookupTexture : register(t2);

static const float3 YBR_COEF = { 0.30, 0.59, 0.11 };
static const float3 RGB_COEF = { -0.508475, 1.0, -0.186441 };

float4 PS_main(VS_OUTPUT input) : SV_Target
{
    float4 screenColor = ScreenTexture.Sample(LinearTextureSampler, input.TexCoord);
    
    if (true)
        screenColor.rgb += BloomTexture.Sample(LinearTextureSampler, input.TexCoord).rgb;
    
    float3 ybr = dot(screenColor.rgb, YBR_COEF);
    
    ybr.xz = (screenColor.rgb - ybr.yyy).xz;
    ybr.y *= input.Exposure.y;
    
    float2 lookup = ToneMapLookupTexture.Sample(LinearTextureSampler, ybr.y).rg;
    
    float3 color = float3(0.0, lookup.r, 0.0);
    color.xz = (lookup.g * input.Exposure.x * ybr.xyz).xz;

    float3 result;
    result.rb = (color.yyy + color).xz;
    result.g = dot(color.rgb, RGB_COEF);
    
    // return float4(result.rgb, screenColor.a);
    return float4(result.rgb, 1.0f);
}
