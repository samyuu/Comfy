#include "TargetPropertyPresets.h"
#include "ChartCommands.h"

namespace Comfy::Studio::Editor
{
	namespace
	{
		constexpr bool IsAnyTargetWithinSyncPairSelected(const TimelineTarget* const syncPair, const i32 pairCount)
		{
			assert(syncPair != nullptr && pairCount > 1 && syncPair[0].Flags.IndexWithinSyncPair == 0);

			for (i32 i = 0; i < pairCount; i++)
			{
				if (syncPair[i].IsSelected)
					return true;
			}

			return false;
		}

		constexpr vec2 AngleCornerTypeToSquarePlacementAreaCorner(Rules::AngleCorner corner, const DynamicSyncPresetSettings& settings)
		{
			const f32 top = 0.0f;
			const f32 left = 0.0f;
			const f32 bot = Rules::PlacementAreaSize.y - (settings.ElevateBottomRow ? Rules::SyncFormationHeightOffsetAlt : Rules::SyncFormationHeightOffset);
			const f32 right = Rules::PlacementAreaSize.x;

			switch (corner)
			{
			case Rules::AngleCorner::TopRight: return vec2(right, top);
			case Rules::AngleCorner::BotRight: return vec2(right, bot);
			case Rules::AngleCorner::BotLeft: return vec2(left, bot);
			case Rules::AngleCorner::TopLeft: return vec2(left, top);
			}

			assert(false);
			return {};
		}

		constexpr f32 TrianglePresetAngle(ButtonType type, bool leftTriangle, bool upperRow)
		{
			if (leftTriangle)
			{
				switch (type)
				{
				case ButtonType::Triangle:
					return upperRow ? -45.0f : -135.0f;
				case ButtonType::Square:
					return upperRow ? +0.0f : +180.0f;
				case ButtonType::Cross:
					return upperRow ? +45.0f : +135.0f;
				}
			}
			else
			{
				switch (type)
				{
				case ButtonType::Square:
					return upperRow ? -45.0f : -135.0f;
				case ButtonType::Cross:
					return upperRow ? +0.0f : +180.0f;
				case ButtonType::Circle:
					return upperRow ? +45.0f : +135.0f;
				}
			}

			assert(false);
			return 0.0f;
		}

		constexpr bool IsButtonTypeOnSameTrianglePresetRow(ButtonType typeA, ButtonType typeB, bool isLeftTriangle)
		{
			if (typeA == typeB)
				return true;

			switch (typeA)
			{
			case ButtonType::Triangle: return (isLeftTriangle && typeB == ButtonType::Cross);
			case ButtonType::Square: return (!isLeftTriangle && typeB == ButtonType::Circle);
			case ButtonType::Cross: return (isLeftTriangle && typeB == ButtonType::Triangle);
			case ButtonType::Circle: return (!isLeftTriangle && typeB == ButtonType::Square);
			}
			assert(false);
			return false;
		}

		template <typename ArrayType>
		constexpr bool DoTypesMatch(const TimelineTarget* const syncPair, const i32 pairCount, const ArrayType& expectedTypesArray)
		{
			if (pairCount != expectedTypesArray.size())
				return false;

			for (size_t i = 0; i < expectedTypesArray.size(); i++)
			{
				if (syncPair[i].Type != expectedTypesArray[i])
					return false;
			}
			return true;
		}

		constexpr i32 DetermineDynamicSyncPresetMasterIndex(const DynamicSyncPreset preset, const DynamicSyncPresetSettings& settings, const TimelineTarget* const syncPair, const i32 pairCount)
		{
			// TODO: Dynamic behavior based on preset type and existing positions (?)
			//		 If for example all 4 targets of a square preset are selected and are positioned in a screen corner,
			//		 then the furthest from the center should be chosen to avoid creating a minimum sized square (?)
			for (i32 i = pairCount - 1; i >= 0; i--)
			{
				if (syncPair[i].IsSelected)
					return i;
			}

			return 0;
		}

