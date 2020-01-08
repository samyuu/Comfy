#pragma once
#include "Types.h"

namespace Graphics
{
	struct Transform
	{
		vec3 Translation;
		vec3 Scale;
		vec3 Rotation;

		Transform() = default;
		Transform(const Transform& other) { *this = other; }
		explicit Transform(vec3 translation) : Translation(translation), Scale(1.0f), Rotation(0.0f) {}

		inline void operator=(const Transform& other)
		{
			Translation = other.Translation;
			Scale = other.Scale;
			Rotation = other.Rotation;
		}

		inline bool operator==(const Transform& other) const
		{
			return Translation == other.Translation && Scale == other.Scale && Rotation == other.Rotation;
		}

		inline bool operator!=(const Transform& other) const
		{
			return !(*this == other);
		}
	};
}
