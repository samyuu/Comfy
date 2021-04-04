#include "PresetWindow.h"
#include "ChartCommands.h"
#include "TargetPropertyRules.h"
#include "Core/ComfyStudioSettings.h"
#include "ImGui/Extensions/ImGuiExtensions.h"
#include <FontIcons.h>

namespace Comfy::Studio::Editor
{
	namespace
	{
		constexpr f32 MaxDimRender = 0.15f, MaxDimOverlay = 0.45f;
		constexpr f32 HoverFadeInMS = 60.0f, HoverFadeOutMS = 75.0f;

		constexpr vec2 PresetButtonSpacing = vec2(2.0f);

		constexpr f32 DynamicSyncButtonHeight = 44.0f;
		constexpr f32 StaticSyncButtonHeight = 22.0f;
		constexpr f32 SyncSettingsButtonWidth = 26.0f;

		constexpr f32 SingleLineSequenceButtonHeight = StaticSyncButtonHeight;
		constexpr f32 SameLineSequenceButtonHeight = StaticSyncButtonHeight;
	}

	PresetWindow::PresetWindow(Undo::UndoManager& undoManager) : undoManager(undoManager)
	{
		// sequencePresetSettings.TickOffset = BeatTick::FromBeats(2);
		sequencePresetSettings.ApplyFirstTargetTickAsOffset = true;
	}

