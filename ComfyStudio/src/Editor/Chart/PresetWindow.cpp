#include "PresetWindow.h"
#include "ChartCommands.h"
#include "TargetPropertyRules.h"
#include "ImGui/Extensions/ImGuiExtensions.h"

namespace Comfy::Studio::Editor
{
	// TODO: "Dynamic Sync Presets", "Static Sync Presets" and "Static Sequence Presets"
	// TODO: Context menu (?) settings for narrow vertical angles, inside-out square / triangle angles and different sync formation height offset

	PresetWindow::PresetWindow(Undo::UndoManager& undoManager) : undoManager(undoManager)
	{
	}

	namespace
	{
		constexpr vec2 AngleCornerTypeToSquarePlacementAreaCorner(Rules::AngleCorner corner)
		{
			constexpr f32 top = 0.0f, bot = Rules::PlacementAreaSize.y - Rules::SyncFormationHeightOffset, left = 0.0f, right = Rules::PlacementAreaSize.x;
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

		constexpr bool ApplyDynamicSyncPresetToSyncPair(const DynamicSyncPreset preset, const TimelineTarget* const syncPair, const i32 pairCount, ApplySyncPreset::Data* outProperties, const i32 masterIndex)
		{
			assert(syncPair != nullptr && pairCount > 1 && outProperties != nullptr && masterIndex >= 0 && masterIndex < pairCount);
			using namespace Rules;

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
				for (i32 i = 0; i < pairCount; i++)
				{
					const auto& slaveTarget = syncPair[i];
					auto& slaveProperties = outProperties[i].NewValue;

					if (&slaveTarget != &masterTarget)
					{
						const auto heightDistanceToMaster = static_cast<f32>(static_cast<u8>(masterTarget.Type) - static_cast<u8>(slaveTarget.Type)) * VerticalSyncPairPlacementDistance;

						slaveProperties.Position.x = masterProperties.Position.x;
						slaveProperties.Position.y = masterProperties.Position.y - heightDistanceToMaster;
					}

					slaveProperties.Angle = Detail::IsUpperPartOfSyncPair(slaveTarget.Type, slaveTarget.Flags) ? 45.0f : 135.0f;
					if (preset == DynamicSyncPreset::VerticalLeft)
						slaveProperties.Angle *= -1.0f;
				}
				return true;
			}
			case DynamicSyncPreset::HorizontalUp:
			case DynamicSyncPreset::HorizontalDown:
			{
				// TODO: Special treatment for sync slides, maybe detect which kind of preset to choose based on existing position (?)
				masterProperties.Position.x = HorizontalSyncPairPositionsX[static_cast<size_t>(masterTarget.Type)];

				for (i32 i = 0; i < pairCount; i++)
				{
					const auto& slaveTarget = syncPair[i];
					auto& slaveProperties = outProperties[i].NewValue;

					if (&slaveTarget != &masterTarget)
					{
						slaveProperties.Position.x = HorizontalSyncPairPositionsX[static_cast<size_t>(slaveTarget.Type)];
						slaveProperties.Position.y = masterProperties.Position.y;
					}

					if (preset == DynamicSyncPreset::HorizontalUp)
						slaveProperties.Angle = Detail::IsUpperPartOfSyncPair(slaveTarget.Type, slaveTarget.Flags) ? -20.0f : +20.0f;
					else
						slaveProperties.Angle = Detail::IsUpperPartOfSyncPair(slaveTarget.Type, slaveTarget.Flags) ? -160.0f : +160.0f;
				}
				return true;
			}
			case DynamicSyncPreset::Square:
			{
				if (!DoTypesMatch(syncPair, pairCount, std::array { ButtonType::Triangle, ButtonType::Square, ButtonType::Cross, ButtonType::Circle }))
					return false;

				constexpr std::array<AngleCorner, EnumCount<ButtonType>()> typeToCorner = { AngleCorner::TopLeft, AngleCorner::TopRight, AngleCorner::BotLeft, AngleCorner::BotRight, };
				constexpr std::array<f32, EnumCount<AngleCorner>()> cornerToAngle = { +45.0f, +135.0f, -135.0f, -45.0f, };

				constexpr auto minCornerDistance = (AngleCornerTypeToSquarePlacementAreaCorner(AngleCorner::BotRight) - VerticalSyncPairPlacementDistance) / 2.0f;

				const auto masterCorner = typeToCorner[static_cast<size_t>(masterTarget.Type)];
				const auto masterCornerPosition = AngleCornerTypeToSquarePlacementAreaCorner(masterCorner);
				const auto masterCornerDistance = glm::min(glm::abs(masterCornerPosition - masterProperties.Position), minCornerDistance);

				for (i32 i = 0; i < pairCount; i++)
				{
					const auto& slaveTarget = syncPair[i];
					const auto slaveCorner = typeToCorner[static_cast<size_t>(slaveTarget.Type)];
					auto& slaveProperties = outProperties[i].NewValue;

					slaveProperties.Angle = cornerToAngle[static_cast<size_t>(slaveCorner)];
					slaveProperties.Position = AngleCornerTypeToSquarePlacementAreaCorner(slaveCorner);

					switch (slaveCorner)
					{
					case AngleCorner::TopRight: slaveProperties.Position += vec2(-masterCornerDistance.x, +masterCornerDistance.y); break;
					case AngleCorner::BotRight: slaveProperties.Position += vec2(-masterCornerDistance.x, -masterCornerDistance.y); break;
					case AngleCorner::BotLeft: slaveProperties.Position += vec2(+masterCornerDistance.x, -masterCornerDistance.y); break;
					case AngleCorner::TopLeft: slaveProperties.Position += vec2(+masterCornerDistance.x, +masterCornerDistance.y); break;
					}
				}
				return true;
			}
			case DynamicSyncPreset::Triangle:
			{
				const bool isLeftTriangle = DoTypesMatch(syncPair, pairCount, std::array { ButtonType::Triangle, ButtonType::Square, ButtonType::Cross });
				const bool isRightTriangle = DoTypesMatch(syncPair, pairCount, std::array { ButtonType::Square, ButtonType::Cross, ButtonType::Circle });

				if (!isLeftTriangle && !isRightTriangle)
					return false;

				constexpr f32 placementBot = (PlacementAreaSize.y - SyncFormationHeightOffset);
				const bool isMasterUpperRow = (masterProperties.Position.y < (placementBot / 2.0f));

				const auto masterHeightDistance = isMasterUpperRow ? masterProperties.Position.y : (placementBot - masterProperties.Position.y);

				for (i32 i = 0; i < pairCount; i++)
				{
					const auto& slaveTarget = syncPair[i];
					auto& slaveProperties = outProperties[i].NewValue;

					const bool isSlaveUpperRow = IsButtonTypeOnSameTrianglePresetRow(slaveTarget.Type, masterTarget.Type, isLeftTriangle) ? isMasterUpperRow : !isMasterUpperRow;

					slaveProperties.Position.x = HorizontalSyncPairPositionsX[static_cast<size_t>(slaveTarget.Type)];
					slaveProperties.Position.y = (isSlaveUpperRow) ? masterHeightDistance : (placementBot - masterHeightDistance);
					slaveProperties.Angle = TrianglePresetAngle(slaveTarget.Type, isLeftTriangle, isSlaveUpperRow);
				}
				return true;
			}
			}

