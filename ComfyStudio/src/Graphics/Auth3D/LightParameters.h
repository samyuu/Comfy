#pragma once
#include "Types.h"
#include "LightDataIBL.h"

namespace Graphics
{
	enum class LightType
	{
		None = 0,
		Parallel = 1,
		Point = 2,
		Spot = 3,
	};

	struct Light
	{
		LightType Type;

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

	enum LightTargetType
	{
		Character = 0,
		Stage = 1,
		Sun = 2,
		Reflect = 3,
		Shadow = 4,
		CharacterColor = 5,
		CharacterF = 6,
		Projection = 7,
	};

	struct LightParameter
	{
		Light Character;
		Light Stage;
		Light Sun;
		Light Reflect;
		Light Shadow;
		Light CharacterColor;
		Light CharacterF;
		Light Projection;

		LightDataIBL IBL;
	};
}
