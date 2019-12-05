#pragma once
#include "Types.h"
#include "Graphics/GraphicsTypes.h"
#include "FileSystem/FileInterface.h"

namespace Graphics
{
	class GlowParameter final : public FileSystem::IBufferParsable
	{
	public:
		GlowParameter();
		~GlowParameter() = default;

	public:
		float Exposure;
		float Gamma;
		int32_t SaturatePower;
		float SaturateCoefficient;
		float FlareA;
		float ShaftA;
		float GhostA;
		vec3 Sigma;
		vec3 Intensity;
		bool AutoExposure;
		ToneMapMethod ToneMapMethod;
		vec4 FadeColor;
		vec4 ToneTransform;

	public:
		void Parse(const uint8_t* buffer) override;
	};
}