		bool ApplyDynamicSyncPresetToSyncPair(const DynamicSyncPreset preset, const DynamicSyncPresetSettings& settings, const TimelineTarget* const syncPair, const i32 pairCount, ApplySyncPreset::Data* outProperties)
		{
			assert(syncPair != nullptr && pairCount > 1 && outProperties != nullptr);
			using namespace Rules;

			const i32 masterIndex = DetermineDynamicSyncPresetMasterIndex(preset, settings, syncPair, pairCount);
			assert(masterIndex >= 0 && masterIndex < pairCount);

			const auto& masterTarget = syncPair[masterIndex];
			auto& masterProperties = outProperties[masterIndex].NewValue;

			for (i32 i = 0; i < pairCount; i++)
			{
				auto& properties = outProperties[i].NewValue;
				properties.Frequency = 0.0f;
				properties.Amplitude = 500.0f;
				properties.Distance = 880.0f;
			}

			switch (preset)
			{
			case DynamicSyncPreset::VerticalLeft:
			case DynamicSyncPreset::VerticalRight:
			{
				const f32 upperAngle = (settings.SteepAngles ? 35.0f : 45.0f);
				const f32 lowerAngle = NormalizeAngle(180.0f - upperAngle);

				const auto masterTargetType = Detail::AdjustSameTypeSyncButtonType(masterTarget.Type, masterTarget.Flags);

				for (i32 i = 0; i < pairCount; i++)
				{
					const auto& slaveTarget = syncPair[i];
					auto& slaveProperties = outProperties[i].NewValue;

					const auto slaveTargetType = Detail::AdjustSameTypeSyncButtonType(slaveTarget.Type, slaveTarget.Flags);

					if (&slaveTarget != &masterTarget)
					{
						const auto heightDistanceToMaster = static_cast<f32>(static_cast<u8>(masterTargetType) - static_cast<u8>(slaveTargetType)) * VerticalSyncPairPlacementDistance;

						slaveProperties.Position.x = masterProperties.Position.x;
						slaveProperties.Position.y = masterProperties.Position.y - heightDistanceToMaster;
					}

					if (settings.SameDirectionAngles)
						slaveProperties.Angle = Detail::IsUpperPartOfSyncPair(masterTargetType, masterTarget.Flags) ? upperAngle : lowerAngle;
					else
						slaveProperties.Angle = Detail::IsUpperPartOfSyncPair(slaveTargetType, slaveTarget.Flags) ? upperAngle : lowerAngle;

					if (preset == DynamicSyncPreset::VerticalLeft)
						slaveProperties.Angle *= -1.0f;
				}
				break;
			}
			case DynamicSyncPreset::HorizontalUp:
			case DynamicSyncPreset::HorizontalDown:
			{
				const f32 rightAngle = (settings.SteepAngles ? 10.0f : 20.0f);
				const f32 leftAngle = -rightAngle;

				const bool useSingleDirection = (masterTarget.Flags.SameTypeSyncCount > 1);
				const bool singleDirectionLeft = (masterTarget.Type == ButtonType::SlideL) ? false : (masterTarget.Type == ButtonType::SlideR) ? true : (masterProperties.Position.x > PlacementAreaCenter.x);

				const bool useDynamicSlides = IsSlideButtonType(masterTarget.Type);
				const bool dynamicSlidesFlip = (masterTarget.Type == ButtonType::SlideL ? (masterProperties.Position.x > PlacementAreaCenter.x) : (masterProperties.Position.x < PlacementAreaCenter.x));
				const bool dynamicSlidesSmall = (masterProperties.Position.x >= (PlacementAreaSize.x / 4.0f) && masterProperties.Position.x <= ((PlacementAreaSize.x / 2.0f) + (PlacementAreaSize.x / 4.0f)));

				auto getHorizontalPositionX = [&](const TimelineTarget& target) -> f32
				{
					if (useSingleDirection)
						return HorizontalSyncPairPositionsX[((singleDirectionLeft ? 2 : 0) + (target.Flags.SameTypeSyncIndex % 2))];
					else if (useDynamicSlides)
						return HorizontalSyncPairPositionsX[((dynamicSlidesFlip ? FlipSlideButtonType(target.Type) : target.Type) == ButtonType::SlideL) ? (dynamicSlidesSmall ? 1 : 0) : (dynamicSlidesSmall ? 2 : 3)];
					else
						return HorizontalSyncPairPositionsX[static_cast<u8>(target.Type)];
				};

				masterProperties.Position.x = getHorizontalPositionX(masterTarget);

				for (i32 i = 0; i < pairCount; i++)
				{
					const auto& slaveTarget = syncPair[i];
					auto& slaveProperties = outProperties[i].NewValue;

					if (&slaveTarget != &masterTarget)
					{
						slaveProperties.Position.x = getHorizontalPositionX(slaveTarget);
						slaveProperties.Position.y = masterProperties.Position.y;
					}

					if (useSingleDirection)
						slaveProperties.Angle = singleDirectionLeft ? leftAngle : rightAngle;
					else if (useDynamicSlides)
						slaveProperties.Angle = (slaveTarget.Type == ButtonType::SlideR) ? leftAngle : rightAngle;
					else
					{
						if (settings.SameDirectionAngles)
							slaveProperties.Angle = Detail::IsUpperPartOfSyncPair(masterTarget.Type, masterTarget.Flags) ? leftAngle : rightAngle;
						else
							slaveProperties.Angle = Detail::IsUpperPartOfSyncPair(slaveTarget.Type, slaveTarget.Flags) ? leftAngle : rightAngle;
					}

					if (preset == DynamicSyncPreset::HorizontalDown)
						slaveProperties.Angle = NormalizeAngle(180.0f - slaveProperties.Angle);
				}
				break;
			}
			case DynamicSyncPreset::Square:
			{
				if (!DoTypesMatch(syncPair, pairCount, std::array { ButtonType::Triangle, ButtonType::Square, ButtonType::Cross, ButtonType::Circle }))
					return false;

				constexpr std::array<AngleCorner, EnumCount<ButtonType>()> typeToCorner = { AngleCorner::TopLeft, AngleCorner::TopRight, AngleCorner::BotLeft, AngleCorner::BotRight, };
				constexpr std::array<f32, EnumCount<AngleCorner>()> cornerToAngle = { +45.0f, +135.0f, -135.0f, -45.0f, };

				const vec2 minCornerDistance = (AngleCornerTypeToSquarePlacementAreaCorner(AngleCorner::BotRight, settings) - VerticalSyncPairPlacementDistance) / 2.0f;

				const auto masterCorner = typeToCorner[static_cast<size_t>(masterTarget.Type)];
				const vec2 masterCornerPosition = AngleCornerTypeToSquarePlacementAreaCorner(masterCorner, settings);
				const vec2 masterCornerDistance = glm::min(glm::abs(masterCornerPosition - masterProperties.Position), minCornerDistance);

				for (i32 i = 0; i < pairCount; i++)
				{
					const auto& slaveTarget = syncPair[i];
					const auto slaveCorner = typeToCorner[static_cast<size_t>(slaveTarget.Type)];
					auto& slaveProperties = outProperties[i].NewValue;

					slaveProperties.Angle = cornerToAngle[static_cast<size_t>(slaveCorner)];
					slaveProperties.Position = AngleCornerTypeToSquarePlacementAreaCorner(slaveCorner, settings);

					switch (slaveCorner)
					{
					case AngleCorner::TopRight: slaveProperties.Position += vec2(-masterCornerDistance.x, +masterCornerDistance.y); break;
					case AngleCorner::BotRight: slaveProperties.Position += vec2(-masterCornerDistance.x, -masterCornerDistance.y); break;
					case AngleCorner::BotLeft: slaveProperties.Position += vec2(+masterCornerDistance.x, -masterCornerDistance.y); break;
					case AngleCorner::TopLeft: slaveProperties.Position += vec2(+masterCornerDistance.x, +masterCornerDistance.y); break;
					}
				}
				break;
			}
			case DynamicSyncPreset::Triangle:
			{
				const bool isLeftTriangle = DoTypesMatch(syncPair, pairCount, std::array { ButtonType::Triangle, ButtonType::Square, ButtonType::Cross });
				const bool isRightTriangle = DoTypesMatch(syncPair, pairCount, std::array { ButtonType::Square, ButtonType::Cross, ButtonType::Circle });

				if (!isLeftTriangle && !isRightTriangle)
					return false;

				const f32 placementBot = PlacementAreaSize.y - (settings.ElevateBottomRow ? SyncFormationHeightOffsetAlt : SyncFormationHeightOffset);
				const bool isMasterUpperRow = (masterProperties.Position.y < (placementBot / 2.0f));

				const f32 masterHeightDistance = isMasterUpperRow ? masterProperties.Position.y : (placementBot - masterProperties.Position.y);

				for (i32 i = 0; i < pairCount; i++)
				{
					const auto& slaveTarget = syncPair[i];
					auto& slaveProperties = outProperties[i].NewValue;

					const bool isSlaveUpperRow = IsButtonTypeOnSameTrianglePresetRow(slaveTarget.Type, masterTarget.Type, isLeftTriangle) ? isMasterUpperRow : !isMasterUpperRow;

					slaveProperties.Position.x = HorizontalSyncPairPositionsX[static_cast<size_t>(slaveTarget.Type)];
					slaveProperties.Position.y = (isSlaveUpperRow) ? masterHeightDistance : (placementBot - masterHeightDistance);
					slaveProperties.Angle = TrianglePresetAngle(slaveTarget.Type, isLeftTriangle, isSlaveUpperRow);
				}
				break;
			}
			}

			if (settings.InsideOutAngles && (preset == DynamicSyncPreset::Square || preset == DynamicSyncPreset::Triangle))
			{
				for (i32 i = 0; i < pairCount; i++)
				{
					auto& properties = outProperties[i].NewValue;
					properties.Angle = NormalizeAngle(properties.Angle + 180.0f);
					properties.Distance = 1360.0f;
				}
			}

			return true;
		}

