#include "TargetRenderWindow.h"
#include "TargetGrid.h"
#include "Core/ComfyStudioSettings.h"
#include "Editor/Core/Theme.h"
#include "Editor/Chart/ChartEditor.h"
#include "Editor/Chart/ChartCommands.h"

namespace Comfy::Studio::Editor
{
	using namespace Graphics;

	TargetRenderWindow::TargetRenderWindow(ChartEditor& parent, TargetTimeline& timeline, Undo::UndoManager& undoManager, Render::Renderer2D& renderer)
		: chartEditor(parent), timeline(timeline), undoManager(undoManager), renderer(renderer), availableTools(TargetTool::CreateAllToolTypes(*this, undoManager))
	{
		SetWindowBackgroundCheckerboardEnabled(true);

		SetKeepAspectRatio(true);
		SetTargetAspectRatio(Rules::PlacementAreaSize.x / Rules::PlacementAreaSize.y);

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

			if (GlobalUserData.TargetPreview.ShowHoldInfo)
				RenderSyncHoldInfoBackground();

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
		boxSelectionTool.UpdateInput(*workingChart, timeline.GetCursorTick(), GlobalUserData.TargetPreview.PostHitLingerDuration);

		if (Gui::IsWindowFocused())
		{
			if (Input::IsAnyPressed(GlobalUserData.Input.TargetPreview_JumpToPreviousTarget, true, Input::ModifierBehavior_Relaxed))
				timeline.AdvanceCursorToNextTarget(-1);

			if (Input::IsAnyPressed(GlobalUserData.Input.TargetPreview_JumpToNextTarget, true, Input::ModifierBehavior_Relaxed))
				timeline.AdvanceCursorToNextTarget(+1);

			if (Input::IsAnyPressed(GlobalUserData.Input.TargetPreview_TogglePlayback, false, Input::ModifierBehavior_Relaxed))
				timeline.GetIsPlayback() ? timeline.PausePlayback() : timeline.ResumePlayback();

			if (Input::IsAnyPressed(GlobalUserData.Input.TargetPreview_SelectPositionTool, false))
				SelectActiveTool(TargetToolType::Position);
			if (Input::IsAnyPressed(GlobalUserData.Input.TargetPreview_SelectPathTool, false))
				SelectActiveTool(TargetToolType::Path);
		}

		if (auto selectedTool = GetSelectedTool(); selectedTool != nullptr)
			selectedTool->UpdateInput(*workingChart);
	}

