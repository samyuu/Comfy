#include "InputLayouts.hlsl"
#include "Material.hlsl"

cbuffer CameraConstantData : register(b0)
{
    matrix CB_ViewProjection;
    float3 CB_EyePosition;
    float CB_Padding[13];
};

cbuffer MaterialData : register(b1)
{
    Material CB_Material;
};

VS_OUTPUT VS_main(VS_INPUT input)
{
    VS_OUTPUT output;
    output.Position = mul(float4(input.Position, 1.0), CB_ViewProjection);
    output.Normal = input.Normal;

    output.TexCoord.x = dot(CB_Material.TextureTransform[0], float4(input.TexCoord, 0.0, 0.0));
    output.TexCoord.y = dot(CB_Material.TextureTransform[1], float4(input.TexCoord, 0.0, 0.0));

    output.ShadowTexCoord = input.ShadowTexCoord;

    output.Color = input.Color;

    float3 eyeDirection = normalize(input.Position - CB_EyePosition);
    output.Reflection.xyz = reflect(eyeDirection, input.Normal);
    output.Reflection.w = 1.0;

    return output;
}
