#include "TargetRenderWindow.h"
#include "TargetGrid.h"
#include "Editor/Core/Theme.h"
#include "Editor/Chart/ChartEditor.h"
#include "Editor/Chart/TargetPropertyRules.h"
#include "Editor/Chart/KeyBindings.h"

namespace Comfy::Studio::Editor
{
	namespace
	{
		constexpr vec2 TargetPositionOrPreset(const TimelineTarget& target)
		{
			return target.Flags.HasProperties ? target.Properties.Position : target.Flags.IsChain ?
				Rules::PresetTargetChainPosition(target.Type, target.Tick, target.Flags) :
				Rules::PresetTargetPosition(target.Type, target.Tick, target.Flags);
		}

		constexpr std::array<u32, EnumCount<ButtonType>()> ButtonTypeColors =
		{
			0xFFCCFE62,
			0xFFD542FF,
			0xFFFEFF62,
			0xFF6412FE,
			0xFF2BD7FF,
			0xFF2BD7FF,
		};

		constexpr u32 GetButtonTypeColorU32(ButtonType type)
		{
			return ButtonTypeColors[static_cast<u8>(type)];
		}
	}

	using namespace Graphics;

	TargetRenderWindow::TargetRenderWindow(ChartEditor& parent, TargetTimeline& timeline, Undo::UndoManager& undoManager, Render::Renderer2D& renderer)
		: chartEditor(parent), timeline(timeline), undoManager(undoManager), renderer(renderer)
	{
		SetWindowBackgroundCheckerboardEnabled(true);

		SetKeepAspectRatio(true);
		SetTargetAspectRatio(Rules::PlacementAreaSize.x / Rules::PlacementAreaSize.y);

		practiceBackground.Data.DrawGrid = false;
		practiceBackground.Data.DrawDim = true;
		practiceBackground.Data.DrawLogo = true;
		practiceBackground.Data.DrawCover = true;
		practiceBackground.Data.DrawBackground = true;

		backgroundCheckerboard.Size = Rules::PlacementAreaSize;
		backgroundCheckerboard.Color = vec4(0.20f, 0.20f, 0.20f, 1.0f);
		backgroundCheckerboard.ColorAlt = vec4(0.26f, 0.26f, 0.26f, 1.0f);

		renderHelper = std::make_unique<TargetRenderHelper>();
		renderHelper->SetAetSprGetter(renderer);

		renderTarget = Render::Renderer2D::CreateRenderTarget();
	}

	void TargetRenderWindow::SetWorkingChart(Chart* chart)
	{
		workingChart = chart;
	}

	ImTextureID TargetRenderWindow::GetTextureID() const
	{
		return (renderTarget != nullptr) ? renderTarget->GetTextureID() : nullptr;
	}

	ImGuiWindowFlags TargetRenderWindow::GetRenderTextureChildWindowFlags() const
	{
		return ImGuiWindowFlags_None;
	}

	void TargetRenderWindow::PreRenderTextureGui()
	{
	}

	void TargetRenderWindow::PostRenderTextureGui()
	{
		auto drawList = Gui::GetWindowDrawList();
		for (const auto& target : workingChart->Targets)
		{
			if (!target.IsSelected)
				continue;

			const auto position = target.Flags.HasProperties ? target.Properties.Position : target.Flags.IsChain ?
				Rules::PresetTargetChainPosition(target.Type, target.Tick, target.Flags) :
				Rules::PresetTargetPosition(target.Type, target.Tick, target.Flags);

			const auto tl = glm::round(TargetAreaToScreenSpace(position - targetHitboxSize));
			const auto br = glm::round(TargetAreaToScreenSpace(position + targetHitboxSize));

			drawList->AddRectFilled(tl, br, GetColor(EditorColor_TimelineSelection));
			drawList->AddRect(tl, br, GetColor(EditorColor_TimelineSelectionBorder));

			drawBuffers.CenterMarkers.push_back({ TargetAreaToScreenSpace(position), GetButtonTypeColorU32(target.Type) });
		}

		const auto markerScreenSize = (camera.Zoom * selectionCenterMarkerSize);
		for (const auto[center, color] : drawBuffers.CenterMarkers)
		{
			drawList->AddLine(center + vec2(-markerScreenSize, -markerScreenSize), center + vec2(+markerScreenSize, +markerScreenSize), color);
			drawList->AddLine(center + vec2(-markerScreenSize, +markerScreenSize), center + vec2(+markerScreenSize, -markerScreenSize), color);
		}
		drawBuffers.CenterMarkers.clear();
	}

