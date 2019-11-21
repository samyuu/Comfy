static const uint VertexAttribute_Position = 0;
static const uint VertexAttribute_Normal = 1;
static const uint VertexAttribute_Tangent = 2;
static const uint VertexAttribute_0x3 = 3;
static const uint VertexAttribute_TextureCoordinate0 = 4;
static const uint VertexAttribute_TextureCoordinate1 = 5;
static const uint VertexAttribute_TextureCoordinate2 = 6;
static const uint VertexAttribute_TextureCoordinate3 = 7;
static const uint VertexAttribute_Color0 = 8;
static const uint VertexAttribute_Color1 = 9;
static const uint VertexAttribute_BoneWeight = 10;
static const uint VertexAttribute_BoneIndex = 11;

struct VS_OUTPUT
{
    float4 Position : SV_POSITION;
    float3 Normal   : NORMAL;
    float2 TexCoord : TEXCOORD0;
    float4 Color    : COLOR0;
};

cbuffer DynamicStuff : register(b0)
{
    uint CB_AttributeFlags;
    uint CB_TextureFlags;
};

cbuffer MaterialData : register(b1)
{
    float3 CB_DiffuseColor;
    float1 CB_Transparency;
    float4 CB_AmbientColor;
    float3 CB_SpecularColor;
    float1 CB_Reflectivity;
    float4 CB_EmissionColor;
    float1 CB_Shininess;
    float1 CB_Intensity;
    float1 CB_BumpDepth;
    float1 CB_AlphaTestThreshold;
};

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

Texture2D DiffuseTexture;
SamplerState DiffuseSampler;

float4 PS_main(VS_OUTPUT input) : SV_Target
{
    float4 outputColor = float4(1, 1, 1, 1);

    if (CB_AttributeFlags & (1 << VertexAttribute_TextureCoordinate0))
        outputColor = DiffuseTexture.Sample(DiffuseSampler, input.TexCoord);

    if (CB_AttributeFlags & (1 << VertexAttribute_Color0))
        outputColor *= input.Color;

    if (CB_AlphaTestThreshold >= 0.0 && outputColor.a < CB_AlphaTestThreshold)
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
