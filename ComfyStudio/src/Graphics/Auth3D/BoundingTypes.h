#pragma once
#include "Types.h"

namespace Graphics
{
	struct Sphere
	{
		vec3 Center;
		float Radius;
	};

	struct Box
	{
		vec3 Center;
		vec3 Size;
	};
}