	void TargetRenderWindow::OnResize(ivec2 newSize)
	{
		renderTarget->Param.Resolution = newSize;
		renderTarget->Param.ClearColor = GetColorVec4(EditorColor_DarkClear);

		camera.ProjectionSize = vec2(newSize);
		camera.Position = vec2(0.0f, 0.0f);
		camera.Zoom = camera.ProjectionSize.x / Rules::PlacementAreaSize.x;
	}

	void TargetRenderWindow::OnRender()
	{
		renderHelper->UpdateAsyncLoading(renderer);

		UpdateAllInput();

		renderer.Begin(camera, *renderTarget);
		{
			RenderBackground();
			RenderHUDBackground();
			RenderAllVisibleTargets();
		}
		renderer.End();
	}

	void TargetRenderWindow::UpdateAllInput()
	{
		// TODO: Context menu tools selection

		// DEBUG: For now at least...
		Gui::WindowContextMenu("TargetRenderWindowContextMenu", [&]
		{
			if (Gui::WideBeginMenu("Layers", true))
			{
				Gui::Checkbox("Draw Buttons", &layers.DrawButtons);
				Gui::Checkbox("Draw Targets", &layers.DrawTargets);
				Gui::PushItemDisabledAndTextColorIf(!layers.DrawTargets);
				Gui::Checkbox("Draw Target Hands", &layers.DrawTargetHands);
				Gui::PopItemDisabledAndTextColorIf(!layers.DrawTargets);
				Gui::EndMenu();
			}

			if (Gui::WideBeginMenu("Settings", true))
			{
				Gui::Checkbox("Checkerboard", &drawCheckerboard);
				Gui::Checkbox("Target Grid", &drawTargetGrid);

				Gui::SliderFloat("Background Dim", &backgroundDim, 0.0f, 1.0f);

				if (auto t = targetPostHitLingerDuration.Ticks(); Gui::SliderInt("Post Hit Linger Duration", &t, 0, TimelineTick::TicksPerBeat * 8, "%d Ticks"))
					targetPostHitLingerDuration = TimelineTick::FromTicks(t);

				Gui::EndMenu();
			}

			if (Gui::WideBeginMenu("Practice Background", true))
			{
				Gui::Checkbox("Enabled", &practiceBackground.Enabled);
				Gui::PushItemDisabledAndTextColorIf(!practiceBackground.Enabled);
				Gui::Checkbox("Grid", &practiceBackground.Data.DrawGrid);
				Gui::Checkbox("Dim", &practiceBackground.Data.DrawDim);
				Gui::Checkbox("Logo", &practiceBackground.Data.DrawLogo);
				Gui::Checkbox("Cover", &practiceBackground.Data.DrawCover);
				Gui::Checkbox("Background", &practiceBackground.Data.DrawBackground);
				Gui::PopItemDisabledAndTextColorIf(!practiceBackground.Enabled);
				Gui::EndMenu();
			}
			Gui::Separator();
		});

		if (!Gui::IsWindowFocused())
			return;

		if (Gui::IsKeyPressed(KeyBindings::JumpToPreviousTarget, true))
			timeline.AdvanceCursorToNextTarget(-1);

		if (Gui::IsKeyPressed(KeyBindings::JumpToNextTarget, true))
			timeline.AdvanceCursorToNextTarget(+1);

		if (Gui::IsKeyPressed(KeyBindings::TogglePlayback, false))
			timeline.GetIsPlayback() ? timeline.PausePlayback() : timeline.ResumePlayback();
	}

