#pragma once
#include "Types.h"

namespace Graphics
{
	struct Transform2DFieldFlags
	{
		bool OriginX : 1;
		bool OriginY : 1;
		bool PositionX : 1;
		bool PositionY : 1;
		bool Rotation : 1;
		bool ScaleX : 1;
		bool ScaleY : 1;
		bool Opacity : 1;
	};

	enum Transform2DField_Enum
	{
		Transform2D_OriginX,
		Transform2D_OriginY,
		Transform2D_PositionX,
		Transform2D_PositionY,
		Transform2D_Rotation,
		Transform2D_ScaleX,
		Transform2D_ScaleY,
		Transform2D_Opacity,
		Transform2D_Count,
	};

	struct Transform2D
	{
		vec2 Origin;
		vec2 Position;
		float Rotation;
		vec2 Scale;
		float Opacity;

		Transform2D() = default;

		constexpr Transform2D(vec2 position)
			: Origin(0.0f), Position(position), Rotation(0.0f), Scale(1.0f), Opacity(1.0f)
		{
		};

		constexpr Transform2D(vec2 origin, vec2 position, float rotation, vec2 scale, float opacity)
			: Origin(origin), Position(position), Rotation(rotation), Scale(scale), Opacity(opacity)
		{
		};

		inline Transform2D& operator=(const Transform2D& other)
		{
			Origin = other.Origin;
			Position = other.Position;
			Rotation = other.Rotation;
			Scale = other.Scale;
			Opacity = other.Opacity;
			return *this;
		}

		inline bool operator==(const Transform2D& other) const
		{
			return (Origin == other.Origin) && (Position == other.Position) && (Rotation == other.Rotation) && (Scale == other.Scale) && (Opacity == other.Opacity);
		}

		inline bool operator!=(const Transform2D& other) const
		{
			return !(*this == other);
		}
	};
}