	void TargetRenderWindow::UpdateInputContextMenu()
	{
		constexpr const char* contextMenuID = "TargetRenderWindowContextMenu";

		if (Gui::IsMouseReleased(ImGuiMouseButton_Right) && Gui::IsWindowHovered(ImGuiHoveredFlags_AllowWhenBlockedByPopup) && !Gui::IsAnyItemHovered())
			Gui::OpenPopup(contextMenuID);

		if (Gui::BeginPopup(contextMenuID, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoMove))
		{
			TargetToolType newToolToSelect = TargetToolType::Count;
			for (size_t toolIndex = 0; toolIndex < availableTools.size(); toolIndex++)
			{
				const auto toolType = static_cast<TargetToolType>(toolIndex);
				const auto tool = availableTools[toolIndex].get();
				const char* toolName = (tool != nullptr) ? tool->GetName() : "<Null Tool>";

				bool isSelected = (toolType == selectedToolType);
				const auto* binding =
					(toolType == TargetToolType::Position) ? &GlobalUserData.Input.TargetPreview_SelectPositionTool :
					(toolType == TargetToolType::Path) ? &GlobalUserData.Input.TargetPreview_SelectPathTool : nullptr;
				assert(binding != nullptr);

				if (Gui::MenuItem(toolName, Input::ToString(*binding).data(), &isSelected, !isSelected))
					newToolToSelect = toolType;
			}
			Gui::Separator();

			if (auto selectedTool = GetSelectedTool(); selectedTool != nullptr)
				selectedTool->OnContextMenuGUI(*workingChart);

			// NOTE: Avoid mid-frame tool switching to prevent ugly single frame context menu changes
			if (newToolToSelect < TargetToolType::Count)
				selectedToolType = newToolToSelect;

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
		// NOTE: Always draw underneath in case the assets haven't been loaded yet
		if (GlobalUserData.TargetPreview.ShowBackgroundCheckerboard)
			backgroundCheckerboard.Render(renderer);

		if (GlobalUserData.TargetPreview.BackgroundDim > 0.0f)
			renderer.Draw(Render::RenderCommand2D(vec2(0.0f, 0.0f), Rules::PlacementAreaSize, vec4(0.0f, 0.0f, 0.0f, GlobalUserData.TargetPreview.BackgroundDim)));

		if (GlobalUserData.TargetPreview.DisplayPracticeBackground)
		{
			TargetRenderHelper::BackgroundData backgroundData;
			backgroundData.DrawGrid = true;
			backgroundData.DrawDim = true;
			backgroundData.DrawCover = true;
			backgroundData.DrawLogo = true;
			backgroundData.DrawBackground = true;
			backgroundData.PlaybackTime = timeline.GetCursorTime();
			backgroundData.CoverSprite = workingChart->Properties.Image.Cover.GetTexSprView();
			backgroundData.LogoSprite = workingChart->Properties.Image.Logo.GetTexSprView();
			backgroundData.BackgroundSprite = workingChart->Properties.Image.Background.GetTexSprView();
			renderHelper->DrawBackground(renderer, backgroundData);
		}
		else if (GlobalUserData.TargetPreview.ShowGrid)
		{
			RenderTargetGrid(renderer);
		}
	}

	void TargetRenderWindow::RenderHUDBackground()
	{
		TargetRenderHelper::HUDData hudData;
		hudData.SongTitle = workingChart->SongTitleOrDefault();
		hudData.Difficulty = workingChart->Properties.Difficulty.Type;
		hudData.PlaybackTime = timeline.GetCursorTime();
		hudData.RestartTime = timeline.GetIsPlayback() ? chartEditor.GetPlaybackTimeOnPlaybackStart() : hudData.PlaybackTime;
		hudData.Duration = workingChart->DurationOrDefault();
		hudData.DrawPracticeInfo = false;

		renderHelper->DrawHUD(renderer, hudData);
	}

	void TargetRenderWindow::RenderSyncHoldInfoBackground()
	{
		const auto cursorTick = timeline.GetCursorTick();
		const auto cursorTime = timeline.GetCursorTime();

		auto checkAddHoldMaxEvent = [](std::vector<HoldEvent>& holdEventStack, TimeSpan timeToCheckAgainst)
		{
			if (holdEventStack.empty())
				return;

			const auto lastEvent = holdEventStack.back();
			if (lastEvent.EventType != HoldEventType::Start && lastEvent.EventType != HoldEventType::Addition)
				return;

			if (timeToCheckAgainst >= lastEvent.StartTime + MaxTargetHoldDuration)
			{
				auto& newMaxEvent = holdEventStack.emplace_back();
				newMaxEvent.EventType = HoldEventType::MaxOut;
				newMaxEvent.CombinedButtonTypes = ButtonTypeFlags_None;
				newMaxEvent.TargetPairIndex = -1;
				newMaxEvent.StartTime = lastEvent.StartTime + MaxTargetHoldDuration;
			}
		};

		for (size_t i = 0; i < workingChart->Targets.size();)
		{
			const auto& firstTargetOfPair = workingChart->Targets[i];
			const size_t firstTargetOfPairIndex = i;

			ButtonTypeFlags pairTypes = ButtonTypeFlags_None;
			ButtonTypeFlags pairTypesHolds = ButtonTypeFlags_None;

			for (size_t pair = 0; pair < firstTargetOfPair.Flags.SyncPairCount; pair++)
			{
				const auto& pairTarget = workingChart->Targets[i + pair];
				const auto pairTypeFlag = ButtonTypeToButtonTypeFlags(pairTarget.Type);

				pairTypes |= pairTypeFlag;
				pairTypesHolds |= (pairTarget.Flags.IsHold) ? pairTypeFlag : 0;
			}

			assert(firstTargetOfPair.Flags.SyncPairCount >= 1);
			i += firstTargetOfPair.Flags.SyncPairCount;

			if (firstTargetOfPair.Tick > cursorTick)
				break;

			const auto pairButtonTime = timeline.TickToTime(firstTargetOfPair.Tick);
			checkAddHoldMaxEvent(holdEventStack, pairButtonTime);

			if (pairTypesHolds == ButtonTypeFlags_None)
			{
				if (holdEventStack.empty())
					continue;

				auto& lastEvent = holdEventStack.back();
				if (lastEvent.EventType != HoldEventType::Start && lastEvent.EventType != HoldEventType::Addition)
					continue;

				const bool newHoldConflicts = (lastEvent.CombinedButtonTypes & pairTypes);
				if (!newHoldConflicts)
					continue;
			}

			auto& newHoldEvent = holdEventStack.emplace_back();
			newHoldEvent.TargetPairIndex = static_cast<i32>(firstTargetOfPairIndex);
			newHoldEvent.StartTime = pairButtonTime;

			if (pairTypesHolds == ButtonTypeFlags_None)
			{
				newHoldEvent.EventType = HoldEventType::Cancel;
				newHoldEvent.CombinedButtonTypes = ButtonTypeFlags_None;
			}
			else if (holdEventStack.size() <= 1)
			{
				newHoldEvent.EventType = HoldEventType::Start;
				newHoldEvent.CombinedButtonTypes = pairTypesHolds;
			}
			else
			{
				const auto& lastEvent = holdEventStack[holdEventStack.size() - 2];

				const bool lastEventIsMergable = (lastEvent.EventType == HoldEventType::Start || lastEvent.EventType == HoldEventType::Addition);
				const bool newHoldConflicts = (lastEvent.CombinedButtonTypes & pairTypes);

				if (!lastEventIsMergable || newHoldConflicts)
				{
					newHoldEvent.EventType = HoldEventType::Start;
					newHoldEvent.CombinedButtonTypes = pairTypesHolds;
				}
				else
				{
					newHoldEvent.EventType = HoldEventType::Addition;
					newHoldEvent.CombinedButtonTypes = (lastEvent.CombinedButtonTypes | pairTypesHolds);
				}
			}
		}

		checkAddHoldMaxEvent(holdEventStack, cursorTime);

		if (!holdEventStack.empty())
		{
			const auto lastValidEvent = std::find_if(holdEventStack.rbegin(), holdEventStack.rend(), [](auto& e) { return (e.EventType == HoldEventType::Start || e.EventType == HoldEventType::Addition); });
			const auto& lastEvent = holdEventStack.back();

			// TODO: Calculate hold score and display max hold info (?)
			const auto syncHoldInfoMarkers = renderHelper->GetSyncHoldInfoMarkerData();
			TargetRenderHelper::SyncHoldInfoData syncInfoData = {};

			constexpr bool noAnimation = false;
			if (noAnimation)
			{
				syncInfoData.Time = syncHoldInfoMarkers.LoopStart;
				syncInfoData.TypeFlags = lastEvent.CombinedButtonTypes;
			}
			else if (lastValidEvent != holdEventStack.rend())
			{
				const bool wasAddition = (lastValidEvent->EventType == HoldEventType::Addition);

				const auto markerLoopStart = wasAddition ? syncHoldInfoMarkers.LoopStartAdd : syncHoldInfoMarkers.LoopStart;
				const auto markerLoopEnd = syncHoldInfoMarkers.LoopEnd;

				if (lastEvent.EventType == HoldEventType::MaxOut)
				{
					const auto timeSinceMaxOut = (cursorTime - lastValidEvent->StartTime - MaxTargetHoldDuration);
					syncInfoData.Time = (timeSinceMaxOut < syncHoldInfoMarkers.MaxLoopEnd) ?
						markerLoopStart + TimeSpan::FromSeconds(glm::mod((timeSinceMaxOut - markerLoopStart).TotalSeconds(), (markerLoopEnd - markerLoopStart).TotalSeconds())) :
						markerLoopEnd + (timeSinceMaxOut - syncHoldInfoMarkers.MaxLoopEnd);

					i32 heldButtonCount = 0;
					for (size_t i = 0; i < EnumCount<ButtonType>(); i++)
						heldButtonCount += static_cast<bool>(lastValidEvent->CombinedButtonTypes & ButtonTypeToButtonTypeFlags(static_cast<ButtonType>(i)));

					TargetRenderHelper::SyncHoldInfoData syncInfoMaxData = {};
					syncInfoMaxData.Time = timeSinceMaxOut;
					syncInfoMaxData.HoldScore = (heldButtonCount * (6000 / 4));
					renderHelper->DrawSyncHoldInfoMax(renderer, syncInfoMaxData);
				}
				else if (lastEvent.EventType == HoldEventType::Cancel)
				{
					const auto timeSinceCancel = (cursorTime - lastEvent.StartTime);
					syncInfoData.Time = markerLoopEnd + timeSinceCancel;
				}
				else
				{
					const auto timeSinceUpdate = (cursorTime - lastValidEvent->StartTime);
					syncInfoData.Time = (timeSinceUpdate > markerLoopStart) ?
						markerLoopStart + TimeSpan::FromSeconds(glm::mod((timeSinceUpdate - markerLoopStart).TotalSeconds(), (markerLoopEnd - markerLoopStart).TotalSeconds())) :
						timeSinceUpdate;
				}

				syncInfoData.TypeFlags = lastValidEvent->CombinedButtonTypes;
				syncInfoData.TypeAdded = wasAddition;
			}

			renderHelper->DrawSyncHoldInfo(renderer, syncInfoData);
			holdEventStack.clear();
		}
	}

	void TargetRenderWindow::RenderAllVisibleTargets()
	{
		AddVisibleTargetsToDrawBuffers();

		TargetRenderHelperExFlushFlags flags = TargetRenderHelperExFlushFlags_None;
		if (!GlobalUserData.TargetPreview.ShowButtons)
			flags |= TargetRenderHelperExFlushFlags_NoButtons;
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

			const auto endTick = buttonTick + GlobalUserData.TargetPreview.PostHitLingerDuration;
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
				targetData.NoHand = !inCursorBarRange;
				// NOTE: Transparent to make the background grid visible and make pre-, post- and cursor bar targets look more uniform
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
				targetData.Opacity = (target.IsSelected || !inCursorBarRange) ? 0.5f : 1.0f;

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