		bool ApplyStaticSyncPresetToSyncPair(const StaticSyncPreset& preset, const TimelineTarget* const syncPair, const i32 pairCount, ApplySyncPreset::Data* outProperties)
		{
			assert(syncPair != nullptr && pairCount > 1 && preset.TargetCount <= preset.Targets.size() && outProperties != nullptr);
			assert(std::is_sorted(preset.Targets.begin(), preset.Targets.begin() + pairCount, [](auto& a, auto& b) { return a.Type < b.Type; }));

			if (pairCount != preset.TargetCount)
				return false;

			for (size_t i = 0; i < pairCount; i++)
			{
				if (preset.Targets[i].Type != syncPair[i].Type)
					return false;
			}

			for (size_t i = 0; i < pairCount; i++)
				outProperties[i].NewValue = preset.Targets[i].Properties;

			return true;
		}
	}

	void ApplyDynamicSyncPresetToSelectedTargets(Undo::UndoManager& undoManager, Chart& chart, const DynamicSyncPreset preset, const DynamicSyncPresetSettings& settings)
	{
		size_t syncSelectionCount = 0;
		for (i32 i = 0; i < static_cast<i32>(chart.Targets.size());)
		{
			const auto& firstTargetOfPair = chart.Targets[i];
			if (firstTargetOfPair.Flags.IsSync)
			{
				for (i32 pair = 0; pair < firstTargetOfPair.Flags.SyncPairCount; pair++)
				{
					if (chart.Targets[i + pair].IsSelected)
					{
						syncSelectionCount += firstTargetOfPair.Flags.SyncPairCount;
						break;
					}
				}
			}

			assert(firstTargetOfPair.Flags.SyncPairCount >= 1);
			i += firstTargetOfPair.Flags.SyncPairCount;
		}

		if (syncSelectionCount < 1)
			return;

		std::vector<ApplySyncPreset::Data> targetData;
		targetData.reserve(syncSelectionCount);

		for (i32 i = 0; i < static_cast<i32>(chart.Targets.size());)
		{
			const auto& firstTargetOfPair = chart.Targets[i];
			if (firstTargetOfPair.Flags.IsSync && IsAnyTargetWithinSyncPairSelected(&firstTargetOfPair, firstTargetOfPair.Flags.SyncPairCount))
			{
				const auto targetDataStartIndex = targetData.size();
				for (i32 j = 0; j < firstTargetOfPair.Flags.SyncPairCount; j++)
				{
					auto& data = targetData.emplace_back();
					data.ID = chart.Targets[i + j].ID;
					data.NewValue = Rules::TryGetProperties(chart.Targets[i + j]);
				}

				const bool failedToApply = !ApplyDynamicSyncPresetToSyncPair(preset, settings, &firstTargetOfPair, firstTargetOfPair.Flags.SyncPairCount, &targetData[targetDataStartIndex]);
				if (failedToApply)
				{
					for (i32 j = 0; j < firstTargetOfPair.Flags.SyncPairCount; j++)
						targetData.pop_back();
				}
			}

			assert(firstTargetOfPair.Flags.SyncPairCount >= 1);
			i += firstTargetOfPair.Flags.SyncPairCount;
		}

		if (!targetData.empty())
		{
			undoManager.DisallowMergeForLastCommand();
			undoManager.Execute<ApplySyncPreset>(chart, std::move(targetData), TargetPropertyFlags_All);
		}
	}

