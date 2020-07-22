#pragma once
#include "Types.h"

namespace Comfy::Graphics
{
	struct Ray
	{
		// NOTE: Assumed to be in world space
		vec3 Origin;

		// NOTE: Assumed to be normalized
		vec3 Direction;
	};
}
