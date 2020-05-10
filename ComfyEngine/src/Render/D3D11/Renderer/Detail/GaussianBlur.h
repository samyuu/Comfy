#pragma once
#include "ConstantData.h"
#include "Graphics/Auth3D/LightParam/GlowParameter.h"

namespace Comfy::Graphics::D3D11
{
	inline void CalculateGaussianBlurKernel(const GlowParameter& glow, PPGaussCoefConstantData& outData)
	{
		constexpr float powStart = 1.0f, powIncrement = 1.0f;
		constexpr float sigmaFactor = 0.8f, intensityFactor = 1.0f;

		const float firstCoef = (powStart - (powIncrement * 0.5f)) * 2.0f;

		for (int channel = 0; channel < 3; channel++)
		{
			const float channelSigma = glow.Sigma[channel] * sigmaFactor;
			const float reciprocalSigma = 1.0f / ((channelSigma * 2.0f) * channelSigma);

			float accumilatedExpResult = firstCoef;
			float accumilatingPow = powStart;

			std::array<float, 8> results = { firstCoef };
			for (int i = 1; i < 7; i++)
			{
				const float result = glm::exp(-((accumilatingPow * accumilatingPow) * reciprocalSigma)) * powIncrement;
				accumilatingPow += powIncrement;

				results[i] = result;
				accumilatedExpResult += result;
			}

			const float channelIntensity = glow.Intensity[channel] * (intensityFactor * 0.5f);

			for (int i = 0; i < outData.Coefficients.size(); i++)
				outData.Coefficients[i][channel] = (results[i] / accumilatedExpResult) * channelIntensity;
		}
	}
}
