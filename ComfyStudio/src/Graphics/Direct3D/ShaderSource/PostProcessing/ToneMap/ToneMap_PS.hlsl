struct VS_OUTPUT
{
    float4 Position : SV_POSITION;
    float2 TexCoord : TEXCOORD;
};

cbuffer ToneMapConstantData : register(b9)
{
    float1 CB_Exposure;
    float1 CB_Gamma;
    float1 CB_SaturatePower;
    float1 CB_SaturateCoefficient;
    float1 CB_AlphaLerp;
    float1 CB_AlphaValue;
};

SamplerState LinearTextureSampler
{
    Filter = MIN_MAG_MIP_LINEAR;
    AddressU = CLAMP;
    AddressV = CLAMP;
};

Texture2D<float4> ScreenTexture : register(t0);
Texture2D<float4> BloomTexture : register(t1);
Texture1D<float2> ToneMapLookupTexture : register(t2);

static const float3 YBR_COEF = { +0.300000, +0.590000, +0.110000 };
static const float3 RGB_COEF = { -0.508475, +1.000000, -0.186441 };

float4 PS_main(VS_OUTPUT input) : SV_Target
{
    float4 screenColor = ScreenTexture.Sample(LinearTextureSampler, input.TexCoord);
    
    if (true)
        screenColor.rgb += BloomTexture.Sample(LinearTextureSampler, input.TexCoord).rgb;
    
    float3 ybr = dot(screenColor.rgb, YBR_COEF);
    
    static const float p_exposure = 0.062500;
    const float2 outExposure = float2(CB_Exposure, CB_Exposure * p_exposure);
    
    ybr.xz = (screenColor.rgb - ybr.yyy).xz;
    ybr.y *= outExposure.y;
    
    float2 lookup = ToneMapLookupTexture.Sample(LinearTextureSampler, ybr.y);
    
    float3 color = float3(0.0, lookup.r, 0.0);
    color.xz = (lookup.g * outExposure.x * ybr.xyz).xz;

    float3 finalColor;
    finalColor.rb = (color.yyy + color).xz;
    finalColor.g = dot(color.rgb, RGB_COEF);
    
    return float4(finalColor.rgb, lerp(screenColor.a, CB_AlphaValue, CB_AlphaLerp));
    // return float4(finalColor.rgb, screenColor.a);
    // return float4(finalColor.rgb, 1.0f);
}
