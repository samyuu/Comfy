#ifndef FONT_HLSL
#define FONT_HLSL

static const float3 TEXT_BORDER_FACTOR = { 0.9, 0.6, 0.8 };

float4 SampleFont(Texture2D inputTexture, SamplerState inputSampler, float2 texCoord, float4 inputColor)
{
    float4 fontColor = inputTexture.Sample(inputSampler, texCoord);
    return fontColor * inputColor;
}

float4 SampleFontWithBorder(Texture2D inputTexture, SamplerState inputSampler, float2 texCoord, float4 inputColor)
{
    float2 textureSize;
    inputTexture.GetDimensions(textureSize.x, textureSize.y);

    float2 texelSize = 1.0 / textureSize;
    float2 texelSizeDouble = 2.0 / textureSize;

    float textureAlpha = inputTexture.Sample(inputSampler, texCoord).a;
    float4 shadow = float4(0.0, 0.0, 0.0, textureAlpha);

    shadow.x = max(shadow.x, inputTexture.Sample(inputSampler, texCoord + float2(-texelSize.x, 0.0)).a );
    shadow.x = max(shadow.x, inputTexture.Sample(inputSampler, texCoord + float2(+texelSize.x, 0.0)).a );
    shadow.x = max(shadow.x, inputTexture.Sample(inputSampler, texCoord + float2(0.0, -texelSize.y)).a );
    shadow.x = max(shadow.x, inputTexture.Sample(inputSampler, texCoord + float2(0.0, +texelSize.y)).a );

    shadow.y = max(shadow.y, inputTexture.Sample(inputSampler, texCoord + float2(-texelSizeDouble.x, 0.0)).a );
    shadow.y = max(shadow.y, inputTexture.Sample(inputSampler, texCoord + float2(+texelSizeDouble.x, 0.0)).a );
    shadow.y = max(shadow.y, inputTexture.Sample(inputSampler, texCoord + float2(0.0, -texelSizeDouble.y)).a );
    shadow.y = max(shadow.y, inputTexture.Sample(inputSampler, texCoord + float2(0.0, +texelSizeDouble.y)).a );
	
    shadow.z = max(shadow.z, inputTexture.Sample(inputSampler, texCoord + float2(-texelSize.x, -texelSize.y)).a );
    shadow.z = max(shadow.z, inputTexture.Sample(inputSampler, texCoord + float2(+texelSize.x, -texelSize.y)).a );
    shadow.z = max(shadow.z, inputTexture.Sample(inputSampler, texCoord + float2(-texelSize.x, +texelSize.y)).a );
    shadow.z = max(shadow.z, inputTexture.Sample(inputSampler, texCoord + float2(+texelSize.x, +texelSize.y)).a );
	
    shadow.a = max(shadow.a, shadow.x * TEXT_BORDER_FACTOR.x);
    shadow.a = max(shadow.a, shadow.y * TEXT_BORDER_FACTOR.y);
    shadow.a = max(shadow.a, shadow.z * TEXT_BORDER_FACTOR.z);

    float3 color = (inputColor.rgb * textureAlpha) * inputColor.a;
    return float4((color * (1.0 / sqrt(shadow.a))), shadow.a * inputColor.a);
}

#endif /* FONT_HLSL */
