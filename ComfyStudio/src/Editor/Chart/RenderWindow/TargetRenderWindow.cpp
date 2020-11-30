#include "TargetRenderWindow.h"
#include "TargetGrid.h"
#include "Editor/Core/Theme.h"
#include "Editor/Chart/ChartEditor.h"
#include "Editor/Chart/ChartCommands.h"
#include "Editor/Chart/KeyBindings.h"

namespace Comfy::Studio::Editor
{
	using namespace Graphics;

	TargetRenderWindow::TargetRenderWindow(ChartEditor& parent, TargetTimeline& timeline, Undo::UndoManager& undoManager, Render::Renderer2D& renderer)
		: chartEditor(parent), timeline(timeline), undoManager(undoManager), renderer(renderer), availableTools(TargetTool::CreateAllToolTypes(*this, undoManager))
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

		SelectActiveTool(TargetToolType::StartupType);
	}

	void TargetRenderWindow::SetWorkingChart(Chart* chart)
	{
		workingChart = chart;
	}

	void TargetRenderWindow::RegisterRenderCallback(TargetRenderWindowRenderCallback onRender)
	{
		onRenderCallback = std::move(onRender);
	}

	void TargetRenderWindow::RegisterOverlayGuiCallback(TargetRenderWindowOverlayGuiCallback onOverlayGui)
	{
		onOverlayGuiCallback = std::move(onOverlayGui);
	}

	ImTextureID TargetRenderWindow::GetTextureID() const
	{
		return (renderTarget != nullptr) ? renderTarget->GetTextureID() : nullptr;
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

	const Render::Camera2D& TargetRenderWindow::GetCamera() const
	{
		return camera;
	}

	TargetRenderHelper& TargetRenderWindow::GetRenderHelper()
	{
		assert(renderHelper != nullptr);
		return *renderHelper;
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
		DrawOverlayGui();

		auto drawList = Gui::GetWindowDrawList();

		auto selectedTool = GetSelectedTool();
		if (selectedTool != nullptr)
			selectedTool->PreRenderGUI(*workingChart, *drawList);

		// NOTE: Avoid overflowing 16-bit vertex buffer indices
		constexpr size_t maxSelectionToDraw = 512;
		size_t selectionDrawCount = 0;

		for (const auto& target : workingChart->Targets)
		{
			if (!target.IsSelected)
				continue;

			const auto position = Rules::TryGetProperties(target).Position;
			const auto tl = glm::round(TargetAreaToScreenSpace(position - TargetHitboxSize));
			const auto br = glm::round(TargetAreaToScreenSpace(position + TargetHitboxSize));

			drawList->AddRectFilled(tl, br, GetColor(EditorColor_TimelineSelection));
			drawList->AddRect(tl, br, GetColor(EditorColor_TimelineSelectionBorder));

			centerMarkersBuffer.push_back({ TargetAreaToScreenSpace(position), GetButtonTypeColorU32(target.Type) });
			if (selectionDrawCount++ >= maxSelectionToDraw)
				break;
		}

		const auto markerScreenSize = (camera.Zoom * SelectionCenterMarkerSize);
		for (const auto[center, color] : centerMarkersBuffer)
		{
			drawList->AddLine(center + vec2(-markerScreenSize, -markerScreenSize), center + vec2(+markerScreenSize, +markerScreenSize), color);
			drawList->AddLine(center + vec2(-markerScreenSize, +markerScreenSize), center + vec2(+markerScreenSize, -markerScreenSize), color);
		}
		centerMarkersBuffer.clear();

		boxSelectionTool.DrawSelection(*drawList);

		if (selectedTool != nullptr)
			selectedTool->PostRenderGUI(*workingChart, *drawList);

		if (onOverlayGuiCallback)
			onOverlayGuiCallback(*this, *drawList);
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
		UpdateAllInput();

		renderHelper->UpdateAsyncLoading(renderer);
		renderer.Begin(camera, *renderTarget);
		{
			RenderBackground();
			RenderHUDBackground();

			auto selectedTool = GetSelectedTool();
			if (selectedTool != nullptr)
				selectedTool->PreRender(*workingChart, renderer);

			RenderAllVisibleTargets();

			if (selectedTool != nullptr)
				selectedTool->PostRender(*workingChart, renderer);

			if (onRenderCallback)
				onRenderCallback(*this, renderer);
		}
		renderer.End();
	}

	void TargetRenderWindow::DrawOverlayGui()
	{
		auto selectedTool = GetSelectedTool();
		if (selectedTool != nullptr)
			selectedTool->OnOverlayGUI(*workingChart);
	}

	void TargetRenderWindow::UpdateAllInput()
	{
		// TODO: "CTRL + A", "CTRL + D" to select all targets on screen and deselect all (?)

		UpdateInputContextMenu();
		boxSelectionTool.UpdateInput(*workingChart, timeline.GetCursorTick(), targetPostHitLingerDuration);

		if (Gui::IsWindowFocused())
		{
			if (Gui::IsKeyPressed(KeyBindings::JumpToPreviousTarget, true))
				timeline.AdvanceCursorToNextTarget(-1);

			if (Gui::IsKeyPressed(KeyBindings::JumpToNextTarget, true))
				timeline.AdvanceCursorToNextTarget(+1);

			if (Gui::IsKeyPressed(KeyBindings::TogglePlayback, false))
				timeline.GetIsPlayback() ? timeline.PausePlayback() : timeline.ResumePlayback();

			for (size_t toolIndex = 0; toolIndex < EnumCount<TargetToolType>(); toolIndex++)
			{
				if (Gui::IsKeyPressed(KeyBindings::TargetToolTypes[toolIndex], false))
					SelectActiveTool(static_cast<TargetToolType>(toolIndex));
			}
		}

		if (auto selectedTool = GetSelectedTool(); selectedTool != nullptr)
			selectedTool->UpdateInput(*workingChart);
	}

	void TargetRenderWindow::UpdateInputContextMenu()
	{
		constexpr const char* contextMenuID = "TargetRenderWindowContextMenu";

		if (Gui::IsMouseReleased(1) && Gui::IsWindowHovered(ImGuiHoveredFlags_AllowWhenBlockedByPopup) && !Gui::IsAnyItemHovered())
			Gui::OpenPopup(contextMenuID);

		if (Gui::BeginPopup(contextMenuID, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoMove))
		{
			std::optional<TargetToolType> newToolToSelect = {};

			for (size_t toolIndex = 0; toolIndex < availableTools.size(); toolIndex++)
			{
				const auto toolType = static_cast<TargetToolType>(toolIndex);
				const auto tool = availableTools[toolIndex].get();
				const char* toolName = (tool != nullptr) ? tool->GetName() : "<Null Tool>";

				bool isSelected = (toolType == selectedToolType);
				if (Gui::MenuItem(toolName, Input::GetKeyCodeName(KeyBindings::TargetToolTypes[toolIndex]), &isSelected, !isSelected))
					newToolToSelect = toolType;
			}
			Gui::Separator();

			if (auto selectedTool = GetSelectedTool(); selectedTool != nullptr)
				selectedTool->OnContextMenuGUI(*workingChart);

			if (Gui::BeginMenu("Settings", true))
			{
				if (Gui::BeginMenu("Target Area", true))
				{
					Gui::Checkbox("Show Target Buttons", &layers.DrawButtons);
					Gui::Checkbox("Show Target Grid", &drawTargetGrid);

					Gui::Checkbox("Background Checkerboard", &drawCheckerboard);

					if (auto p = (backgroundDim * 100.0f); Gui::SliderFloat("##BackgroundDimSlider", &p, 0.0f, 100.0f, "Background Dim: %.f%%"))
						backgroundDim = (p / 100.0f);

					if (auto t = targetPostHitLingerDuration.Ticks(); Gui::SliderInt("##PostHitLingerSlider", &t, 0, BeatTick::TicksPerBeat * 8, "Post Hit Linger: %d Ticks"))
						targetPostHitLingerDuration = BeatTick::FromTicks(t);

					Gui::EndMenu();
				}

				if (Gui::BeginMenu("Practice Background", true))
				{
					Gui::Checkbox("Enabled", &practiceBackground.Enabled);
					Gui::PushItemDisabledAndTextColorIf(!practiceBackground.Enabled);
					Gui::Checkbox("Grid", &practiceBackground.Data.DrawGrid);
					Gui::Checkbox("Dim", &practiceBackground.Data.DrawDim);
					Gui::Checkbox("Cover", &practiceBackground.Data.DrawCover);
					Gui::Checkbox("Logo", &practiceBackground.Data.DrawLogo);
					Gui::Checkbox("Background", &practiceBackground.Data.DrawBackground);
					Gui::PopItemDisabledAndTextColorIf(!practiceBackground.Enabled);
					Gui::EndMenu();
				}

				if (Gui::MenuItem("Set Default Editor View"))
				{
					backgroundDim = 0.35f;
					drawTargetGrid = true;
					practiceBackground.Enabled = false;
				}

				if (Gui::MenuItem("Set Default Practice View"))
				{
					backgroundDim = 0.0f;
					drawTargetGrid = false;
					practiceBackground.Enabled = true;
					practiceBackground.Data.DrawGrid = true;
					practiceBackground.Data.DrawDim = true;
					practiceBackground.Data.DrawCover = true;
					practiceBackground.Data.DrawLogo = true;
					practiceBackground.Data.DrawBackground = true;
				}

				Gui::EndMenu();
			}

			// NOTE: Avoid mid-frame tool switching to prevent ugly single frame context menu changes
			if (newToolToSelect.has_value())
				selectedToolType = newToolToSelect.value();

			Gui::EndPopup();
		}
	}

	void TargetRenderWindow::SelectActiveTool(TargetToolType toolType)
	{
		if (selectedToolType == toolType)
			return;

		if (auto deselectedTool = GetSelectedTool(); deselectedTool != nullptr)
			deselectedTool->OnDeselected();

		selectedToolType = toolType;

		if (auto selectedTool = GetSelectedTool(); selectedTool != nullptr)
			selectedTool->OnSelected();
	}

	TargetTool* TargetRenderWindow::GetSelectedTool()
	{
		const auto tool = IndexOrNull(static_cast<u8>(selectedToolType), availableTools);
		return (tool != nullptr) ? tool->get() : nullptr;
	}

	void TargetRenderWindow::RenderBackground()
	{
		if (drawCheckerboard)
			backgroundCheckerboard.Render(renderer);

		if (practiceBackground.Enabled)
		{
			practiceBackground.Data.PlaybackTime = timeline.GetCursorTime();
			practiceBackground.Data.CoverSprite = workingChart->Properties.Image.Cover.GetTexSprView();
			practiceBackground.Data.LogoSprite = workingChart->Properties.Image.Logo.GetTexSprView();
			practiceBackground.Data.BackgroundSprite = workingChart->Properties.Image.Background.GetTexSprView();
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
		hudData.Difficulty = workingChart->Properties.Difficulty.Type;
		hudData.IsPlayback = timeline.GetIsPlayback();
		hudData.PlaybackTime = timeline.GetCursorTime();
		hudData.PlaybackTimeOnStart = chartEditor.GetPlaybackTimeOnPlaybackStart();
		hudData.Duration = workingChart->DurationOrDefault();

		renderHelper->DrawHUD(renderer, hudData);
	}

	void TargetRenderWindow::RenderAllVisibleTargets()
	{
		AddVisibleTargetsToDrawBuffers();

		TargetRenderHelperExFlushFlags flags = TargetRenderHelperExFlushFlags_None;
		if (!layers.DrawButtons)
			flags |= TargetRenderHelperExFlushFlags_NoButtons;
		if (!layers.DrawTargets)
			flags |= TargetRenderHelperExFlushFlags_NoTargets;
		renderHelperEx.Flush(renderer, *renderHelper, flags);
	}

	void TargetRenderWindow::AddVisibleTargetsToDrawBuffers()
	{
		constexpr auto chainHitTickThreshold = (BeatTick::FromBars(1) / 16);

		const auto& targets = workingChart->Targets;
		const bool isPlayback = chartEditor.GetIsPlayback();

		const auto cursorTime = timeline.GetCursorTime();
		const auto cursorTick = timeline.GetCursorTick();

		for (const auto& target : targets)
		{
			const auto buttonTick = target.Tick;
			const auto buttonTime = workingChart->TimelineMap.GetTimeAt(buttonTick);

			const auto endTick = buttonTick + targetPostHitLingerDuration;
			const auto endTime = workingChart->TimelineMap.GetTimeAt(endTick);

			const auto targetTick = target.Tick - BeatTick::FromBars(1);
			const auto targetTime = workingChart->TimelineMap.GetTimeAt(targetTick);

			if (target.IsSelected || (cursorTick >= targetTick && cursorTick <= endTick))
			{
				const auto progressUnbound = static_cast<f32>(ConvertRange(targetTime.TotalSeconds(), buttonTime.TotalSeconds(), 0.0, 1.0, cursorTime.TotalSeconds()));
				const auto progress = glm::clamp(progressUnbound, 0.0f, 1.0f);

				auto properties = Rules::TryGetProperties(target);

				if (target.Flags.IsChain && !target.Flags.IsChainStart)
					properties.Position.x += Rules::ChainFragmentStartEndOffsetDistance * (target.Type == ButtonType::SlideL ? -1.0f : +1.0f);

				const bool inCursorBarRange = (cursorTick >= targetTick && cursorTick <= buttonTick);

				auto& targetData = renderHelperEx.EmplaceTarget();
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
				targetData.Chance = target.Flags.IsChance;
				targetData.Position = properties.Position;
				targetData.Progress = progress;
				targetData.Scale = 1.0f;

				if (inCursorBarRange)
				{
					auto& buttonData = renderHelperEx.EmplaceButton();
					buttonData.Type = targetData.Type;
					buttonData.Sync = targetData.Sync;
					buttonData.Chain = targetData.Chain;
					buttonData.ChainStart = targetData.ChainStart;
					buttonData.Shadow = TargetRenderHelper::ButtonShadowType::Black;
					buttonData.Position = GetButtonPathSinePoint(progress, properties);
					buttonData.Progress = progress;
					buttonData.Scale = 1.0f;

					if (!buttonData.Sync)
					{
						const auto flyDuration = (buttonTime - targetTime);
						renderHelperEx.ConstructButtonTrail(renderHelperEx.EmplaceButtonTrail(), target.Type, progress, progressUnbound, properties, flyDuration);
					}

					if (target.Flags.IsSync && target.Flags.IndexWithinSyncPair == 0)
					{
						auto& syncLineData = renderHelperEx.EmplaceSyncLine();
						syncLineData.SyncPairCount = target.Flags.SyncPairCount;
						syncLineData.Progress = progressUnbound;
						syncLineData.Scale = 1.0f;
						syncLineData.Opacity = 1.0f;

						const auto thisIndex = std::distance(&targets[0], &target);
						for (size_t i = 0; i < std::min(4u, syncLineData.SyncPairCount); i++)
						{
							const auto& syncTarget = targets[thisIndex + i];
							const auto syncTargetProperty = Rules::TryGetProperties(syncTarget);

							syncLineData.TargetPositions[i] = syncTargetProperty.Position;
							syncLineData.ButtonPositions[i] = GetButtonPathSinePoint(progress, syncTargetProperty);
						}
					}
				}
			}
		}
	}
}
