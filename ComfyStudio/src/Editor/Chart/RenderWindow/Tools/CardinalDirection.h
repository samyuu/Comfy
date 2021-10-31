#pragma once
#include "Types.h"
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
		return (cardinal >= CardinalDirection::IntercardinalStart && cardinal <= CardinalDirection::IntercardinalEnd);
	}

	constexpr vec2 CardinalToTargetRowDirection(CardinalDirection cardinal, vec2 perBeatDiagonalSpacing)
	{
		switch (cardinal)
		{
		case CardinalDirection::North: return vec2(+0.0f, -1.0f);
		case CardinalDirection::East: return vec2(+1.0f, +0.0f);
		case CardinalDirection::South: return vec2(+0.0f, +1.0f);
		case CardinalDirection::West: return vec2(-1.0f, +0.0f);
		}

		const vec2 diagonalDirection = glm::normalize(glm::abs(perBeatDiagonalSpacing));
		switch (cardinal)
		{
		case CardinalDirection::NorthEast: return vec2(+diagonalDirection.x, -diagonalDirection.y);
		case CardinalDirection::SouthEast: return vec2(+diagonalDirection.x, +diagonalDirection.y);
		case CardinalDirection::SouthWest: return vec2(-diagonalDirection.x, +diagonalDirection.y);
		case CardinalDirection::NorthWest: return vec2(-diagonalDirection.x, -diagonalDirection.y);
		default: return vec2(1.0f, 0.0f);
		}
	}
}