	void ApplyStaticSyncPresetToSelectedTargets(Undo::UndoManager& undoManager, Chart& chart, const StaticSyncPreset& preset)
	{
		size_t syncSelectionCount = 0;
		for (i32 i = 0; i < static_cast<i32>(chart.Targets.size());)
		{
			const auto& firstTargetOfPair = chart.Targets[i];
			if (firstTargetOfPair.Flags.IsSync && IsAnyTargetWithinSyncPairSelected(&firstTargetOfPair, firstTargetOfPair.Flags.SyncPairCount))
				syncSelectionCount += firstTargetOfPair.Flags.SyncPairCount;

			assert(firstTargetOfPair.Flags.SyncPairCount >= 1);
			i += firstTargetOfPair.Flags.SyncPairCount;
		}

		if (syncSelectionCount < 1)
			return;

		std::vector<ApplySyncPreset::Data> targetData;
		targetData.reserve(syncSelectionCount);

		for (i32 i = 0; i < static_cast<i32>(chart.Targets.size());)
		{
			const auto& firstTargetOfPair = chart.Targets[i];
			if (firstTargetOfPair.Flags.IsSync && IsAnyTargetWithinSyncPairSelected(&firstTargetOfPair, firstTargetOfPair.Flags.SyncPairCount))
			{
				const auto targetDataStartIndex = targetData.size();
				for (i32 j = 0; j < firstTargetOfPair.Flags.SyncPairCount; j++)
				{
					auto& data = targetData.emplace_back();
					data.ID = chart.Targets[i + j].ID;
					data.NewValue = Rules::TryGetProperties(chart.Targets[i + j]);
				}

				const bool failedToApply = !ApplyStaticSyncPresetToSyncPair(preset, &firstTargetOfPair, firstTargetOfPair.Flags.SyncPairCount, &targetData[targetDataStartIndex]);
				if (failedToApply)
				{
					for (i32 j = 0; j < firstTargetOfPair.Flags.SyncPairCount; j++)
						targetData.pop_back();
				}
			}

			assert(firstTargetOfPair.Flags.SyncPairCount >= 1);
			i += firstTargetOfPair.Flags.SyncPairCount;
		}

		if (!targetData.empty())
		{
			undoManager.DisallowMergeForLastCommand();
			undoManager.Execute<ApplySyncPreset>(chart, std::move(targetData), TargetPropertyFlags_All);
		}
	}

