#include "TargetRenderWindow.h"
#include "Editor/Core/Theme.h"
#include "Editor/Chart/ChartEditor.h"
#include "Editor/Chart/TargetPropertyRules.h"
#include "Editor/Chart/KeyBindings.h"

namespace Comfy::Studio::Editor
{
	using namespace Graphics;

	TargetRenderWindow::TargetRenderWindow(ChartEditor& parent, TargetTimeline& timeline, Undo::UndoManager& undoManager, Render::Renderer2D& renderer)
		: chartEditor(parent), timeline(timeline), undoManager(undoManager), renderer(renderer)
	{
		SetWindowBackgroundCheckerboardEnabled(true);

		SetKeepAspectRatio(true);
		SetTargetAspectRatio(Rules::PlacementAreaSize.x / Rules::PlacementAreaSize.y);

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

			const auto position = target.Flags.HasProperties ? target.Properties.Position : Rules::PresetTargetPosition(target.Type, target.Tick, target.Flags);

			const auto tl = glm::round(TargetAreaToScreenSpace(position - targetHitboxSize));
			const auto br = glm::round(TargetAreaToScreenSpace(position + targetHitboxSize));

			drawList->AddRectFilled(tl, br, GetColor(EditorColor_TimelineSelection));
			drawList->AddRect(tl, br, GetColor(EditorColor_TimelineSelectionBorder));
		}
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
		backgroundCheckerboard.Size = Rules::PlacementAreaSize;
		backgroundCheckerboard.Render(renderer);

		renderer.Draw(Render::RenderCommand2D(vec2(0.0f, 0.0f), Rules::PlacementAreaSize, vec4(0.0f, 0.0f, 0.0f, backgroundDim)));

#if COMFY_DEBUG && 0 // DEBUG:
		const auto rect = ImRect { Rules::RecommendedPlacementAreaMin, Rules::RecommendedPlacementAreaMax };
		// renderer.Draw(Render::RenderCommand2D(rect.GetTL(), rect.GetSize(), vec4(0.0f, 0.0f, 0.0f, 0.25f)));
		// renderer.DrawRect(rect.GetTL(), rect.GetTR(), rect.GetBL(), rect.GetBR(), vec4(0.0f, 0.0f, 0.0f, 0.5f), 2.0f);
#endif
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

			const auto endTick = buttonTick + TimelineTick::FromBeats(1);
			const auto endTime = workingChart->TimelineMap.GetTimeAt(endTick);

			const auto targetTick = target.Tick - TimelineTick::FromBars(1);
			const auto targetTime = workingChart->TimelineMap.GetTimeAt(targetTick);

			const auto flyDuration = (buttonTime - targetTime);

			if (target.IsSelected || (cursorTick >= targetTick && cursorTick <= endTick))
			{
				const auto progressUnbound = static_cast<f32>(ConvertRange(targetTime.TotalSeconds(), buttonTime.TotalSeconds(), 0.0, 1.0, cursorTime.TotalSeconds()));
				const auto progress = glm::clamp(progressUnbound, 0.0f, 1.0f);

				const auto properties = tryGetProperties(target);

				auto& targetData = drawBuffers.Targets.emplace_back();
				targetData.Type = target.Type;
				targetData.NoHand = (cursorTick > buttonTick || cursorTick < targetTick);
				targetData.Transparent = targetData.NoHand;
				targetData.NoScale = !isPlayback;
				targetData.Sync = target.Flags.IsSync;
				targetData.HoldText = target.Flags.IsHold;
				targetData.Chain = (target.Flags.IsChain && !target.Flags.IsChainStart);
				targetData.ChainHit = false;
				targetData.Position = properties.Position;
				targetData.Progress = progress;

				if (!targetData.NoHand)
				{
					auto& buttonData = drawBuffers.Buttons.emplace_back();
					buttonData.Type = target.Type;
					buttonData.Sync = target.Flags.IsSync;
					buttonData.Chain = (target.Flags.IsChain && !target.Flags.IsChainStart);
					buttonData.Shadow = (isPlayback || true) ? TargetRenderHelper::ButtonShadowType::Black : TargetRenderHelper::ButtonShadowType::None;
					buttonData.Position = GetButtonPathSinePoint(progress, properties);
					buttonData.Progress = progress;

					if (!buttonData.Sync)
					{
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
		for (const auto& data : drawBuffers.Trails)
			renderHelper->DrawButtonTrail(renderer, data);

		for (const auto& data : drawBuffers.Buttons)
		{
			if (data.Shadow != TargetRenderHelper::ButtonShadowType::None)
				renderHelper->DrawButtonShadow(renderer, data);
		}

		for (const auto& data : drawBuffers.Targets)
			renderHelper->DrawTarget(renderer, data);

		for (const auto& data : drawBuffers.SyncLines)
			renderHelper->DrawButtonPairSyncLines(renderer, data);

		for (const auto& data : drawBuffers.Buttons)
			renderHelper->DrawButton(renderer, data);

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
