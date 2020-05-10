#pragma once
#include "Types.h"
#include "CoreTypes.h"

namespace Comfy::Graphics
{
	enum class VectorComponent : u32 { X, Y, Z };

	constexpr std::array<VectorComponent, vec3::length()> RotationModeComponentsOrder = { VectorComponent::Z, VectorComponent::Y, VectorComponent::X };

	template <size_t Index>
	constexpr u32 GetRotationModeIndex()
	{
		static_assert(Index < RotationModeComponentsOrder.size());
		return static_cast<u32>(RotationModeComponentsOrder[Index]);
	}

	// TODO: Rename to Transform3D
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

		inline std::array<mat4, 3> CalculateRotationMatrices() const
		{
			return
			{
				glm::rotate(mat4(1.0f), glm::radians(Rotation.x), vec3(1.0f, 0.0f, 0.0f)),
				glm::rotate(mat4(1.0f), glm::radians(Rotation.y), vec3(0.0f, 1.0f, 0.0f)),
				glm::rotate(mat4(1.0f), glm::radians(Rotation.z), vec3(0.0f, 0.0f, 1.0f)),
			};
		}

		inline mat4 CalculateMatrix() const
		{
			const mat4 translation = glm::translate(mat4(1.0f), Translation);
			const mat4 scale = glm::scale(mat4(1.0f), Scale);

			if (Rotation != vec3(0.0f))
			{
				const auto rotations = CalculateRotationMatrices();
				return translation * rotations[GetRotationModeIndex<0>()] * rotations[GetRotationModeIndex<1>()] * rotations[GetRotationModeIndex<2>()] * scale;
			}
			else
			{
				return translation * scale;
			}
		}

		inline void ApplyParent(const Transform& parent)
		{
			Scale *= parent.Scale;
			Translation *= parent.Scale;

			if (parent.Rotation != vec3(0.0f))
			{
				Rotation += parent.Rotation;
				
				const auto rotations = parent.CalculateRotationMatrices();
				Translation = rotations[GetRotationModeIndex<0>()] * rotations[GetRotationModeIndex<1>()] * rotations[GetRotationModeIndex<2>()] * vec4(Translation, 1.0f);
			}
			
			Translation += parent.Translation;
		}
	};
}
