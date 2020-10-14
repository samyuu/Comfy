#pragma once
#include "Types.h"
#include "CoreTypes.h"
#include "Editor/Chart/TargetPropertyRules.h"

namespace Comfy::Studio::Editor
{
	enum class CardinalDirection : u8
	{
		North,
		East,
		South,
		West,
		NorthEast,
		SouthEast,
		SouthWest,
		NorthWest,
		Count,

		CardinalStart = North,
		CardinalEnd = West,
		IntercardinalStart = NorthEast,
		IntercardinalEnd = NorthWest,
	};

	constexpr std::array<const char*, EnumCount<CardinalDirection>()> CardinalDirectionAbbreviations =
	{
		"N", "E", "S", "W", "NE", "SE", "SW", "NW",
	};

	constexpr CardinalDirection AngleToNearestCardinal(const f32 degrees)
	{
		if (degrees >= -67.5 && degrees <= -22.5f)
			return CardinalDirection::NorthEast;
		if (degrees >= +22.5f && degrees <= +67.5)
			return CardinalDirection::SouthEast;
		if (degrees >= +112.5f && degrees <= +157.5f)
			return CardinalDirection::SouthWest;
		if (degrees >= -157.5f && degrees <= -112.5f)
			return CardinalDirection::NorthWest;

		if (degrees >= -35.0f && degrees <= +35.0f)
			return CardinalDirection::East;
		if (degrees >= -135.0f && degrees <= +35.0f)
			return CardinalDirection::North;
		if ((degrees >= -180.0f && degrees <= -135.0f) || (degrees <= 180.0f && degrees >= 135.0f))
			return CardinalDirection::West;
		else
			return CardinalDirection::South;
	}

	constexpr bool IsIntercardinal(CardinalDirection cardinal)
	{
		return (cardinal >= CardinalDirection::IntercardinalStart);
	}

	constexpr std::array<vec2, EnumCount<CardinalDirection>()> CardinalTargetRowDirections =
	{
		vec2(+0.0f, -1.0f),
		vec2(+1.0f, +0.0f),
		vec2(+0.0f, +1.0f),
		vec2(-1.0f, +0.0f),
		vec2(+Rules::PlacementStairDirection.x, -Rules::PlacementStairDirection.y),
		vec2(+Rules::PlacementStairDirection.x, +Rules::PlacementStairDirection.y),
		vec2(-Rules::PlacementStairDirection.x, +Rules::PlacementStairDirection.y),
		vec2(-Rules::PlacementStairDirection.x, -Rules::PlacementStairDirection.y),
	};

	constexpr vec2 CardinalToTargetRowDirection(CardinalDirection direction)
	{
		return CardinalTargetRowDirections[static_cast<u8>(direction)];
	}
}
