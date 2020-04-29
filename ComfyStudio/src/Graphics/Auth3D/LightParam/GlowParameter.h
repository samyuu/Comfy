#pragma once
#include "Types.h"
#include "Graphics/GraphicTypes.h"
#include "IO/FileInterface.h"

namespace Comfy::Graphics
{
	class GlowParameter final : public IO::IBufferParsable
	{
	public:
		GlowParameter();
		~GlowParameter() = default;

	public:
		float Exposure;
		float Gamma;
		i32 SaturatePower;
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
		void Parse(const u8* buffer, size_t bufferSize) override;
	};
}
