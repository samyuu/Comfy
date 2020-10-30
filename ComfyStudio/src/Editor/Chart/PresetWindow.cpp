#include "PresetWindow.h"
#include "ChartCommands.h"
#include "TargetPropertyRules.h"
#include "ImGui/Extensions/ImGuiExtensions.h"

namespace Comfy::Studio::Editor
{
	namespace
	{
		template <size_t TargetCount>
		StaticSyncPreset ConstructStaticSyncPreset(std::string name, std::array<StaticSyncPreset::Data, TargetCount> targetData)
		{
			StaticSyncPreset result = {};
			result.Name = std::move(name);
			result.TargetCount = static_cast<u32>(TargetCount);
			for (size_t i = 0; i < TargetCount; i++)
				result.Targets[i] = targetData[i];
			return result;
		}

		std::vector<StaticSyncPreset> GetTestStaticSyncPresets()
		{
			std::vector<StaticSyncPreset> presets;
			presets.reserve(8);
			presets.push_back(ConstructStaticSyncPreset("Smallest Sideways Square", std::array
				{
					StaticSyncPreset::Data { ButtonType::Triangle, { vec2(960.0f, 480.0f), 0.0f, 0.0f, 500.0f, 1040.0f } },
					StaticSyncPreset::Data { ButtonType::Square, { vec2(864.0f, 576.0f), -90.0f, 0.0f, 500.0f, 1040.0f } },
					StaticSyncPreset::Data { ButtonType::Cross, { vec2(960.0f, 672.0f), 180.0f, 0.0f, 500.0f, 1040.0f } },
					StaticSyncPreset::Data { ButtonType::Circle, { vec2(1056.0f, 576.0f), +90.0f, 0.0f, 500.0f, 1040.0f } },
				}));
			presets.push_back(ConstructStaticSyncPreset("Small Sideways Square", std::array
				{
					StaticSyncPreset::Data { ButtonType::Triangle, { vec2(960.0f, 336.0f), 0.0f, 0.0f, 500.0f, 960.0f } },
					StaticSyncPreset::Data { ButtonType::Square, { vec2(744.0f, 552.0f), -90.0f, 0.0f, 500.0f, 960.0f } },
					StaticSyncPreset::Data { ButtonType::Cross, { vec2(960.0f, 768.0f), 180.0f, 0.0f, 500.0f, 960.0f } },
					StaticSyncPreset::Data { ButtonType::Circle, { vec2(1176.0f, 552.0f), 90.0f, 0.0f, 500.0f, 960.0f } },
				}));
			presets.push_back(ConstructStaticSyncPreset("Large Sideways Square", std::array
				{
					StaticSyncPreset::Data { ButtonType::Triangle, { vec2(960.0f, 192.0f), 0.0f, 0.0f, 500.0f, 800.0f } },
					StaticSyncPreset::Data { ButtonType::Square, { vec2(384.0f, 528.0f), -90.0f, 0.0f, 500.0f, 800.0f } },
					StaticSyncPreset::Data { ButtonType::Cross, { vec2(960.0f, 864.0f), 180.0f, 0.0f, 500.0f, 800.0f } },
					StaticSyncPreset::Data { ButtonType::Circle, { vec2(1536.0f, 528.0f), 90.0f, 0.0f, 500.0f, 800.0f } },
				}));

			presets.push_back(ConstructStaticSyncPreset("Curved Small Square", std::array
				{
					StaticSyncPreset::Data { ButtonType::Triangle, { vec2(912.0f, 480.0f), -45.0f, 1.0f, 3000.0f, 2000.0f } },
					StaticSyncPreset::Data { ButtonType::Square, { vec2(1008.0f, 480.0f),45.0f, -1.0f, 3000.0f, 2000.0f } },
					StaticSyncPreset::Data { ButtonType::Cross, { vec2(912.0f, 576.0f), -135.0f, -1.0f, 3000.0f, 2000.0f } },
					StaticSyncPreset::Data { ButtonType::Circle, { vec2(1008.0f, 576.0f), 135.0f, 1.0f, 3000.0f, 2000.0f } },
				}));

			presets.push_back(ConstructStaticSyncPreset("Parallelogram", std::array
				{
					StaticSyncPreset::Data { ButtonType::Triangle, { vec2(240.0f, 240.0f), -90.0f, 0.0f, 500.0f, 880.0f } },
					StaticSyncPreset::Data { ButtonType::Square, { vec2(720.0f, 768.0f), -90.0f, 0.0f, 500.0f, 880.0f } },
					StaticSyncPreset::Data { ButtonType::Cross, { vec2(1200.0f, 240.0f), 90.0f, 0.0f, 500.0f, 880.0f } },
					StaticSyncPreset::Data { ButtonType::Circle, { vec2(1680.0f, 768.0f), 90.0f, 0.0f, 500.0f, 880.0f } },
				}));

			presets.push_back(ConstructStaticSyncPreset("Fancy Square Stuff [0]", std::array
				{
					StaticSyncPreset::Data { ButtonType::Triangle, { vec2(960, 432), 45.0f, -2.0f, 1000.0f, 1040.0f } },
					StaticSyncPreset::Data { ButtonType::Square, { vec2(864, 528), -45.0f, -2.0f, 1000.0f, 1040.0f } },
					StaticSyncPreset::Data { ButtonType::Cross, { vec2(960, 624), -135.0f, -2.0f, 1000.0f, 1040.0f } },
					StaticSyncPreset::Data { ButtonType::Circle, { vec2(1056, 528), 135.0f, -2.0f, 1000.0f, 1040.0f } },
				}));
			presets.push_back(ConstructStaticSyncPreset("Fancy Square Stuff [1]", std::array
				{
					StaticSyncPreset::Data { ButtonType::Triangle, { vec2(960.0f, 336.0f), 90.0f, -2.0f, 1000.0f, 1040.0f } },
					StaticSyncPreset::Data { ButtonType::Square, { vec2(768.0f, 528.0f), 0.0f, -2.0f, 1000.0f, 1040.0f } },
					StaticSyncPreset::Data { ButtonType::Cross, { vec2(960.0f, 720.0f), -90.0f, -2.0f, 1000.0f, 1040.0f } },
					StaticSyncPreset::Data { ButtonType::Circle, { vec2(1152.0f, 528.0f), 180.0f, -2.0f, 1000.0f, 1040.0f } },
				}));
			presets.push_back(ConstructStaticSyncPreset("Fancy Square Stuff [2]", std::array
				{
					StaticSyncPreset::Data { ButtonType::Triangle, { vec2(960, 168), 165.0f, -2.0f, 1000.0f, 2080.0f } },
					StaticSyncPreset::Data { ButtonType::Square, { vec2(600, 528), 75.0f, -2.0f, 1000.0f, 2080.0f } },
					StaticSyncPreset::Data { ButtonType::Cross, { vec2(960, 888), -15.0f, -2.0f, 1000.0f, 2080.0f } },
					StaticSyncPreset::Data { ButtonType::Circle, { vec2(1320, 528), -105.0f, -2.0f, 1000.0f, 2080.0f } },
				}));
			return presets;
		}
	}

