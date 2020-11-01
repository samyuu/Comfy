#include "PresetWindow.h"
#include "ChartCommands.h"
#include "TargetPropertyRules.h"
#include "ImGui/Extensions/ImGuiExtensions.h"

namespace Comfy::Studio::Editor
{
	namespace
	{
		template <size_t TargetCount>
		StaticSyncPreset ConstructStaticSyncPreset(std::string name, std::array<PresetTargetData, TargetCount> targetData)
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
					PresetTargetData { ButtonType::Triangle, { vec2(960.0f, 480.0f), 0.0f, 0.0f, 500.0f, 1040.0f } },
					PresetTargetData { ButtonType::Square, { vec2(864.0f, 576.0f), -90.0f, 0.0f, 500.0f, 1040.0f } },
					PresetTargetData { ButtonType::Cross, { vec2(960.0f, 672.0f), 180.0f, 0.0f, 500.0f, 1040.0f } },
					PresetTargetData { ButtonType::Circle, { vec2(1056.0f, 576.0f), +90.0f, 0.0f, 500.0f, 1040.0f } },
				}));
			presets.push_back(ConstructStaticSyncPreset("Small Sideways Square", std::array
				{
					PresetTargetData { ButtonType::Triangle, { vec2(960.0f, 336.0f), 0.0f, 0.0f, 500.0f, 960.0f } },
					PresetTargetData { ButtonType::Square, { vec2(744.0f, 552.0f), -90.0f, 0.0f, 500.0f, 960.0f } },
					PresetTargetData { ButtonType::Cross, { vec2(960.0f, 768.0f), 180.0f, 0.0f, 500.0f, 960.0f } },
					PresetTargetData { ButtonType::Circle, { vec2(1176.0f, 552.0f), 90.0f, 0.0f, 500.0f, 960.0f } },
				}));
			presets.push_back(ConstructStaticSyncPreset("Large Sideways Square", std::array
				{
					PresetTargetData { ButtonType::Triangle, { vec2(960.0f, 192.0f), 0.0f, 0.0f, 500.0f, 800.0f } },
					PresetTargetData { ButtonType::Square, { vec2(384.0f, 528.0f), -90.0f, 0.0f, 500.0f, 800.0f } },
					PresetTargetData { ButtonType::Cross, { vec2(960.0f, 864.0f), 180.0f, 0.0f, 500.0f, 800.0f } },
					PresetTargetData { ButtonType::Circle, { vec2(1536.0f, 528.0f), 90.0f, 0.0f, 500.0f, 800.0f } },
				}));

			presets.push_back(ConstructStaticSyncPreset("Curved Small Square", std::array
				{
					PresetTargetData { ButtonType::Triangle, { vec2(912.0f, 480.0f), -45.0f, 1.0f, 3000.0f, 2000.0f } },
					PresetTargetData { ButtonType::Square, { vec2(1008.0f, 480.0f),45.0f, -1.0f, 3000.0f, 2000.0f } },
					PresetTargetData { ButtonType::Cross, { vec2(912.0f, 576.0f), -135.0f, -1.0f, 3000.0f, 2000.0f } },
					PresetTargetData { ButtonType::Circle, { vec2(1008.0f, 576.0f), 135.0f, 1.0f, 3000.0f, 2000.0f } },
				}));

			presets.push_back(ConstructStaticSyncPreset("Parallelogram", std::array
				{
					PresetTargetData { ButtonType::Triangle, { vec2(240.0f, 240.0f), -90.0f, 0.0f, 500.0f, 880.0f } },
					PresetTargetData { ButtonType::Square, { vec2(720.0f, 768.0f), -90.0f, 0.0f, 500.0f, 880.0f } },
					PresetTargetData { ButtonType::Cross, { vec2(1200.0f, 240.0f), 90.0f, 0.0f, 500.0f, 880.0f } },
					PresetTargetData { ButtonType::Circle, { vec2(1680.0f, 768.0f), 90.0f, 0.0f, 500.0f, 880.0f } },
				}));

			presets.push_back(ConstructStaticSyncPreset("Fancy Square Stuff [0]", std::array
				{
					PresetTargetData { ButtonType::Triangle, { vec2(960, 432), 45.0f, -2.0f, 1000.0f, 1040.0f } },
					PresetTargetData { ButtonType::Square, { vec2(864, 528), -45.0f, -2.0f, 1000.0f, 1040.0f } },
					PresetTargetData { ButtonType::Cross, { vec2(960, 624), -135.0f, -2.0f, 1000.0f, 1040.0f } },
					PresetTargetData { ButtonType::Circle, { vec2(1056, 528), 135.0f, -2.0f, 1000.0f, 1040.0f } },
				}));
			presets.push_back(ConstructStaticSyncPreset("Fancy Square Stuff [1]", std::array
				{
					PresetTargetData { ButtonType::Triangle, { vec2(960.0f, 336.0f), 90.0f, -2.0f, 1000.0f, 1040.0f } },
					PresetTargetData { ButtonType::Square, { vec2(768.0f, 528.0f), 0.0f, -2.0f, 1000.0f, 1040.0f } },
					PresetTargetData { ButtonType::Cross, { vec2(960.0f, 720.0f), -90.0f, -2.0f, 1000.0f, 1040.0f } },
					PresetTargetData { ButtonType::Circle, { vec2(1152.0f, 528.0f), 180.0f, -2.0f, 1000.0f, 1040.0f } },
				}));
			presets.push_back(ConstructStaticSyncPreset("Fancy Square Stuff [2]", std::array
				{
					PresetTargetData { ButtonType::Triangle, { vec2(960, 168), 165.0f, -2.0f, 1000.0f, 2080.0f } },
					PresetTargetData { ButtonType::Square, { vec2(600, 528), 75.0f, -2.0f, 1000.0f, 2080.0f } },
					PresetTargetData { ButtonType::Cross, { vec2(960, 888), -15.0f, -2.0f, 1000.0f, 2080.0f } },
					PresetTargetData { ButtonType::Circle, { vec2(1320, 528), -105.0f, -2.0f, 1000.0f, 2080.0f } },
				}));
			return presets;
		}
	}

	// TODO: Context menu (?) settings for narrow vertical angles, inside-out square / triangle angles and different sync formation height offset ("elevate bottom row" (?))
	//		 Sequence preset option to round position to nearest pixel and angle to nearest whole (?)
	// struct DynamicSyncPresetSettings { ... };

	PresetWindow::PresetWindow(Undo::UndoManager& undoManager) : undoManager(undoManager), staticSyncPresets(GetTestStaticSyncPresets())
	{
	}

	void PresetWindow::SyncGui(Chart& chart)
	{
		hovered.DynamicSyncPreset = {};
		hovered.StaticSyncPreset = {};
		hovered.AnyChildWindow = Gui::IsWindowHovered(ImGuiHoveredFlags_ChildWindows);

		const auto& style = Gui::GetStyle();

		constexpr auto buttonSpacing = vec2(2.0f);
		constexpr auto dynamicButtonHeight = 44.0f; // 46.0f;
		constexpr auto staticButtonHeight = 22.0f; // 26.0f;

		const bool anySyncTargetSelected = std::any_of(chart.Targets.begin(), chart.Targets.end(), [](auto& t) { return (t.IsSelected && t.Flags.IsSync); });

		// TODO: Correctly factor in window padding and other missing stlye vars (?)
		const auto dynamicChildHeight = (dynamicButtonHeight + buttonSpacing.y) * 3.0f + (style.WindowPadding.y * 2.0f) - buttonSpacing.y;
		const auto addChildHeight = (staticButtonHeight + buttonSpacing.y + style.WindowPadding.y);
		const auto minStaticChildHeight = ((staticButtonHeight + buttonSpacing.y) * 2.5f);
		const auto staticChildHeight = std::max(addChildHeight + dynamicChildHeight + minStaticChildHeight + (style.WindowPadding.y * 2.0f), Gui::GetContentRegionAvail().y) - addChildHeight - dynamicChildHeight - (style.WindowPadding.y * 2.0f); // (staticButtonHeight + buttonSpacing.y) * (staticSyncPresets.size()) + (style.WindowPadding.y * 2.0f) - buttonSpacing.y;

		Gui::BeginChild("DynamicSyncPresetsChild", vec2(0.0f, dynamicChildHeight), true);
		{
			hovered.DynamincSyncPresetChild = Gui::IsWindowHovered();

			const auto halfWidth = (Gui::GetContentRegionAvailWidth() - buttonSpacing.x) / 2.0f;
			std::array<ImRect, EnumCount<DynamicSyncPreset>()> presetIconRectsToDraw;

			auto dynamicSyncPresetButton = [&](DynamicSyncPreset preset)
			{
				Gui::PushID(static_cast<int>(preset));
				if (Gui::ButtonEx("##DynamicSyncPresetButton", vec2(halfWidth, dynamicButtonHeight)))
					ApplyDynamicSyncPresetToSelectedTargets(undoManager, chart, preset);
				Gui::PopID();

				if (Gui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
					hovered.DynamicSyncPreset = preset;

				const auto rect = [r = Gui::FitFixedAspectRatio(Gui::GetCurrentWindowRead()->DC.LastItemRect, 1.0f)]() mutable { r.Expand(-4.0f); return r; }();
				presetIconRectsToDraw[static_cast<size_t>(preset)] = rect;
			};

			Gui::PushStyleVar(ImGuiStyleVar_ItemSpacing, buttonSpacing);
			{
				Gui::PushItemDisabledAndTextColorIf(!anySyncTargetSelected);
				for (size_t i = 0; i < EnumCount<DynamicSyncPreset>(); i += 2)
				{
					dynamicSyncPresetButton(static_cast<DynamicSyncPreset>(i + 0));
					Gui::SameLine();
					dynamicSyncPresetButton(static_cast<DynamicSyncPreset>(i + 1));
				}

				// NOTE: Delay icon rendering to optimize texture batching
				for (size_t i = 0; i < EnumCount<DynamicSyncPreset>(); i++)
				{
					const auto iconRect = presetIconRectsToDraw[i];
					const auto iconSpr = IndexOr(i, sprites.DynamicSyncPresetIcons, nullptr);

					if (iconSpr != nullptr)
						Gui::AddSprite(Gui::GetWindowDrawList(), *editorSprites, *iconSpr, iconRect.GetTL(), iconRect.GetBR(), Gui::GetColorU32(ImGuiCol_Text));
				}
				Gui::PopItemDisabledAndTextColorIf(!anySyncTargetSelected);
			}
			Gui::PopStyleVar();
		}
		Gui::EndChild();

		Gui::BeginChild("StaticSyncPresetsChild", vec2(0.0f, staticChildHeight), true, ImGuiWindowFlags_AlwaysVerticalScrollbar);
		{
			hovered.StaticSyncPresetChild = Gui::IsWindowHovered();

			Gui::PushItemDisabledAndTextColorIf(!anySyncTargetSelected);
			for (const auto& staticSyncPreset : staticSyncPresets)
			{
				Gui::PushID(&staticSyncPreset);
				if (Gui::ButtonEx(staticSyncPreset.Name.c_str(), vec2(Gui::GetContentRegionAvailWidth(), staticButtonHeight)))
					ApplyStaticSyncPresetToSelectedTargets(undoManager, chart, staticSyncPreset);
				Gui::PopID();

				if (Gui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
					hovered.StaticSyncPreset = static_cast<size_t>(std::distance(&*staticSyncPresets.cbegin(), &staticSyncPreset));
			}
			Gui::PopItemDisabledAndTextColorIf(!anySyncTargetSelected);
		}
		Gui::EndChild();

		Gui::BeginChild("AddPresetsChild", vec2(0.0f, addChildHeight), true);
		{
			hovered.AddPresetChild = Gui::IsWindowHovered();

			// TODO: Open edit dialog with button to "create from selection" (?)
			//		 Hide "Add New..." buttons in context menu (?)
			if (Gui::ButtonEx("Add New...", vec2(Gui::GetContentRegionAvailWidth(), staticButtonHeight)))
				staticSyncPresets.push_back(StaticSyncPreset { "DUMMY_PRESET" });
		}
		Gui::EndChild();

		hovered.AnyHoveredLastFrame = hovered.AnyHoveredThisFrame;
		hovered.AnyHoveredThisFrame = ((hovered.AnyChildWindow && !hovered.AddPresetChild) || hovered.DynamicSyncPreset.has_value() || hovered.StaticSyncPreset.has_value());

		if (hovered.AnyHoveredThisFrame)
			hovered.LastHoverStopwatch.Restart();
	}

	void PresetWindow::SequenceGui(Chart& chart)
	{
		// TODO: Implement at least least circle presets
		Gui::TextDisabled("TODO:");
	}

	void PresetWindow::OnRenderWindowRender(Chart& chart, TargetRenderWindow& renderWindow, Render::Renderer2D& renderer, TargetRenderHelper& renderHelper)
	{
		if (const auto dimness = GetPresetPreviewDimness(false); dimness > 0.0f)
			renderer.Draw(Render::RenderCommand2D(vec2(0.0f, 0.0f), Rules::PlacementAreaSize, vec4(0.0f, 0.0f, 0.0f, dimness)));

		presetPreview.TargetCount = 0;
		if (!hovered.DynamicSyncPreset.has_value() && !hovered.StaticSyncPreset.has_value())
			return;

		if (hovered.DynamicSyncPreset.has_value())
		{
			presetPreview.TargetCount = FindFirstApplicableDynamicSyncPresetDataForSelectedTargets(chart, hovered.DynamicSyncPreset.value(), presetPreview.Targets);
			RenderSyncPresetPreview(renderer, renderHelper, presetPreview.TargetCount, presetPreview.Targets);
		}
		else if (hovered.StaticSyncPreset.has_value())
		{
			const auto& hoveredPreset = staticSyncPresets[*hovered.StaticSyncPreset];
			presetPreview.TargetCount = hoveredPreset.TargetCount;
			presetPreview.Targets = hoveredPreset.Targets;
			RenderSyncPresetPreview(renderer, renderHelper, hoveredPreset.TargetCount, hoveredPreset.Targets);
		}
	}

	void PresetWindow::OnRenderWindowOverlayGui(Chart& chart, TargetRenderWindow& renderWindow, ImDrawList& drawList)
	{
		const auto windowRect = Gui::GetCurrentWindow()->Rect();
		if (const auto dimness = GetPresetPreviewDimness(true); dimness > 0.0f)
			drawList.AddRectFilled(windowRect.GetTL(), windowRect.GetBR(), ImColor(0.0f, 0.0f, 0.0f, dimness));

		if (!hovered.DynamicSyncPreset.has_value() && !hovered.StaticSyncPreset.has_value())
			return;

		for (u32 i = 0; i < presetPreview.TargetCount; i++)
		{
			const auto& presetTarget = presetPreview.Targets[i];

			const auto color = GetButtonTypeColorU32(presetTarget.Type);
			DrawCurvedButtonPathLine(renderWindow, drawList, presetTarget.Properties, color, 2.0f);

			const auto targetPos = GetButtonPathSinePoint(1.0f, presetTarget.Properties);
			const auto targetPosTangent = glm::normalize(GetButtonPathSinePoint(1.0f - CurvedButtonPathStepDistance, presetTarget.Properties) - targetPos);
			DrawButtonPathArrowHead(renderWindow, drawList, targetPos, targetPosTangent, color, 2.0f);
		}
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

	f32 PresetWindow::GetPresetPreviewDimness(bool overlayPass) const
	{
		constexpr f32 maxDimRender = 0.15f, maxDimOverlay = 0.45f, transitionMS = 75.0f;
		const auto max = overlayPass ? maxDimOverlay : maxDimRender;

		if (!hovered.LastHoverStopwatch.IsRunning())
			return 0.0f;
		if (hovered.AnyHoveredThisFrame)
			return max;

		const auto sinceHoverMS = static_cast<f32>(hovered.LastHoverStopwatch.GetElapsed().TotalMilliseconds());
		const auto dim = ConvertRange<f32>(0.0f, transitionMS, max, 0.0f, sinceHoverMS);
		return std::clamp(dim, 0.0f, max);
	}

	void PresetWindow::RenderSyncPresetPreview(Render::Renderer2D& renderer, TargetRenderHelper& renderHelper, u32 targetCount, const std::array<PresetTargetData, Rules::MaxSyncPairCount>& presetTargets)
	{
		assert(targetCount <= presetTargets.size());
		if (targetCount < 1)
			return;

		TargetRenderHelper::ButtonSyncLineData syncLineData = {};
		TargetRenderHelper::TargetData targetData = {};

		syncLineData.SyncPairCount = targetCount;
		syncLineData.Progress = 0.0f;
		for (size_t i = 0; i < targetCount; i++)
		{
			syncLineData.TargetPositions[i] = presetTargets[i].Properties.Position;
			syncLineData.ButtonPositions[i] = presetTargets[i].Properties.Position;
		}
		renderHelper.DrawButtonPairSyncLines(renderer, syncLineData);

		for (size_t i = 0; i < targetCount; i++)
		{
			const auto& presetTarget = presetTargets[i];

			targetData.Type = presetTarget.Type;
			targetData.Position = presetTarget.Properties.Position;
			targetData.NoScale = true;
			targetData.Transparent = true;
			renderHelper.DrawTarget(renderer, targetData);
		}
	}
}