			assert(false);
			return false;
		}

		void ApplyDynamicSyncPresetsToSelectedTargets(Undo::UndoManager& undoManager, Chart& chart, DynamicSyncPreset preset)
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
				if (firstTargetOfPair.Flags.IsSync)
				{
					i32 masterSelectionIndexWithinPair = -1;

#if 0 // TODO: Should it be the first or the last (?)
					for (i32 pair = firstTargetOfPair.Flags.SyncPairCount - 1; pair >= 0; pair--)
#else
					for (i32 pair = 0; pair < firstTargetOfPair.Flags.SyncPairCount; pair++)
#endif
					{
						if (chart.Targets[i + pair].IsSelected)
							masterSelectionIndexWithinPair = pair;
					}

					// TODO: Or maybe let the preset pick its own master (based on position for example) if multiple within a pair are selected (?)
					if (masterSelectionIndexWithinPair > -1)
					{
						const auto targetDataStartIndex = targetData.size();
						for (i32 j = 0; j < firstTargetOfPair.Flags.SyncPairCount; j++)
						{
							auto& data = targetData.emplace_back();
							data.TargetIndex = (i + j);
							data.NewValue = Rules::TryGetProperties(chart.Targets[i + j]);
						}

						const bool failedToApply = !ApplyDynamicSyncPresetToSyncPair(preset, &firstTargetOfPair, firstTargetOfPair.Flags.SyncPairCount, &targetData[targetDataStartIndex], masterSelectionIndexWithinPair);
						if (failedToApply)
						{
							for (i32 j = 0; j < firstTargetOfPair.Flags.SyncPairCount; j++)
								targetData.pop_back();
						}
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
	}

	void PresetWindow::Gui(Chart& chart)
	{
		Gui::BeginChild("PresetsChild", {}, true);
		{
			const auto halfWidth = Gui::GetContentRegionAvailWidth() / 2.0f;
			const auto buttonHeight = 46.0f;

			// NOTE: Delay icon rendering to optimize texture batching
			std::array<ImRect, EnumCount<DynamicSyncPreset>()> presetIconRectsToDraw;

			auto dynamicSyncPresetButton = [&](DynamicSyncPreset preset)
			{
				Gui::PushID(static_cast<int>(preset));
				if (Gui::ButtonEx("##DynamicSyncPresetButton", vec2(halfWidth, buttonHeight)))
					ApplyDynamicSyncPresetsToSelectedTargets(undoManager, chart, preset);
				Gui::PopID();

				if (Gui::IsItemHovered())
				{
					// TODO: Hover preview in TargetRenderWindow (?)
				}

				const auto rect = [rect = Gui::FitFixedAspectRatio(Gui::GetCurrentWindowRead()->DC.LastItemRect, 1.0f)]() { auto r = rect; r.Expand(-4.0f); return r; }();
				presetIconRectsToDraw[static_cast<size_t>(preset)] = rect;
			};

			const bool anySyncTargetSelected = std::any_of(chart.Targets.begin(), chart.Targets.end(), [](auto& t) { return (t.IsSelected && t.Flags.IsSync); });

			Gui::PushStyleVar(ImGuiStyleVar_ItemSpacing, vec2(2.0f, 2.0f));
			Gui::PushItemDisabledAndTextColorIf(!anySyncTargetSelected);
			{
				for (size_t i = 0; i < EnumCount<DynamicSyncPreset>(); i += 2)
				{
					dynamicSyncPresetButton(static_cast<DynamicSyncPreset>(i + 0));
					Gui::SameLine();
					dynamicSyncPresetButton(static_cast<DynamicSyncPreset>(i + 1));
				}

				for (size_t i = 0; i < EnumCount<DynamicSyncPreset>(); i++)
				{
					const auto iconRect = presetIconRectsToDraw[i];
					const auto iconSpr = IndexOr(i, sprites.DynamicSyncPresetIcons, nullptr);

					if (iconSpr != nullptr)
						Gui::AddSprite(Gui::GetWindowDrawList(), *editorSprites, *iconSpr, iconRect.GetTL(), iconRect.GetBR(), Gui::GetColorU32(ImGuiCol_Text));
				}
			}
			Gui::PopItemDisabledAndTextColorIf(!anySyncTargetSelected);
			Gui::PopStyleVar();
		}
		Gui::EndChild();
	}

	void PresetWindow::OnEditorSpritesLoaded(const Graphics::SprSet* sprSet)
	{
		editorSprites = sprSet;
		if (editorSprites == nullptr)
			return;

		auto findSprite = [this](std::string_view spriteName)
		{
			const auto found = FindIfOrNull(editorSprites->Sprites, [&](const auto& spr) { return spr.Name == spriteName; });
			return (found != nullptr && InBounds(found->TextureIndex, editorSprites->TexSet.Textures)) ? found : nullptr;
		};

		sprites.DynamicSyncPresetIcons[static_cast<size_t>(DynamicSyncPreset::VerticalLeft)] = findSprite("SYNC_PRESET_ICON_VERTICAL_LEFT");
		sprites.DynamicSyncPresetIcons[static_cast<size_t>(DynamicSyncPreset::VerticalRight)] = findSprite("SYNC_PRESET_ICON_VERTICAL_RIGHT");
		sprites.DynamicSyncPresetIcons[static_cast<size_t>(DynamicSyncPreset::HorizontalUp)] = findSprite("SYNC_PRESET_ICON_HORIZONTAL_UP");
		sprites.DynamicSyncPresetIcons[static_cast<size_t>(DynamicSyncPreset::HorizontalDown)] = findSprite("SYNC_PRESET_ICON_HORIZONTAL_DOWN");
		sprites.DynamicSyncPresetIcons[static_cast<size_t>(DynamicSyncPreset::Square)] = findSprite("SYNC_PRESET_ICON_SQUARE");
		sprites.DynamicSyncPresetIcons[static_cast<size_t>(DynamicSyncPreset::Triangle)] = findSprite("SYNC_PRESET_ICON_TRIANGLE");
	}
}
