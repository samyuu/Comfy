#ifndef COLOR_CORRECTION_HLSL
#define COLOR_CORRECTION_HLSL

#if 0
static const float3 SAT_PP_COEF_R = float3(1.0, 0.0, 0.0);
static const float3 SAT_PP_COEF_G = float3(0.0, 1.0, 0.0);
static const float3 SAT_PP_COEF_B = float3(0.0, 0.0, 1.0);
#elif 0
static const float3 SAT_PP_COEF_R = float3(1.095, -0.000, 0.0);
static const float3 SAT_PP_COEF_G = float3(0.0, 1.0, 0.0);
static const float3 SAT_PP_COEF_B = float3(0.0, 0.0, 1.0);
#elif 0
static const float3 SAT_PP_COEF_R = float3(1.16027, -0.00977, 5.27083E-9);
static const float3 SAT_PP_COEF_G = float3(-0.00827, 1.00827, 5.28067E-9);
static const float3 SAT_PP_COEF_B = float3(-0.0082, -0.05057, 1.05877);
#endif
    
float4 ColorCorrect_PP(float4 inputColor, float saturationParam, float brightnessParam, float3 coefParamR, float3 coefParamG, float3 coefParamB)
{
	const float almostZero = 0.0000000001;
	const float3 saturation = float3(saturationParam, saturationParam, saturationParam);
	const float3 brightness = float3(brightnessParam, brightnessParam, brightnessParam);
    
	const float3 saturatedColor = float3(
		exp2(log2(max(abs(inputColor.r), almostZero)) * saturation.r),
		exp2(log2(max(abs(inputColor.g), almostZero)) * saturation.g),
		exp2(log2(max(abs(inputColor.b), almostZero)) * saturation.b));

	return float4(
		exp2(log2(max(abs(clamp(dot(saturatedColor, coefParamR), 0.0, 1.0)), almostZero)) * brightness.r),
		exp2(log2(max(abs(clamp(dot(saturatedColor, coefParamG), 0.0, 1.0)), almostZero)) * brightness.g),
		exp2(log2(max(abs(clamp(dot(saturatedColor, coefParamB), 0.0, 1.0)), almostZero)) * brightness.b),
		inputColor.a);
}

#endif /* COLOR_CORRECTION_HLSL */