	void TargetRenderWindow::RenderBackground()
	{
		if (drawCheckerboard)
			backgroundCheckerboard.Render(renderer);

		if (practiceBackground.Enabled)
		{
			practiceBackground.Data.PlaybackTime = timeline.GetCursorTime();;
			renderHelper->DrawBackground(renderer, practiceBackground.Data);
		}

		renderer.Draw(Render::RenderCommand2D(vec2(0.0f, 0.0f), Rules::PlacementAreaSize, vec4(0.0f, 0.0f, 0.0f, backgroundDim)));

		if (drawTargetGrid)
			RenderTargetGrid(renderer);
	}

	void TargetRenderWindow::RenderHUDBackground()
	{
		TargetRenderHelper::HUDData hudData;
		hudData.SongTitle = workingChart->SongTitleOrDefault();
		hudData.IsPlayback = timeline.GetIsPlayback();
		hudData.PlaybackTime = timeline.GetCursorTime();
		hudData.PlaybackTimeOnStart = chartEditor.GetPlaybackTimeOnPlaybackStart();
		hudData.Duration = workingChart->DurationOrDefault();

		renderHelper->DrawHUD(renderer, hudData);
	}

	void TargetRenderWindow::RenderAllVisibleTargets()
	{
		AddVisibleTargetsToDrawBuffers();
		FlushRenderTargetsDrawBuffers();
	}

	void TargetRenderWindow::AddVisibleTargetsToDrawBuffers()
	{
		// TODO: Move... somewhere else (?)
		constexpr auto fragDisplayOffsetX = 32.0f;
		constexpr auto chainHitTickThreshold = (TimelineTick::FromBars(1) / 16);

		auto tryGetProperties = [](const TimelineTarget& target)
		{
			return target.Flags.HasProperties ? target.Properties : Rules::PresetTargetProperties(target.Type, target.Tick, target.Flags);
		};

		const auto& targets = workingChart->Targets;
		const bool isPlayback = chartEditor.GetIsPlayback();

		const auto cursorTime = timeline.GetCursorTime();
		const auto cursorTick = timeline.GetCursorTick();

		const TimelineTarget* lastTarget = nullptr;

		for (const auto& target : targets)
		{
			const auto buttonTick = target.Tick;
			const auto buttonTime = workingChart->TimelineMap.GetTimeAt(buttonTick);

			const auto endTick = buttonTick + targetPostHitLingerDuration;
			const auto endTime = workingChart->TimelineMap.GetTimeAt(endTick);

			const auto targetTick = target.Tick - TimelineTick::FromBars(1);
			const auto targetTime = workingChart->TimelineMap.GetTimeAt(targetTick);

			if (target.IsSelected || (cursorTick >= targetTick && cursorTick <= endTick))
			{
				const auto progressUnbound = static_cast<f32>(ConvertRange(targetTime.TotalSeconds(), buttonTime.TotalSeconds(), 0.0, 1.0, cursorTime.TotalSeconds()));
				const auto progress = glm::clamp(progressUnbound, 0.0f, 1.0f);

				auto properties = tryGetProperties(target);

				if (target.Flags.IsChain && !target.Flags.IsChainStart)
					properties.Position.x += fragDisplayOffsetX * (target.Type == ButtonType::SlideL ? -1.0f : +1.0f);

				const bool inCursorBarRange = (cursorTick >= targetTick && cursorTick <= buttonTick);

				auto& targetData = drawBuffers.Targets.emplace_back();
				targetData.Type = target.Type;
				targetData.NoHand = (!layers.DrawTargetHands || !inCursorBarRange);
				// NOTE: Transparent to make the background grid visible and make pre-, post- and cursor bar targets look more uniform
				targetData.Transparent = (target.IsSelected || !inCursorBarRange);
				targetData.NoScale = !isPlayback;
				targetData.Sync = target.Flags.IsSync;
				targetData.HoldText = target.Flags.IsHold;
				targetData.Chain = target.Flags.IsChain;
				targetData.ChainStart = target.Flags.IsChainStart;
				targetData.ChainHit = (target.Flags.IsChain && ((buttonTick - cursorTick) <= chainHitTickThreshold));
				targetData.Position = properties.Position;
				targetData.Progress = progress;

				if (inCursorBarRange)
				{
					auto& buttonData = drawBuffers.Buttons.emplace_back();
					buttonData.Type = targetData.Type;
					buttonData.Sync = targetData.Sync;
					buttonData.Chain = targetData.Chain;
					buttonData.ChainStart = targetData.ChainStart;
					buttonData.Shadow = TargetRenderHelper::ButtonShadowType::Black;
					buttonData.Position = GetButtonPathSinePoint(progress, properties);
					buttonData.Progress = progress;

					if (!buttonData.Sync)
					{
						const auto flyDuration = (buttonTime - targetTime);

						auto& trailData = drawBuffers.Trails.emplace_back();
						constexpr f32 trailFactor = 2.55f;
						const f32 pixelLength = (properties.Distance / 1000.0f) * (240.0f / static_cast<f32>(flyDuration.TotalSeconds()) * trailFactor);
						const f32 normalizedLength = pixelLength / properties.Distance;
						trailData.Type = target.Type;
						trailData.Chance = false;
						trailData.Properties = properties;
						trailData.Progress = static_cast<f32>(flyDuration.TotalSeconds()) * progressUnbound;
						trailData.ProgressStart = progress;
						trailData.ProgressEnd = (progress - normalizedLength);
					}

					if (target.Flags.IsSync && target.Flags.IndexWithinSyncPair == 0)
					{
						auto& syncLineData = drawBuffers.SyncLines.emplace_back();
						syncLineData.SyncPairCount = target.Flags.SyncPairCount;
						syncLineData.Progress = progressUnbound;

						const auto thisIndex = std::distance(&targets[0], &target);
						for (size_t i = 0; i < std::min(4u, syncLineData.SyncPairCount); i++)
						{
							const auto& syncTarget = targets[thisIndex + i];
							const auto syncTargetProperty = tryGetProperties(syncTarget);

							syncLineData.TargetPositions[i] = syncTargetProperty.Position;
							syncLineData.ButtonPositions[i] = GetButtonPathSinePoint(progress, syncTargetProperty);
						}
					}
				}
			}

			lastTarget = &target;
		}
	}

