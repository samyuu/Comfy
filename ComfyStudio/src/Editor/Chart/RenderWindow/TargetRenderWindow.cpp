#include "TargetRenderWindow.h"
#include "Editor/Core/Theme.h"
#include "Editor/Chart/ChartEditor.h"
#include "Editor/Chart/TargetPropertyRules.h"
#include "Editor/Chart/KeyBindings.h"

namespace Comfy::Studio::Editor
{
	constexpr auto GridColor = vec4(0.527f, 0.527f, 0.527f, 0.737f);
	constexpr auto GridColorSub = vec4(0.586f, 0.586f, 0.586f, 0.255f);

	constexpr auto GridLineCount = ivec2(19, 8);
	constexpr auto GridSubDivisions = 8;

	constexpr auto GridVertexCountRough = (GridLineCount.x + GridLineCount.y) * 2;
	constexpr auto GridVertexCountSub = (((GridLineCount.x - 1) * GridSubDivisions + 1) + ((GridLineCount.y - 1) * GridSubDivisions + 1)) * 2;
	constexpr auto GridVertexCount = GridVertexCountRough + GridVertexCountSub;

	static_assert(GridVertexCountRough == 54);
	static_assert(GridVertexCountSub == 404);
	static_assert(GridVertexCount == 458);

	constexpr std::array<Render::PositionTextureColorVertex, GridVertexCount> GenerateGridVertices()
	{
		std::array<Render::PositionTextureColorVertex, GridVertexCount> vertices = {};

		for (size_t i = 0; i < GridVertexCount; i++)
			vertices[i].TextureCoordinates = vec2(0.5f);

		for (size_t i = 0; i < GridVertexCountRough; i++)
			vertices[i].Color = GridColor;

		for (size_t i = GridVertexCountRough; i < GridVertexCount; i++)
			vertices[i].Color = GridColorSub;

		constexpr auto min = Rules::RecommendedPlacementAreaMin, max = Rules::RecommendedPlacementAreaMax;
		constexpr auto gridStep = Rules::TickToDistance((TimelineTick::FromBars(1) / 8));
		constexpr auto gridStepSub = Rules::TickToDistance((TimelineTick::FromBars(1) / 8)) / static_cast<f32>(GridSubDivisions);

		size_t v = 0;
		for (i32 x = 0; x < GridLineCount.x; x++)
		{
			vertices[v++].Position = min + vec2(gridStep * x, 0.0f);
			vertices[v++].Position = min + vec2(gridStep * x, max.y - min.y);
		}
		for (i32 y = 0; y < GridLineCount.y; y++)
		{
			vertices[v++].Position = min + vec2(0.0f, gridStep * y);
			vertices[v++].Position = min + vec2(max.x - min.x, gridStep * y);
		}

		for (i32 x = 0; x <= (GridLineCount.x - 1) * GridSubDivisions; x++)
		{
			vertices[v++].Position = min + vec2(gridStepSub * x, 0.0f);
			vertices[v++].Position = min + vec2(gridStepSub * x, max.y - min.y);
		}
		for (i32 y = 0; y <= (GridLineCount.y - 1) * GridSubDivisions; y++)
		{
			vertices[v++].Position = min + vec2(0.0f, gridStepSub * y);
			vertices[v++].Position = min + vec2(max.x - min.x, gridStepSub * y);
		}

		return vertices;
	}
}

namespace Comfy::Studio::Editor
{
	namespace
	{
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

		practiceBackgroundData.DrawGrid = false;
		practiceBackgroundData.DrawDim = true;
		practiceBackgroundData.DrawLogo = true;
		practiceBackgroundData.DrawCover = true;
		practiceBackgroundData.DrawBackground = true;

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

		if (drawPracticeBackground)
		{
			practiceBackgroundData.PlaybackTime = timeline.GetCursorTime();;
			renderHelper->DrawBackground(renderer, practiceBackgroundData);
		}

		renderer.Draw(Render::RenderCommand2D(vec2(0.0f, 0.0f), Rules::PlacementAreaSize, vec4(0.0f, 0.0f, 0.0f, backgroundDim)));

		if (drawTargetGrid)
		{
			// DEBUG: Not entirely sure about this yet...
			static constexpr auto gridVertices = GenerateGridVertices();
			renderer.DrawVertices(gridVertices.data(), gridVertices.size(), nullptr, AetBlendMode::Normal, PrimitiveType::Lines);
		}
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
				targetData.NoHand = (!drawTargetHands || !inCursorBarRange);
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
		if (drawButtons)
		{
			for (const auto& data : drawBuffers.Trails)
				renderHelper->DrawButtonTrail(renderer, data);

			for (const auto& data : drawBuffers.Buttons)
				if (data.Shadow != TargetRenderHelper::ButtonShadowType::None) { renderHelper->DrawButtonShadow(renderer, data); }
		}

		if (drawTargets)
		{
			// NOTE: Draw chain starts on top of child fragments to make sure the target hand is always fully visible
			for (const auto& data : drawBuffers.Targets)
				if (!data.ChainHit && !data.ChainStart) { renderHelper->DrawTarget(renderer, data); }

			for (const auto& data : drawBuffers.Targets)
				if (!data.ChainHit && data.ChainStart) { renderHelper->DrawTarget(renderer, data); }

			for (const auto& data : drawBuffers.Targets)
				if (data.ChainHit) { renderHelper->DrawTarget(renderer, data); }
		}

		if (drawButtons)
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