	u32 FindFirstApplicableDynamicSyncPresetDataForSelectedTargets(const Chart& chart, const DynamicSyncPreset preset, const DynamicSyncPresetSettings& settings, std::array<PresetTargetData, Rules::MaxSyncPairCount>& outPresetTargets)
	{
		for (i32 i = 0; i < static_cast<i32>(chart.Targets.size());)
		{
			const auto& firstTargetOfPair = chart.Targets[i];
			if (firstTargetOfPair.Flags.IsSync && IsAnyTargetWithinSyncPairSelected(&firstTargetOfPair, firstTargetOfPair.Flags.SyncPairCount))
			{
				std::array<ApplySyncPreset::Data, Rules::MaxSyncPairCount> tempProperties;
				const auto syncPairCount = std::min<i32>(firstTargetOfPair.Flags.SyncPairCount, Rules::MaxSyncPairCount);

				for (i32 j = 0; j < syncPairCount; j++)
				{
					auto& data = tempProperties[j];
					data.ID = chart.Targets[i + j].ID;
					data.NewValue = Rules::TryGetProperties(chart.Targets[i + j]);
				}

				if (ApplyDynamicSyncPresetToSyncPair(preset, settings, &firstTargetOfPair, syncPairCount, tempProperties.data()))
				{
					for (i32 j = 0; j < syncPairCount; j++)
					{
						outPresetTargets[j].Type = chart.Targets[chart.Targets.FindIndex(tempProperties[j].ID)].Type;
						outPresetTargets[j].Properties = tempProperties[j].NewValue;
					}

					return syncPairCount;
				}
			}

			assert(firstTargetOfPair.Flags.SyncPairCount >= 1);
			i += firstTargetOfPair.Flags.SyncPairCount;
		}

		return 0;
	}

