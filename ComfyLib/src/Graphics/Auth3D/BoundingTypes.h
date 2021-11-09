#pragma once
#include "Types.h"
#include "Graphics/Auth3D/Transform.h"

namespace Comfy::Graphics
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
			Transform(transform.CalculateMatrix(), transform.Scale);
		}

		inline void Transform(const mat4& transform, const vec3& scale)
		{
			Center = transform * vec4(Center, 1.0f);
			Radius *= Max(scale.x, Max(scale.y, scale.z));
		}

		inline bool Contains(const vec3& point) const
		{
			return (glm::distance(Center, point) < Radius);
		}
	};

	struct Box
	{
		vec3 Center;
		vec3 Size;
	};
}