	void TargetRenderWindow::FlushRenderTargetsDrawBuffers()
	{
		if (layers.DrawButtons)
		{
			for (const auto& data : drawBuffers.Trails)
				renderHelper->DrawButtonTrail(renderer, data);

			for (const auto& data : drawBuffers.Buttons)
				if (data.Shadow != TargetRenderHelper::ButtonShadowType::None) { renderHelper->DrawButtonShadow(renderer, data); }
		}

		if (layers.DrawTargets)
		{
			// NOTE: Draw chain starts on top of child fragments to make sure the target hand is always fully visible
			for (const auto& data : drawBuffers.Targets)
				if (!data.ChainHit && !data.ChainStart) { renderHelper->DrawTarget(renderer, data); }

			for (const auto& data : drawBuffers.Targets)
				if (!data.ChainHit && data.ChainStart) { renderHelper->DrawTarget(renderer, data); }

			for (const auto& data : drawBuffers.Targets)
				if (data.ChainHit) { renderHelper->DrawTarget(renderer, data); }
		}

		if (layers.DrawButtons)
		{
			for (const auto& data : drawBuffers.SyncLines)
				renderHelper->DrawButtonPairSyncLines(renderer, data);

			for (const auto& data : drawBuffers.Buttons)
				renderHelper->DrawButton(renderer, data);
		}

		drawBuffers.Targets.clear();
		drawBuffers.Buttons.clear();
		drawBuffers.Trails.clear();
		drawBuffers.SyncLines.clear();
	}

	vec2 TargetRenderWindow::TargetAreaToScreenSpace(const vec2 targetAreaSpace) const
	{
		const auto renderRegion = GetRenderRegion();
		const auto scale = (renderRegion.GetSize() / Rules::PlacementAreaSize);

		return renderRegion.GetTL() + (targetAreaSpace * scale);
	}

	vec2 TargetRenderWindow::ScreenToTargetAreaSpace(const vec2 screenSpace) const
	{
		const auto renderRegion = GetRenderRegion();
		const auto scale = (Rules::PlacementAreaSize / renderRegion.GetSize());

		return (screenSpace - renderRegion.GetTL()) * scale;
	}
}