	void ApplySequencePresetToSelectedTargets(Undo::UndoManager& undoManager, Chart& chart, const SequencePreset& preset, const SequencePresetSettings& settings)
	{
		// TODO: ...

		const size_t selectionCount = std::count_if(chart.Targets.begin(), chart.Targets.end(), [](const auto& t) { return t.IsSelected; });
		if (selectionCount < 1)
			return;

		std::vector<ApplySequencePreset::Data> targetData;
		targetData.reserve(selectionCount);

		if (preset.Type == SequencePresetType::Circle)
		{
			const BeatTick tickOffset = ((settings.ApplyFirstTargetTickAsOffset) ?
				std::find_if(chart.Targets.begin(), chart.Targets.end(), [](auto& t) { return t.IsSelected; })->Tick : BeatTick::Zero()) - settings.TickOffset;

			for (const auto& target : chart.Targets)
			{
				if (!target.IsSelected)
					continue;

				auto& data = targetData.emplace_back();
				data.ID = target.ID;
				data.NewValue = Rules::TryGetProperties(target);

				const BeatTick circularTick = (target.Tick - tickOffset) % preset.Circle.Duration;
				const f32 angleForTargetTick = ((circularTick == BeatTick::Zero() ? 0.0f :
					static_cast<f32>(circularTick.Ticks()) / static_cast<f32>(preset.Circle.Duration.Ticks()) * 360.0f) * preset.Circle.Direction - 90.0f);

				data.NewValue.Position.x = preset.Circle.Center.x + (glm::cos(glm::radians(angleForTargetTick)) * preset.Circle.Radius);
				data.NewValue.Position.y = preset.Circle.Center.y + (glm::sin(glm::radians(angleForTargetTick)) * preset.Circle.Radius);
				data.NewValue.Angle = Rules::NormalizeAngle(angleForTargetTick + 90.0f);
			}
		}

		if (!targetData.empty())
		{
			undoManager.DisallowMergeForLastCommand();
			undoManager.Execute<ApplySequencePreset>(chart, std::move(targetData), TargetPropertyFlags_All);
		}
	}
}
