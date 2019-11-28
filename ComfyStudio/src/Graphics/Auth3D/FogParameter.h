#pragma once
#include "Types.h"

namespace Graphics
{
	enum class FogType
	{
		None = 0,
		Linear = 1,
		Exp = 2,
		Exp2 = 3,
	};

	struct Fog
	{
		FogType Type;
		float Density;
		float Start;
		float End;
		vec3 Color;
	};

	struct FogParameter
	{
		Fog Depth;
		Fog Height;
		Fog Bump;
	};
}