	// TODO: Context menu (?) settings for narrow vertical angles, inside-out square / triangle angles and different sync formation height offset
	// struct DynamicSyncPresetSettings { ... };

	PresetWindow::PresetWindow(Undo::UndoManager& undoManager) : undoManager(undoManager), staticSyncPresets(GetTestStaticSyncPresets())
	{
	}

	void PresetWindow::SyncGui(Chart& chart)
	{
		const auto& style = Gui::GetStyle();

		constexpr auto dynamicButtonHeight = 46.0f;
		constexpr auto dynamicButtonSpacing = vec2(2.0f);
		constexpr auto staticButtonHeight = 26.0f;

		const bool anySyncTargetSelected = std::any_of(chart.Targets.begin(), chart.Targets.end(), [](auto& t) { return (t.IsSelected && t.Flags.IsSync); });
		Gui::PushItemDisabledAndTextColorIf(!anySyncTargetSelected);

		Gui::BeginChild("DynamicSyncPresetsChild", vec2(0.0f, (dynamicButtonHeight + dynamicButtonSpacing.y) * 3.0f + (style.WindowPadding.y * 2.0f)) - dynamicButtonSpacing.y, true);
		{
			const auto halfWidth = Gui::GetContentRegionAvailWidth() / 2.0f;

			// NOTE: Delay icon rendering to optimize texture batching
			std::array<ImRect, EnumCount<DynamicSyncPreset>()> presetIconRectsToDraw;

			// TODO: Hover preview in TargetRenderWindow (?)
			hoveredDynamicSyncPreset = {};

			auto dynamicSyncPresetButton = [&](DynamicSyncPreset preset)
			{
				Gui::PushID(static_cast<int>(preset));
				if (Gui::ButtonEx("##DynamicSyncPresetButton", vec2(halfWidth, dynamicButtonHeight)))
					ApplyDynamicSyncPresetToSelectedTargets(undoManager, chart, preset);
				Gui::PopID();

				if (Gui::IsItemHovered())
					hoveredDynamicSyncPreset = preset;

				const auto rect = [rect = Gui::FitFixedAspectRatio(Gui::GetCurrentWindowRead()->DC.LastItemRect, 1.0f)]() { auto r = rect; r.Expand(-4.0f); return r; }();
				presetIconRectsToDraw[static_cast<size_t>(preset)] = rect;
			};

			Gui::PushStyleVar(ImGuiStyleVar_ItemSpacing, dynamicButtonSpacing);
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
			Gui::PopStyleVar();
		}
		Gui::EndChild();

		Gui::BeginChild("StaticSyncPresetsChild", {}, true);
		{
			for (const auto& staticSyncPreset : staticSyncPresets)
			{
				Gui::PushID(&staticSyncPreset);
				if (Gui::ButtonEx(staticSyncPreset.Name.c_str(), vec2(Gui::GetContentRegionAvailWidth(), staticButtonHeight)))
					ApplyStaticSyncPresetToSelectedTargets(undoManager, chart, staticSyncPreset);
				Gui::PopID();
			}

			// TODO: Open edit dialog with button to "create from selection" (?)
			Gui::ButtonEx("Add New...", vec2(Gui::GetContentRegionAvailWidth(), staticButtonHeight));
		}
		Gui::EndChild();

		Gui::PopItemDisabledAndTextColorIf(!anySyncTargetSelected);
	}

	void PresetWindow::SequenceGui(Chart& chart)
	{
		// TODO: Implement at least least circle presets
		Gui::TextDisabled("TODO:");
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
