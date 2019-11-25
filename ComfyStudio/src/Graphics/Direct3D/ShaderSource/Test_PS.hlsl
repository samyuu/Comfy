#include "InputLayouts.hlsl"
#include "VertexAttributes.hlsl"
#include "Material.hlsl"

cbuffer DynamicStuff : register(b0)
{
    uint CB_AttributeFlags;
    uint CB_TextureFlags;
};

cbuffer MaterialData : register(b1)
{
    Material CB_Material;
};

#if 0
struct Light
{
    float3 Position;
    float3 Color;
    float Intensity;
};

static const Light MainLight =
{
    float3(0.0, -1000.0, 0.0),
    float3(0.9, 0.3, 0.1),
    0.6,
};

static const float4 AmbientColor = { 1.00, 0.65, 0.68, 1.0 };
static const float AmbientBrightness = 0.1;
#endif

SamplerState DiffuseSampler     : register(s0);
SamplerState AmbientSampler     : register(s1);
SamplerState ReflectionSampler  : register(s2);

Texture2D DiffuseTexture        : register(t0);
Texture2D AmbientTexture        : register(t1);
TextureCube ReflectionTexture   : register(t2);

float4 PS_main(VS_OUTPUT input) : SV_Target
{
    float4 outputColor = float4(1, 1, 1, 1);

    if (CB_AttributeFlags & (1 << VertexAttribute_TextureCoordinate0))
        outputColor = DiffuseTexture.Sample(DiffuseSampler, input.TexCoord);

    if (CB_AttributeFlags & (1 << VertexAttribute_TextureCoordinate1))
        outputColor *= AmbientTexture.Sample(AmbientSampler, input.ShadowTexCoord);

    if (CB_AttributeFlags & (1 << VertexAttribute_Color0))
        outputColor *= input.Color;

    if (CB_Material.AlphaTestThreshold >= 0.0 && outputColor.a < CB_Material.AlphaTestThreshold)
        discard;

#if 0
    outputColor += CB_AmbientColor;
    outputColor.rgb *= CB_DiffuseColor;
    //outputColor += CB_Emissioncolor;

    // float3 lightDirection = normalize(MainLight.Position - input.Position.rgb);
    float3 lightDirection = { 0.5, -0.555, 0.5f };

    outputColor += saturate(dot(lightDirection, input.Normal) * float4(MainLight.Color, 1.0) * MainLight.Intensity);
#endif

    return outputColor;
}
