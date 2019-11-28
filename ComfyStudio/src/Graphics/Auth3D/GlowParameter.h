#pragma once
#include "Types.h"

namespace Graphics
{
	enum class ToneMapMethod
	{
		YCC_Exponent = 0,
		RGB_Linear = 1,
		RGB_Liear2 = 2,
	};

	struct GlowParameter
	{
		float Exposure;
		float Gamma;
		int32_t SaturatePower;
		float SaturateCoefficient;
		vec3 Flare;
		vec3 Sigma;
		vec3 Intensity;
		bool AutoExposure;
		ToneMapMethod ToneMapMethod;
		vec3 FadeColor;
		vec4 ToneTransform;
	};
}
