#pragma once
#include "Types.h"
#include "Render/Core/Camera.h"
#include "ConstantData.h"
#include "Graphics/Auth3D/ObjSet.h"

namespace Comfy::Render::Detail
{
	constexpr float DefaultSSSParameter = 0.6f;

	inline bool UsesSSSSkin(const Graphics::Material& material)
	{
		if (material.ShaderType == Graphics::Material::ShaderIdentifiers::Skin ||
			material.ShaderType == Graphics::Material::ShaderIdentifiers::EyeBall ||
			material.ShaderType == Graphics::Material::ShaderIdentifiers::Cloth ||
			material.ShaderType == Graphics::Material::ShaderIdentifiers::EyeLens)
			return true;

		return false;
	}

	inline bool UsesSSSSkinConst(const Graphics::Material& material)
	{
		if (material.ShaderType == Graphics::Material::ShaderIdentifiers::Hair ||
			material.ShaderType == Graphics::Material::ShaderIdentifiers::Tights)
			return true;

		// NOTE: The Ambient alpha is used to determine the SSS strength so this is really just an optimization to avoid using the more expensive non cost shader
		if (material.ShaderType == Graphics::Material::ShaderIdentifiers::Cloth && material.Color.Ambient.a >= 1.0f)
			return true;

		return false;
	}

	inline float RootMeanSquare(const vec3 value)
	{
		const vec3 squared = (value * value);
		return glm::sqrt(squared.x + squared.y + squared.z);
	}

	inline double CalculateSSSCameraCoefficient(const vec3 viewPoint, const vec3 interest, const float fieldOfView, const std::array<std::optional<vec3>, 2>& characterHeadPositions)
	{
		std::array<float, 2> meanSquareRoots;
		std::array<vec3, 2> headPositions;

		for (size_t i = 0; i < meanSquareRoots.size(); i++)
		{
			if (characterHeadPositions[i].has_value())
			{
				headPositions[i] = characterHeadPositions[i].value();
				meanSquareRoots[i] = RootMeanSquare(viewPoint - characterHeadPositions[i].value());
			}
			else
			{
				headPositions[i] = interest;
				meanSquareRoots[i] = 999999.0f;
			}
		}

		vec3 headPosition;
		if (meanSquareRoots[0] <= meanSquareRoots[1])
		{
			headPosition = headPositions[0];
		}
		else
		{
			headPosition = headPositions[1];
			headPositions[0] = headPositions[1];
		}

		const double result = 1.0f / Clamp(
			Max(glm::tan(glm::radians(fieldOfView * 0.5f)) * 5.0f, 0.25f) *
			Max(RootMeanSquare(viewPoint - ((RootMeanSquare(interest - headPosition) > 1.25f) ? headPositions[0] : interest)), 0.25f),
			0.25f, 100.0f);

		return result;
	}

	inline std::array<vec4, 36> CalculateSSSFilterCoefficient(double cameraCoefficient)
	{
		static constexpr std::array<std::array<double, 3>, 4> weights =
		{
			std::array { 1.0, 2.0, 5.0, },
			std::array { 0.2, 0.4, 1.2, },
			std::array { 0.3, 0.7, 2.0, },
			std::array { 0.4, 0.3, 0.3, },
		};

		constexpr double expFactorIncrement = 1.0;

		std::array<vec4, 36> coefficients = {};

		for (int iteration = 0; iteration < 3; iteration++)
		{
			for (int component = 0; component < 3; component++)
			{
				const double reciprocalWeight = 1.0 / (cameraCoefficient * weights[component][iteration]);

				double expSum = 0.0;
				double expFactorSum = 0.0;

				std::array<double, 6> exponentials;
				for (double& exp : exponentials)
				{
					const double expResult = glm::exp(reciprocalWeight * expFactorSum * -0.5 * (reciprocalWeight * expFactorSum));
					exp = expResult;

					expSum += expResult;
					expFactorSum += expFactorIncrement;
				}

				const double reciprocalExpSum = 1.0 / expSum;
				for (double& exp : exponentials)
					exp *= reciprocalExpSum;

				for (int i = 0; i < 6; i++)
				{
					const double weight = weights.back()[iteration] * exponentials[i];
					for (int j = 0; j < 6; j++)
					{
						float& coef = coefficients[(i * 6) + j][component];
						coef = static_cast<float>(static_cast<double>(coef) + exponentials[j] * weight);
					}
				}
			}
		}

		return coefficients;
	}

	inline void CalculateSSSCoefficients(const Camera3D& camera, SSSFilterConstantData& outData)
	{
		const std::array<std::optional<vec3>, 2> characterHeadPositions = { vec3(0.0f, 1.055f, 0.0f) };
		const double cameraCoefficient = CalculateSSSCameraCoefficient(camera.ViewPoint, camera.Interest, camera.FieldOfView, characterHeadPositions);

		outData.Coefficients = CalculateSSSFilterCoefficient(cameraCoefficient);
	}
}