	void PresetWindow::SyncGui(Chart& chart)
	{
		hovered.Sync.DynamicPreset = {};
		hovered.Sync.StaticPreset = {};
		hovered.Sync.AnyChildWindow = Gui::IsWindowHovered(ImGuiHoveredFlags_ChildWindows);
		hovered.Sync.ContextMenu = false;
		hovered.Sync.ContextMenuOpen = false;

		const auto& style = Gui::GetStyle();
		const auto presetSettingsContextMenuID = Gui::GetCurrentWindow()->GetID("PresetWindowSyncSettingsContextMenu");

		const bool anySyncTargetSelected = std::any_of(chart.Targets.begin(), chart.Targets.end(), [](auto& t) { return (t.IsSelected && t.Flags.IsSync); });

		// TODO: Correctly factor in window padding and other missing stlye vars (?)
		const f32 dynamicChildHeight = (DynamicSyncButtonHeight + PresetButtonSpacing.y) * 3.0f + (style.WindowPadding.y * 2.0f) - PresetButtonSpacing.y;
		const f32 addChildHeight = (StaticSyncButtonHeight + PresetButtonSpacing.y + style.WindowPadding.y);
		const f32 minStaticChildHeight = ((StaticSyncButtonHeight + PresetButtonSpacing.y) * 2.5f);
		const f32 staticChildHeight = std::max(addChildHeight + dynamicChildHeight + minStaticChildHeight + (style.WindowPadding.y * 2.0f), Gui::GetContentRegionAvail().y) - addChildHeight - dynamicChildHeight - (style.WindowPadding.y * 2.0f);

		Gui::BeginChild("DynamicSyncPresetsChild", vec2(0.0f, dynamicChildHeight), true);
		{
			hovered.Sync.DynamincChildWindow = Gui::IsWindowHovered();

			const f32 halfWidth = (Gui::GetContentRegionAvailWidth() - PresetButtonSpacing.x) / 2.0f;
			std::array<ImRect, EnumCount<DynamicSyncPreset>()> presetIconRectsToDraw;

			auto dynamicSyncPresetButton = [&](DynamicSyncPreset preset)
			{
				Gui::PushID(static_cast<int>(preset));
				if (Gui::ButtonEx("##DynamicSyncPresetButton", vec2(halfWidth, DynamicSyncButtonHeight)))
					ApplyDynamicSyncPresetToSelectedTargets(undoManager, chart, preset, dynamicSyncPresetSettings);
				Gui::PopID();

				if (Gui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
					hovered.Sync.DynamicPreset = preset;

				const auto rect = [r = Gui::FitFixedAspectRatio(Gui::GetCurrentWindowRead()->DC.LastItemRect, 1.0f)]() mutable { r.Expand(-4.0f); return r; }();
				presetIconRectsToDraw[static_cast<size_t>(preset)] = rect;
			};

			Gui::PushStyleVar(ImGuiStyleVar_ItemSpacing, PresetButtonSpacing);
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
			hovered.Sync.StaticChildWindow = Gui::IsWindowHovered();

			for (const auto& staticSyncPreset : GlobalUserData.TargetPreset.StaticSyncPresets)
			{
				const bool presetDisabled = (!anySyncTargetSelected || staticSyncPreset.TargetCount < 1);
				Gui::PushID(&staticSyncPreset);
				Gui::PushItemDisabledAndTextColorIf(presetDisabled);

				if (Gui::ButtonEx(staticSyncPreset.Name.c_str(), vec2(Gui::GetContentRegionAvailWidth(), StaticSyncButtonHeight)))
					ApplyStaticSyncPresetToSelectedTargets(undoManager, chart, staticSyncPreset);

				Gui::PopItemDisabledAndTextColorIf(presetDisabled);
				Gui::PopID();

				if (Gui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
					hovered.Sync.StaticPreset = static_cast<size_t>(std::distance(&*GlobalUserData.TargetPreset.StaticSyncPresets.cbegin(), &staticSyncPreset));

				// TODO: At least basic item context menu for changing the name, move up/down and delete (?)
			}
		}
		Gui::EndChild();

		Gui::BeginChild("AddPresetsChild", vec2(0.0f, addChildHeight), true);
		{
			hovered.Sync.AddChildWindow = Gui::IsWindowHovered();

			const bool addNewEnabled = COMFY_DEBUG_RELEASE_SWITCH(anySyncTargetSelected, false);
			Gui::PushItemDisabledAndTextColorIf(!addNewEnabled);

			if (Gui::ButtonEx("Add New...", vec2(Gui::GetContentRegionAvailWidth() - SyncSettingsButtonWidth, StaticSyncButtonHeight)))
			{
#if COMFY_DEBUG && 1 // TODO:
				if (const auto firstSelectedTarget = FindIfOrNull(chart.Targets.GetRawView(), [&](auto& t) { return (t.IsSelected && t.Flags.IsSync); }); firstSelectedTarget != nullptr)
				{
					const auto syncPair = &firstSelectedTarget[-firstSelectedTarget->Flags.IndexWithinSyncPair];
					assert(syncPair[0].Flags.IndexWithinSyncPair == 0);

					auto& newPreset = GlobalUserData.Mutable().TargetPreset.StaticSyncPresets.emplace_back();
					newPreset.Name = "Unnamed Preset " + std::to_string(GlobalUserData.TargetPreset.StaticSyncPresets.size());
					newPreset.TargetCount = syncPair->Flags.SyncPairCount;
					for (size_t i = 0; i < newPreset.TargetCount; i++)
					{
						newPreset.Targets[i].Type = syncPair[i].Type;
						newPreset.Targets[i].Properties = Rules::TryGetProperties(syncPair[i]);
					}

					GlobalUserData.Mutable().SaveToFile();
				}
#endif
			}

			Gui::PopItemDisabledAndTextColorIf(!addNewEnabled);

			Gui::SameLine(0.0f, 0.0f);
			if (Gui::Button(ICON_FA_COG, vec2(Gui::GetContentRegionAvailWidth(), 0.0f)))
				Gui::OpenPopupEx(presetSettingsContextMenuID);
		}
		Gui::EndChild();

		if (Gui::IsMouseReleased(1) && (hovered.AnyHoveredThisFrame && Gui::WasHoveredWindowHoveredOnMouseClicked(1)) && Gui::IsMouseSteady())
			Gui::OpenPopupEx(presetSettingsContextMenuID);

		if (Gui::BeginPopupEx(presetSettingsContextMenuID, (ImGuiWindowFlags_NoMove | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoDocking)))
		{
			hovered.Sync.ContextMenu = Gui::IsWindowHovered(ImGuiHoveredFlags_ChildWindows | ImGuiHoveredFlags_AllowWhenBlockedByActiveItem);
			hovered.Sync.ContextMenuOpen = true;

			static constexpr std::array checkboxBindings =
			{
				Input::Binding(Input::KeyCode_1), Input::Binding(Input::KeyCode_2), Input::Binding(Input::KeyCode_3),
				Input::Binding(Input::KeyCode_4), Input::Binding(Input::KeyCode_5), Input::Binding(Input::KeyCode_6),
				Input::Binding(Input::KeyCode_7), Input::Binding(Input::KeyCode_8), Input::Binding(Input::KeyCode_9),
			};

			auto checkbox = [](const char* label, bool& inOutBool, u32 checkboxIndex, const char* description = nullptr)
			{
				const auto* binding = IndexOrNull(checkboxIndex, checkboxBindings);
				if (Gui::MenuItemWithFlags(label, (binding != nullptr) ? Input::ToString(*binding).data() : nullptr, inOutBool, true, ImGuiSelectableFlags_DontClosePopups))
					inOutBool ^= true;

				if (binding != nullptr && Gui::IsWindowFocused() && Input::IsPressed(*binding, false))
					inOutBool ^= true;

				// NOTE: Extra long delay time to hopefully not get in the way
				if (description != nullptr && !inOutBool && Gui::IsItemHoveredDelayed(ImGuiHoveredFlags_None, 1.5f))
					Gui::WideSetTooltip(description);
			};

			u32 checkboxIndex = 0;
			Gui::TextUnformatted("Dynamic Sync Preset Settings:\t\t\t\t");
			Gui::Separator();
			checkbox("Steep Angles", dynamicSyncPresetSettings.SteepAngles, checkboxIndex++,
				"Applies to vertical and horizontal sync presets\n"
				"- Use steeper 35 instead of 45 degree vertical angles\n"
				"- Use steeper 10 instead of 20 degree horizontal angles\n"
				"Intended for\n"
				"- Sync pairs placed in quick succession\n"
				"- Sync pairs positioned closely to the top / bottom edge of the screen"
			);
			checkbox("Same Direction Angles", dynamicSyncPresetSettings.SameDirectionAngles, checkboxIndex++,
				"Applies to vertical and horizontal sync presets\n"
				"- Set all angles to the same direction"
			);
			Gui::Separator();
			checkbox("Inside Out Angles", dynamicSyncPresetSettings.InsideOutAngles, checkboxIndex++,
				"Applies to square and triangle sync presets\n"
				"- Flip angles by 180 degrees\n"
				"- Increase button distances"
			);
			checkbox("Elevate Bottom Row", dynamicSyncPresetSettings.ElevateBottomRow, checkboxIndex++,
				"Applies to square and triangle sync presets\n"
				"- Raise position height of bottom row targets by one 1/8th step"
			);

			Gui::EndPopup();
		}
	}

	void PresetWindow::SequenceGui(Chart& chart)
	{

		hovered.Sequence.Preset = {};
		hovered.Sequence.AnyChildWindow = Gui::IsWindowHovered(ImGuiHoveredFlags_ChildWindows);

		const auto& style = Gui::GetStyle();
		const bool anyTargetSelected = std::any_of(chart.Targets.begin(), chart.Targets.end(), [](auto& t) { return (t.IsSelected); });

		const f32 addChildHeight = 0.0f; // (StaticSyncButtonHeight + PresetButtonSpacing.y + style.WindowPadding.y);
		const f32 childHeight = std::max(addChildHeight + (style.WindowPadding.y * 2.0f), Gui::GetContentRegionAvail().y) - addChildHeight - (style.WindowPadding.y * 2.0f);

		Gui::BeginChild("SequencePresetsChild", vec2(0.0f, childHeight), true, ImGuiWindowFlags_AlwaysVerticalScrollbar);
		{
			hovered.Sequence.ChildWindow = Gui::IsWindowHovered();

			const f32 fullWidth = Gui::GetContentRegionAvailWidth();
			const f32 halfWidth = (fullWidth - PresetButtonSpacing.x) / 2.0f;

			Gui::PushItemDisabledAndTextColorIf(!anyTargetSelected);
			for (size_t i = 0; i < GlobalUserData.TargetPreset.SequencePresets.size(); i++)
			{
				const auto& thisSequencePreset = GlobalUserData.TargetPreset.SequencePresets[i];
				Gui::PushID(&thisSequencePreset);

				if (thisSequencePreset.ButtonType == SequencePresetButtonType::SingleLine)
				{
					if (Gui::ButtonEx(thisSequencePreset.Name.c_str(), vec2(fullWidth, SingleLineSequenceButtonHeight)))
						ApplySequencePresetToSelectedTargets(undoManager, chart, thisSequencePreset, sequencePresetSettings);
					if (Gui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
						hovered.Sequence.Preset = i;
				}
				else if (thisSequencePreset.ButtonType == SequencePresetButtonType::SameLine)
				{
					Gui::PushStyleVar(ImGuiStyleVar_ItemSpacing, PresetButtonSpacing);

					if (Gui::ButtonEx(thisSequencePreset.Name.c_str(), vec2(halfWidth, SameLineSequenceButtonHeight)))
						ApplySequencePresetToSelectedTargets(undoManager, chart, thisSequencePreset, sequencePresetSettings);
					if (Gui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
						hovered.Sequence.Preset = i;

					if (i + 1 < GlobalUserData.TargetPreset.SequencePresets.size())
					{
						Gui::SameLine();

						const auto& nextSequencePreset = GlobalUserData.TargetPreset.SequencePresets[i + 1];
						Gui::PushID(&nextSequencePreset);

						if (Gui::ButtonEx(nextSequencePreset.Name.c_str(), vec2(halfWidth, SameLineSequenceButtonHeight)))
							ApplySequencePresetToSelectedTargets(undoManager, chart, nextSequencePreset, sequencePresetSettings);
						if (Gui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
							hovered.Sequence.Preset = i + 1;

						Gui::PopID();
						++i;
					}

					Gui::PopStyleVar();

				}

				Gui::PopID();
			}
			Gui::PopItemDisabledAndTextColorIf(!anyTargetSelected);
		}
		Gui::EndChild();
	}

	void PresetWindow::UpdateStateAfterBothGuiPartsHaveBeenDrawn()
	{
		bool anyHovered = false;

		if (hovered.Sync.AnyChildWindow && !hovered.Sync.AddChildWindow && !hovered.Sync.ContextMenu)
			anyHovered = true;
		if (hovered.Sync.DynamicPreset.has_value() || hovered.Sync.StaticPreset.has_value())
			anyHovered = true;

		// TODO: Add checks for future additions
		if (hovered.Sequence.AnyChildWindow)
			anyHovered = true;
		if (hovered.Sequence.Preset.has_value())
			anyHovered = true;
		if (hovered.Sync.ContextMenuOpen)
			anyHovered = true;

		// HACK: Super hacky but mainly to prevent undesired hover detecting for cases like resize hovering a docked window...
		if (auto c = Gui::GetMouseCursor(); c != ImGuiMouseCursor_None && c != ImGuiMouseCursor_Arrow && c != ImGuiMouseCursor_Hand)
			anyHovered = false;

		hovered.AnyHoveredLastFrame = hovered.AnyHoveredThisFrame;
		hovered.AnyHoveredThisFrame = anyHovered;

		if (anyHovered)
			hovered.LastHoverStopwatch.Restart();
		else
			hovered.HoverDurationStopwatch.Restart();
	}

	void PresetWindow::OnRenderWindowRender(Chart& chart, TargetRenderWindow& renderWindow, Render::Renderer2D& renderer)
	{
		auto& renderHelper = renderWindow.GetRenderHelper();

		if (const auto dimness = GetPresetPreviewDimness(false); dimness > 0.0f)
			renderer.Draw(Render::RenderCommand2D(vec2(0.0f, 0.0f), Rules::PlacementAreaSize, vec4(0.0f, 0.0f, 0.0f, dimness)));

		syncPresetPreview.TargetCount = 0;
		if (hovered.Sync.DynamicPreset.has_value())
		{
			syncPresetPreview.TargetCount = FindFirstApplicableDynamicSyncPresetDataForSelectedTargets(chart, hovered.Sync.DynamicPreset.value(), dynamicSyncPresetSettings, syncPresetPreview.Targets);
			RenderSyncPresetPreview(renderer, renderHelper, syncPresetPreview.TargetCount, syncPresetPreview.Targets);
		}
		else if (hovered.Sync.StaticPreset.has_value())
		{
			const auto& hoveredPreset = GlobalUserData.TargetPreset.StaticSyncPresets[*hovered.Sync.StaticPreset];
			syncPresetPreview.TargetCount = hoveredPreset.TargetCount;
			syncPresetPreview.Targets = hoveredPreset.Targets;
			RenderSyncPresetPreview(renderer, renderHelper, hoveredPreset.TargetCount, hoveredPreset.Targets);
		}
		else if (hovered.Sequence.Preset.has_value())
		{
			// TODO: ...
			const auto& preset = GlobalUserData.TargetPreset.SequencePresets[*hovered.Sequence.Preset];

			if (preset.Type == SequencePresetType::Circle)
			{

			}
		}
	}

	void PresetWindow::OnRenderWindowOverlayGui(Chart& chart, TargetRenderWindow& renderWindow, ImDrawList& drawList)
	{
		const auto windowRect = Gui::GetCurrentWindow()->Rect();
		if (const auto dimness = GetPresetPreviewDimness(true); dimness > 0.0f)
			drawList.AddRectFilled(windowRect.GetTL(), windowRect.GetBR(), ImColor(0.0f, 0.0f, 0.0f, dimness));

		// NOTE: Helps to reduce visual noise while quickly mouse hover skipping over a preset window
		const f32 hoverFadeOpacity = GetHoverFadeInPreviewOpacity();
		const u32 hoverFadeOpacityU8 = static_cast<u8>(hoverFadeOpacity * std::numeric_limits<u8>::max());

		if (hovered.Sync.DynamicPreset.has_value() || hovered.Sync.StaticPreset.has_value())
		{
			for (u32 i = 0; i < syncPresetPreview.TargetCount; i++)
			{
				const auto& presetTarget = syncPresetPreview.Targets[i];

				const u32 color = GetButtonTypeColorU32(presetTarget.Type, hoverFadeOpacityU8);
				DrawCurvedButtonPathLine(renderWindow, drawList, presetTarget.Properties, color, 2.0f);

				const f32 stepDistance = 1.0f / static_cast<f32>(GlobalUserData.System.Gui.TargetButtonPathCurveSegments);

				const vec2 targetPos = GetButtonPathSinePoint(1.0f, presetTarget.Properties);
				const vec2 targetPosTangent = glm::normalize(GetButtonPathSinePoint(1.0f - stepDistance, presetTarget.Properties) - targetPos);
				DrawButtonPathArrowHead(renderWindow, drawList, targetPos, targetPosTangent, color, 2.0f);
			}
		}
		else if (hovered.Sequence.Preset.has_value())
		{
			// TODO: Proper implementation...
			const auto& preset = GlobalUserData.TargetPreset.SequencePresets[*hovered.Sequence.Preset];

			if (preset.Type == SequencePresetType::Circle)
			{
				const u32 circleColor = ImColor(0.69f, 0.69f, 0.69f, hoverFadeOpacity);
				drawList.AddCircle(renderWindow.TargetAreaToScreenSpace(preset.Circle.Center), preset.Circle.Radius * renderWindow.GetCamera().Zoom, circleColor, 64, 2.0f);

				constexpr i32 arrowCount = 8; // 4;
				for (i32 i = 0; i < arrowCount; i++)
				{
					const f32 angleRadians = static_cast<f32>(i) * (glm::two_pi<f32>() / static_cast<f32>(arrowCount)) * preset.Circle.Direction;

					const vec2 normal = vec2(glm::cos(angleRadians), glm::sin(angleRadians));
					const vec2 tangent = vec2(normal.y, -normal.x) * preset.Circle.Direction;
					const vec2 pointOnCircle = preset.Circle.Center + normal * preset.Circle.Radius;

					DrawButtonPathArrowHeadCentered(renderWindow, drawList, pointOnCircle, tangent, circleColor, 2.0f);
				}
			}
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
		const f32 max = overlayPass ? MaxDimOverlay : MaxDimRender;

		if (hovered.AnyHoveredThisFrame)
		{
			if (!hovered.HoverDurationStopwatch.IsRunning())
				return max;

			const f32 hoverMS = static_cast<f32>(hovered.HoverDurationStopwatch.GetElapsed().TotalMilliseconds());
			return std::clamp(ConvertRange<f32>(0.0f, HoverFadeInMS, 0.0f, max, hoverMS), 0.0f, max);
		}
		else
		{
			if (!hovered.LastHoverStopwatch.IsRunning())
				return 0.0f;

			const f32 sinceHoverMS = static_cast<f32>(hovered.LastHoverStopwatch.GetElapsed().TotalMilliseconds());
			return std::clamp(ConvertRange<f32>(0.0f, HoverFadeOutMS, max, 0.0f, sinceHoverMS), 0.0f, max);
		}
	}

	f32 PresetWindow::GetHoverFadeInPreviewOpacity() const
	{
		const f32 hoverMS = static_cast<f32>(hovered.HoverDurationStopwatch.GetElapsed().TotalMilliseconds());
		const f32 opacity = std::clamp<f32>(ConvertRange<f32>(0.0f, HoverFadeInMS, 0.0f, 1.0f, hoverMS), 0.0f, 1.0f);
		return (opacity * opacity);
	}

	void PresetWindow::RenderSyncPresetPreview(Render::Renderer2D& renderer, TargetRenderHelper& renderHelper, u32 targetCount, const std::array<PresetTargetData, Rules::MaxSyncPairCount>& presetTargets)
	{
		assert(targetCount <= presetTargets.size());
		if (targetCount < 1)
			return;

		const f32 hoverFadeOpacity = GetHoverFadeInPreviewOpacity();

		TargetRenderHelper::ButtonSyncLineData syncLineData = {};
		TargetRenderHelper::TargetData targetData = {};

#if 0 // NOTE: Not rendering these as sync targets improves readability when rendering on top of the existing targets to be replaced
		targetData.Sync = (targetCount > 1);
#endif

		syncLineData.SyncPairCount = targetCount;
		syncLineData.Progress = 0.0f;
		syncLineData.Scale = 1.0f;
		syncLineData.Opacity = hoverFadeOpacity;
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
			targetData.Scale = 1.0f;
			targetData.Opacity = hoverFadeOpacity;
			renderHelper.DrawTarget(renderer, targetData);
		}
	}
}
