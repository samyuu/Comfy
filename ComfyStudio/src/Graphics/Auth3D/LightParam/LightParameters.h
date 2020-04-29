#pragma once
#include "Types.h"
#include "Graphics/GraphicTypes.h"
#include "IO/FileInterface.h"

namespace Comfy::Graphics
{
	struct Light
	{
		LightSourceType Type;

		// NOTE: All light types
		vec3 Ambient;
		vec3 Diffuse;
		vec3 Specular;
		vec3 Position;

		// NOTE: Character light only
		float ToneCurveBegin;
		float ToneCurveEnd;
		float ToneCurveBlendRate;

		// NOTE: Spot light only
		vec3 SpotDirection;
		float SpotExponent;
		float SpotCuttoff;
		float AttenuationConstant;
		float AttenuationLinear;
		float AttenuationQuadratic;
	};

	class LightParameter final : public IO::IBufferParsable
	{
	public:
		LightParameter();
		~LightParameter() = default;

	public:
		Light Character;
		Light Stage;
		Light Sun;
		Light Reflect;
		Light Shadow;
		Light CharacterColor;
		Light CharacterF;
		Light Projection;

		Light* GetLight(LightTargetType type);

	public:
		void Parse(const u8* buffer, size_t bufferSize) override;
	};
}
