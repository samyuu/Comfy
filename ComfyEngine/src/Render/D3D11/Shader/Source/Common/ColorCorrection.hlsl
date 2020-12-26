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
    
float4 ColorCorrect_PP(float4 inputColor, float gamma, float contrast, float3 coefR, float3 coefG, float3 coefB)
{
    const float almostZero = 0.0000000001;
    
    const float3 gammaCorrected = float3(
		exp2(log2(max(abs(inputColor.r), almostZero)) * gamma),
		exp2(log2(max(abs(inputColor.g), almostZero)) * gamma),
		exp2(log2(max(abs(inputColor.b), almostZero)) * gamma));

    return float4(
		exp2(log2(max(abs(clamp(dot(gammaCorrected, coefR), 0.0, 1.0)), almostZero)) * contrast),
		exp2(log2(max(abs(clamp(dot(gammaCorrected, coefG), 0.0, 1.0)), almostZero)) * contrast),
		exp2(log2(max(abs(clamp(dot(gammaCorrected, coefB), 0.0, 1.0)), almostZero)) * contrast),
		inputColor.a);
}

#endif /* COLOR_CORRECTION_HLSL */
