#pragma once
#include "Types.h"
#include "CoreTypes.h"
#include "SortedTargetList.h"

namespace Comfy::Studio::Editor
{
	inline vec2 GetButtonPathSinePoint(f32 progress, vec2 pivot, f32 degrees, f32 frequency, f32 amplitude, f32 distance)
	{
		auto rotate = [](vec2 point, f32 radians)
		{
			const auto sin = glm::sin(radians), cos = glm::cos(radians);
			return vec2(point.x * cos - point.y * sin, point.x * sin + point.y * cos);
		};

		if (distance == 0.0f)
			return pivot;

		// TODO: Ehhh...
		if (static_cast<i32>(frequency) % 2 != 0)
			frequency *= -1.0f;

		progress = (1.0f - progress);

		const auto sinePoint = vec2(progress * distance, glm::sin(progress * glm::pi<f32>() * frequency) / 12.0f * amplitude);
		return rotate(sinePoint, glm::radians(degrees - 90.0f)) + pivot;
	}

	inline vec2 GetButtonPathSinePoint(f32 progress, const TargetProperties& properties)
	{
		return GetButtonPathSinePoint(progress, properties.Position, properties.Angle, properties.Frequency, properties.Amplitude, properties.Distance);
	}

	namespace Rules
	{
		constexpr vec2 PlacementAreaSize = { 1920.0f, 1080.0f };
		constexpr vec2 PlacementAreaCenter = (PlacementAreaSize * 0.5f);

		constexpr f32 PlacementDistancePerBeat = 192.0f;
		constexpr f32 PlacementDistancePerBeatStair = 186.59046f;

		constexpr f32 TickToDistance(const TimelineTick tick)
		{
			const auto beats = static_cast<f64>(tick.Ticks()) / static_cast<f64>(TimelineTick::TicksPerBeat);
			const auto beatDistanceRatio = static_cast<f64>(PlacementDistancePerBeat);

			return static_cast<f32>(beats * beatDistanceRatio);
		}

		constexpr TimelineTick DistanceToTick(const f32 distance)
		{
			const auto beats = static_cast<f64>(distance) / static_cast<f64>(PlacementDistancePerBeat);
			const auto ticks = beats * static_cast<f64>(TimelineTick::TicksPerBeat);

			return TimelineTick::FromTicks(static_cast<i32>(ticks));
		}

		constexpr f32 TickToStairDistance(const TimelineTick tick)
		{
			const auto beats = static_cast<f64>(tick.Ticks()) / static_cast<f64>(TimelineTick::TicksPerBeat);
			const auto beatDistanceRatio = static_cast<f32>(PlacementDistancePerBeatStair);

			return static_cast<f32>(beats * beatDistanceRatio);
		}

		constexpr vec2 RecommendedPlacementAreaMin =
		{
			TickToDistance(TimelineTick::FromBars(1) / 8),
			TickToDistance(TimelineTick::FromBars(1) / 4),
		};

		constexpr vec2 RecommendedPlacementAreaMax =
		{
			PlacementAreaSize.x - TickToDistance((TimelineTick::FromBars(1) / 8)),
			PlacementAreaSize.y - TickToDistance((TimelineTick::FromBars(1) / 4) + (TimelineTick::FromBars(1) / 32)),
		};

		constexpr f32 VerticalSyncPairPlacementDistance = TickToDistance(TimelineTick::FromBars(1) / 8);

		static_assert(RecommendedPlacementAreaMin == vec2(96.0f, 192.0f));
		static_assert(RecommendedPlacementAreaMax == vec2(1824.0f, 864.0f));

		static_assert(TickToDistance(TimelineTick::FromBeats(1)) == PlacementDistancePerBeat);
		static_assert(DistanceToTick(PlacementDistancePerBeat) == TimelineTick::FromBeats(1));
		static_assert(DistanceToTick(PlacementDistancePerBeat / 4.0f) == (TimelineTick::FromBeats(1) / 4));
		static_assert(DistanceToTick(TickToDistance(TimelineTick::FromTicks(1))) == TimelineTick::FromTicks(1));
		static_assert(DistanceToTick(TickToDistance(TimelineTick::FromBeats(1))) == TimelineTick::FromBeats(1));
		static_assert(DistanceToTick(TickToDistance(TimelineTick::FromBars(1))) == TimelineTick::FromBars(1));

		enum class AngleCorner
		{
			TopRight,
			BotRight,
			BotLeft,
			TopLeft,
			Count
		};

		constexpr std::array<f32, EnumCount<AngleCorner>()> VerticalSyncPairAngles =
		{
			+45.0f,
			+135.0f,
			-135.0f,
			-45.0f,
		};

		constexpr std::array<f32, EnumCount<AngleCorner>()> VerticalSyncPairAnglesSteep =
		{
			+35.0f,
			+145.0f,
			-145.0f,
			-35.0f,
		};

		constexpr std::array<f32, EnumCount<AngleCorner>()> HorizontalSyncPairAngles =
		{
			+20.0f,
			+160.0f,
			-160.0f,
			-20.0f,
		};

		inline vec2 PresetTargetPosition(ButtonType type, TimelineTick tick, TargetFlags flags)
		{
			static constexpr auto verticalSyncHeightFactors = std::array<f32, EnumCount<ButtonType>()>
			{
				-3.0f, -2.0f, -1.0f, -0.0f, -1.0f, -0.0f,
			};

			constexpr auto baselineX = TickToDistance(TimelineTick::FromBars(1) / 4);
			constexpr auto baselineY = TickToDistance(TimelineTick::FromBars(1));

			const auto x = TickToDistance((TimelineTick::FromBars(1) + tick) % TimelineTick::FromBars(2));
			const auto y = (flags.IsSync) ? (verticalSyncHeightFactors[static_cast<size_t>(type)] * VerticalSyncPairPlacementDistance) : 0.0f;

			return vec2(baselineX + x, baselineY + y);
		}

		inline TargetProperties PresetTargetProperties(ButtonType type, TimelineTick tick, TargetFlags flags, bool syncAnglesSteep = false)
		{
			TargetProperties result;
			result.Position = PresetTargetPosition(type, tick, flags);
			if (flags.IsSync)
			{
				const auto pairCount = flags.SyncPairCount;
				const auto pairIndex = flags.IndexWithinSyncPair;

				bool upperHalfOfPair = pairIndex < (pairCount / 2);
				if (pairCount == 3)
				{
					if (type == ButtonType::Square && pairIndex == 1)
						upperHalfOfPair = true;
					if (type == ButtonType::Cross)
						upperHalfOfPair = false;
				}

				const auto& anglesArray = syncAnglesSteep ? VerticalSyncPairAnglesSteep : VerticalSyncPairAngles;
				result.Angle = anglesArray[static_cast<size_t>(upperHalfOfPair ? AngleCorner::TopRight : AngleCorner::BotRight)];
				result.Frequency = 0;
				result.Amplitude = 500.0f;
				result.Distance = 880.0f;
			}
			else
			{
				result.Angle = 0.0f;
				result.Frequency = -2;
				result.Amplitude = 500.0f;
				result.Distance = 1200.0f;
			}
			return result;
		}
	}
}
