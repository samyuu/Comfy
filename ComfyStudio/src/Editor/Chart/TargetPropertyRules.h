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
		constexpr size_t MaxSyncPairCount = 4;

		constexpr vec2 PlacementAreaSize = { 1920.0f, 1080.0f };
		constexpr vec2 PlacementAreaCenter = (PlacementAreaSize * 0.5f);

		constexpr f32 PlacementDistancePerBeat = 192.0f;
		constexpr f32 PlacementDistancePerBeatStair = 186.59046f;

		// NOTE: Assumes chain fragments at a 32nd bar division
		constexpr f32 PlacementDistancePerBeatChain = 288.0f;

		constexpr vec2 PlacementStairDirection = (vec2(5.0f, 3.0f) / 5.83095169f);

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

		constexpr f32 TickToDistanceStair(const TimelineTick tick)
		{
			const auto beats = static_cast<f64>(tick.Ticks()) / static_cast<f64>(TimelineTick::TicksPerBeat);
			const auto beatDistanceRatio = static_cast<f32>(PlacementDistancePerBeatStair);

			return static_cast<f32>(beats * beatDistanceRatio);
		}

		constexpr f32 TickToDistanceChain(const TimelineTick tick)
		{
			const auto beats = static_cast<f64>(tick.Ticks()) / static_cast<f64>(TimelineTick::TicksPerBeat);
			const auto beatDistanceRatio = static_cast<f32>(PlacementDistancePerBeatChain);

			return static_cast<f32>(beats * beatDistanceRatio);
		}

		constexpr f32 ChainFragmentPlacementDistance = TickToDistanceChain(TimelineTick::FromBars(1) / 32);
		constexpr f32 ChainFragmentStartEndOffsetDistance = 32.0f;

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

		constexpr f32 SyncFormationHeightOffset = TickToDistance(TimelineTick::FromBars(1) / 32);
		constexpr f32 SyncFormationHeightOffsetAlt = TickToDistance(TimelineTick::FromBars(1) / 8 + TimelineTick::FromBars(1) / 32);

		static_assert(RecommendedPlacementAreaMin == vec2(96.0f, 192.0f));
		static_assert(RecommendedPlacementAreaMax == vec2(1824.0f, 864.0f));

		static_assert(TickToDistance(TimelineTick::FromBeats(1)) == PlacementDistancePerBeat);
		static_assert(DistanceToTick(PlacementDistancePerBeat) == TimelineTick::FromBeats(1));
		static_assert(DistanceToTick(PlacementDistancePerBeat / 4.0f) == (TimelineTick::FromBeats(1) / 4));
		static_assert(DistanceToTick(TickToDistance(TimelineTick::FromTicks(1))) == TimelineTick::FromTicks(1));
		static_assert(DistanceToTick(TickToDistance(TimelineTick::FromBeats(1))) == TimelineTick::FromBeats(1));
		static_assert(DistanceToTick(TickToDistance(TimelineTick::FromBars(1))) == TimelineTick::FromBars(1));

		enum class AngleCorner : u8
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

		constexpr std::array<f32, EnumCount<ButtonType>()> HorizontalSyncPairPositionsX =
		{
			240.0f,
			720.0f,
			1200.0f,
			1680.0f,
			240.0f,
			1680.0f,
		};

		namespace Detail
		{
			constexpr std::array<f32, EnumCount<ButtonType>()> VerticalSyncPairTypeHeightOffsets =
			{
				VerticalSyncPairPlacementDistance * -3.0f,
				VerticalSyncPairPlacementDistance * -2.0f,
				VerticalSyncPairPlacementDistance * -1.0f,
				VerticalSyncPairPlacementDistance * -0.0f,
				VerticalSyncPairPlacementDistance * -1.0f,
				VerticalSyncPairPlacementDistance * -0.0f,
			};

			constexpr vec2 PresetTargetPosition(ButtonType type, TimelineTick tick, TargetFlags flags)
			{
				constexpr auto baselineX = TickToDistance(TimelineTick::FromBars(1) / 4);
				constexpr auto baselineY = TickToDistance(TimelineTick::FromBars(1));

				const auto x = TickToDistance((TimelineTick::FromBars(1) + tick) % TimelineTick::FromBars(2));
				const auto y = flags.IsSync ? VerticalSyncPairTypeHeightOffsets[static_cast<size_t>(type)] : 0.0f;

				return vec2(baselineX + x, baselineY + y);
			}

			constexpr vec2 PresetTargetChainPosition(ButtonType type, TimelineTick tick, TargetFlags flags)
			{
				const bool isLeft = (type == ButtonType::SlideL);
				const bool evenBar = (tick % TimelineTick::FromBars(2)) < TimelineTick::FromBars(1);

				constexpr auto baselineXOdd = TickToDistance(TimelineTick::FromBeats(1) / 2);
				constexpr auto baselineXEven = TickToDistance(TimelineTick::FromBars(1) - TimelineTick::FromBars(1) / 8);

				constexpr auto baselineY = TickToDistance(TimelineTick::FromBars(1));

				const auto x = (evenBar ? baselineXEven : baselineXOdd) + (TickToDistanceChain(tick % TimelineTick::FromBars(1)));
				const auto y = (VerticalSyncPairPlacementDistance * (evenBar ? -2.0f : -1.0f));

				return vec2(isLeft ? (PlacementAreaSize.x - x) : x, baselineY + y);
			}

			constexpr bool IsUpperPartOfSyncPair(const ButtonType type, const TargetFlags flags)
			{
				if (flags.IndexWithinSyncPair < (flags.SyncPairCount / 2))
					return true;

				if (flags.SyncPairCount == 3)
				{
					if (type == ButtonType::Square && flags.IndexWithinSyncPair == 1)
						return true;
					if (type == ButtonType::Cross)
						return false;
				}

				return false;
			}

			constexpr TargetProperties PresetTargetProperties(ButtonType type, TimelineTick tick, TargetFlags flags)
			{
				if (flags.IsChain)
					return { Detail::PresetTargetChainPosition(type, tick, flags), 0.0f, (type == ButtonType::SlideL) ? +2.0f : -2.0f, 500.0f, 1200.0f, };

				if (!flags.IsSync)
					return { Detail::PresetTargetPosition(type, tick, flags), 0.0f, -2.0f, 500.0f, 1200.0f, };

				const bool upperHalfOfPair = IsUpperPartOfSyncPair(type, flags);
				const auto angle = VerticalSyncPairAngles[static_cast<size_t>(upperHalfOfPair ? AngleCorner::TopRight : AngleCorner::BotRight)];
				return { Detail::PresetTargetPosition(type, tick, flags), angle, 0.0f, 500.0f, 880.0f, };
			}

			constexpr TargetProperties PresetTargetProperties(const TimelineTarget& target)
			{
				return PresetTargetProperties(target.Type, target.Tick, target.Flags);
			}
		}

		constexpr TargetProperties TryGetProperties(const TimelineTarget& target)
		{
			return target.Flags.HasProperties ? target.Properties : Detail::PresetTargetProperties(target.Type, target.Tick, target.Flags);
		}

		inline f32 NormalizeAngle(const f32 degrees)
		{
			if (!std::isnormal(degrees))
				return 0.0f;

			auto normalized = glm::mod(degrees + 180.0f, 360.0f);
			if (normalized < 0.0f)
				normalized += 360.0f;
			normalized -= 180.0f;

			if (normalized == -180.0f)
				return -normalized;
			else
				return normalized;
		}
	}
}
