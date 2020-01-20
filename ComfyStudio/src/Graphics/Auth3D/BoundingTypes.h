#pragma once
#include "Types.h"
#include "Graphics/Auth3D/Transform.h"

namespace Graphics
{
	struct Sphere
	{
		vec3 Center;
		float Radius;

		inline void operator=(const Sphere& other)
		{
			Center = other.Center;
			Radius = other.Radius;
		}

		inline bool operator==(const Sphere& other) const
		{
			return (Center == other.Center) && (Radius == other.Radius);
		}

		inline Sphere operator*(const Transform& transform) const
		{
			Sphere output = *this;
			output.Transform(transform);
			return output;
		}

		inline Sphere operator*=(const Transform& transform)
		{
			Transform(transform);
			return *this;
		}

		inline void Transform(const Transform& transform)
		{
			Center = transform.CalculateMatrix() * vec4(Center, 1.0f);
			Radius *= glm::max(transform.Scale.x, glm::max(transform.Scale.y, transform.Scale.z));
		}
	};

	struct Box
	{
		vec3 Center;
		vec3 Size;
	};
}
