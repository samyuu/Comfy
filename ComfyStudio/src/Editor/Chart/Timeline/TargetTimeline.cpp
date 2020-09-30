#include "TargetTimeline.h"
#include "Editor/Chart/ChartEditor.h"
#include "Editor/Chart/ChartCommands.h"
#include "Editor/Chart/SortedTempoMap.h"
#include "Editor/Chart/KeyBindings.h"
#include "Time/TimeSpan.h"
#include "ImGui/Extensions/PropertyEditor.h"
#include <FontIcons.h>

namespace Comfy::Studio::Editor
{
	TargetTimeline::TargetTimeline(ChartEditor& parent, Undo::UndoManager& undoManager) : chartEditor(parent), undoManager(undoManager)
	{
		scrollSpeed = 2.5f;
		scrollSpeedFast = 5.5f;
		autoScrollCursorOffsetPercentage = 0.35f;
		infoColumnWidth = 240.0f;

		buttonIcons = std::make_unique<TimelineButtonIcons>();
	}

	TimelineTick TargetTimeline::GridDivisionTick() const
	{
		return TimelineTick::FromBars(1) / activeBarGridDivision;
	}

	TimelineTick TargetTimeline::FloorTickToGrid(TimelineTick tick) const
	{
		const auto gridTicks = static_cast<f64>(GridDivisionTick().Ticks());
		return TimelineTick::FromTicks(static_cast<i32>(glm::floor(static_cast<f64>(tick.Ticks()) / gridTicks) * gridTicks));
	}

	TimelineTick TargetTimeline::RoundTickToGrid(TimelineTick tick) const
	{
		const auto gridTicks = static_cast<f64>(GridDivisionTick().Ticks());
		return TimelineTick::FromTicks(static_cast<i32>(glm::round(static_cast<f64>(tick.Ticks()) / gridTicks) * gridTicks));
	}

	// TODO: Rename all of these to contain their conversion types in the name and remove Get prefix
	f32 TargetTimeline::GetTimelinePosition(TimeSpan time) const
	{
		return TimelineBase::GetTimelinePosition(time);
	}

	f32 TargetTimeline::GetTimelinePosition(TimelineTick tick) const
	{
		return GetTimelinePosition(TickToTime(tick));
	}

	TimelineTick TargetTimeline::TimeToTick(TimeSpan time) const
	{
		return workingChart->TimelineMap.GetTickAt(time);
	}

	TimelineTick TargetTimeline::TimeToTickFixedTempo(TimeSpan time, Tempo tempo) const
	{
		return workingChart->TimelineMap.GetTickAtFixedTempo(time, tempo);
	}

	TimelineTick TargetTimeline::GetTimelineTick(f32 position) const
	{
		return TimeToTick(GetTimelineTime(position));
	}

	TimeSpan TargetTimeline::TickToTime(TimelineTick tick) const
	{
		return workingChart->TimelineMap.GetTimeAt(tick);
	}

	TimeSpan TargetTimeline::GetTimelineTime(f32 position) const
	{
		return TimelineBase::GetTimelineTime(position);
	}

	TimelineTick TargetTimeline::GetCursorMouseXTick(bool floorToGrid) const
	{
		const auto tickAtMousePosition = GetTimelineTick(ScreenToTimelinePosition(Gui::GetMousePos().x));
		const auto gridAdjusted = floorToGrid ? FloorTickToGrid(tickAtMousePosition) : tickAtMousePosition;

		// NOTE: There should never be a need to click before the start of the timeline
		const auto clamped = std::max(TimelineTick::Zero(), gridAdjusted);

		return clamped;
	}

	f32 TargetTimeline::GetButtonEdgeFadeOpacity(f32 screenX) const
	{
		constexpr auto fadeSpan = 35.0f;

#if 0 // NOTE: Left side fade
		if (screenX < 0.0f)
			return 0.0f;

		const auto lowerThreshold = fadeSpan;
		if (screenX < lowerThreshold)
			return ImLerp(0.0f, 1.0f, screenX / lowerThreshold);
#endif

#if 0 // NOTE: Right side fade
		if (screenX > baseWindow->Size.x)
			return 0.0f;

		const auto upperThreshold = baseWindow->Size.x - fadeSpan;
		if (screenX > upperThreshold)
			return ImLerp(0.0f, 1.0f, 1.0f - ((screenX - upperThreshold) / (baseWindow->Size.x - upperThreshold)));
#endif

		return 1.0f;
	}

	void TargetTimeline::OnUpdate()
	{
		UpdateOffsetChangeCursorTimeAdjustment();

		if (GetIsPlayback())
			UpdatePlaybackButtonSounds();
	}

	void TargetTimeline::UpdateOffsetChangeCursorTimeAdjustment()
	{
		lastFrameStartOffset = thisFrameStartOffset;
		thisFrameStartOffset = workingChart->StartOffset;

		// NOTE: Cancel out the cursor being moved by a change of offset because this just feels more intuitive to use
		//		 and always automatically applying the start offset to the playback time makes other calculations easier
		if (thisFrameStartOffset != lastFrameStartOffset)
		{
			if (GetIsPlayback())
				chartEditor.SetPlaybackTime(chartEditor.GetPlaybackTimeAsync() + (thisFrameStartOffset - lastFrameStartOffset));
			else
				chartEditor.SetPlaybackTime(TickToTime(pausedCursorTick));
		}
	}

	void TargetTimeline::UpdatePlaybackButtonSounds()
	{
		lastFrameButtonSoundCursorTime = thisFrameButtonSoundCursorTime;
		thisFrameButtonSoundCursorTime = GetCursorTime();

		const auto elapsedTime = (thisFrameButtonSoundCursorTime - lastFrameButtonSoundCursorTime);

		// NOTE: To prevent stacking of button sounds that happened "too long" ago, especially when the main window went inactive.
		//		 Should be long enough to never be reached as a normal frametime
		constexpr auto elapsedThresholdAtWhichPlayingSoundsMakesNoSense = TimeSpan::FromSeconds(1.0 / 5.0);
		if (elapsedTime >= elapsedThresholdAtWhichPlayingSoundsMakesNoSense)
			return;

		if (metronomeEnabled)
			metronome.UpdatePlaySounds(*workingChart, thisFrameButtonSoundCursorTime, lastFrameButtonSoundCursorTime, buttonSoundFutureOffset);

		// NOTE: Play back button sounds in the future with a negative offset to achieve sample perfect accuracy
		for (const auto& target : workingChart->Targets)
		{
			// NOTE: Stacked button sounds should be handled by the button sound controller automatically 
			//		 but doing an additional sync check here has the advantage of offloading audio engine work
			if (target.Flags.IsSync && target.Flags.IndexWithinSyncPair > 0 && !IsSlideButtonType(target.Type))
				continue;

			if (target.Flags.IsChain && !target.Flags.IsChainStart && !target.Flags.IsChainEnd)
				continue;

			const auto buttonTime = workingChart->TimelineMap.GetTimeAt(target.Tick);
			const auto offsetButtonTime = buttonTime - buttonSoundFutureOffset;

			if (offsetButtonTime >= lastFrameButtonSoundCursorTime && offsetButtonTime <= thisFrameButtonSoundCursorTime)
			{
				// NOTE: Don't wanna cause any audio cutoffs. If this happens the future threshold is either set too low for the current frame time
				//		 or playback was started on top of an existing target
				const auto startTime = std::min((thisFrameButtonSoundCursorTime - buttonTime), TimeSpan::Zero());
				const auto externalClock = buttonTime;

				if (target.Flags.IsChain)
				{
					const auto chainSoundSlot = (target.Type == ButtonType::SlideL) ? ChainSoundSlot::Left : ChainSoundSlot::Right;
					if (target.Flags.IsChainStart)
					{
						buttonSoundController.PlayChainSoundStart(chainSoundSlot, startTime, externalClock);
					}
					if (target.Flags.IsChainEnd)
					{
						if (constexpr bool success = true; success)
							buttonSoundController.PlayChainSoundSuccess(chainSoundSlot, startTime, externalClock);
						else
							buttonSoundController.PlayChainSoundFailure(chainSoundSlot, startTime, externalClock);

						// BUG: Sync chain slides with the left chain ending earlier than the right one. In practice this should rarely ever happen though
						buttonSoundController.FadeOutLastChainSound(chainSoundSlot);
					}
				}
				else if (IsSlideButtonType(target.Type))
				{
					buttonSoundController.PlaySlideSound(startTime, externalClock);
				}
				else
				{
					buttonSoundController.PlayButtonSound(startTime, externalClock);
				}
			}
		}
	}

	void TargetTimeline::OnPlaybackResumed()
	{
		Audio::AudioEngine::GetInstance().EnsureStreamRunning();
	}

	void TargetTimeline::OnPlaybackPaused()
	{
	}

	void TargetTimeline::OnPlaybackStopped()
	{
		CenterCursor();
	}

	void TargetTimeline::SetWorkingChart(Chart* chart)
	{
		workingChart = chart;
	}

	void TargetTimeline::OnSongLoaded()
	{
		Audio::AudioEngine::GetInstance().EnsureStreamRunning();

		if (const auto sampleProvider = Audio::AudioEngine::GetInstance().GetSharedSource(chartEditor.GetSongSource()); sampleProvider != nullptr)
			songWaveform.SetSource(sampleProvider);
		else
			songWaveform.Clear();

		songTextureCachedWaveform.InvalidateAll();
		waveformUpdatePending = true;
	}

	void TargetTimeline::OnDrawTimelineHeaderWidgets()
	{
		return;
	}

	void TargetTimeline::OnDrawTimelineInfoColumnHeader()
	{
		TimelineBase::OnDrawTimelineInfoColumnHeader();

		constexpr const char* settingsPopupName = "TimelineSettingsPopup::ChartTimeline";

		Gui::PushStyleVar(ImGuiStyleVar_ItemSpacing, vec2(0.0f, 0.0f));
		Gui::PushStyleVar(ImGuiStyleVar_FramePadding, vec2(8.0f, 8.0f));

		constexpr vec4 transparent = vec4(0.0f);
		Gui::PushStyleColor(ImGuiCol_Button, transparent);
		{
			const bool isFirstFrame = (cursorTime <= TimeSpan::Zero());
			const bool isLastFrame = (cursorTime >= workingChart->DurationOrDefault());
			const bool isPlayback = GetIsPlayback();

			constexpr float borderSize = 1.0f;
			Gui::SetCursorPosX(Gui::GetCursorPosX() + borderSize);

			// NOTE: First frame button
			{
				Gui::PushItemDisabledAndTextColorIf(isFirstFrame);
				if (Gui::Button(ICON_FA_FAST_BACKWARD))
				{
					SetCursorTime(TimeSpan::Zero());
					CenterCursor();
				}
				Gui::PopItemDisabledAndTextColorIf(isFirstFrame);
				Gui::SetWideItemTooltip("Go to first beat");
			}

			// NOTE: Previous frame button
			{
				Gui::SameLine();
				Gui::PushItemDisabledAndTextColorIf(isFirstFrame);
				if (Gui::Button(ICON_FA_BACKWARD))
				{
					AdvanceCursorByGridDivisionTick(-1);
				}
				Gui::PopItemDisabledAndTextColorIf(isFirstFrame);
				Gui::SetWideItemTooltip("Go to previous grid tick");
			}

			// NOTE: Playback toggle button
			{
				Gui::SameLine();
				if (Gui::Button(isPlayback ? ICON_FA_PAUSE : ICON_FA_PLAY))
					isPlayback ? PausePlayback() : ResumePlayback();

				Gui::SetWideItemTooltip(isPlayback ? "Pause playback" : "Resume playback");
			}

			// NOTE: Playback stop button
			{
				Gui::SameLine();
				Gui::PushItemDisabledAndTextColorIf(!isPlayback);
				if (Gui::Button(ICON_FA_STOP))
				{
					StopPlayback();
				}
				Gui::PopItemDisabledAndTextColorIf(!isPlayback);
				Gui::SetWideItemTooltip("Stop playback");
			}

			// NOTE: Next frame button
			{
				Gui::SameLine();
				Gui::PushItemDisabledAndTextColorIf(isLastFrame);
				if (Gui::Button(ICON_FA_FORWARD))
				{
					AdvanceCursorByGridDivisionTick(+1);
				}
				Gui::PopItemDisabledAndTextColorIf(isLastFrame);
				Gui::SetWideItemTooltip("Go to next grid tick");
			}

			// NOTE: Last frame button
			{
				Gui::SameLine();
				Gui::PushItemDisabledAndTextColorIf(isLastFrame);
				if (Gui::Button(ICON_FA_FAST_FORWARD))
				{
					SetCursorTime(workingChart->DurationOrDefault());
					CenterCursor();
				}
				Gui::PopItemDisabledAndTextColorIf(isLastFrame);
				Gui::SetWideItemTooltip("Go to last beat");
			}

			// TODO: What functionality should go here?
			{
				Gui::SameLine();
				Gui::Button(ICON_FA_ADJUST);
				Gui::SetWideItemTooltip("???");
			}

			// NOTE: Settings button
			{
				Gui::SameLine();
				if (Gui::Button(ICON_FA_COG))
					Gui::OpenPopup(settingsPopupName);
				Gui::SetWideItemTooltip("Timeline settings");
			}
		}
		Gui::PopStyleColor(1);
		Gui::PopStyleVar(2);

		// NOTE: Settings popup
		if (Gui::WideBeginPopup(settingsPopupName))
		{
			// TODO: Come up with a neat comfy layout
			Gui::Text("TODO:");
			Gui::EndPopup();
		}
	}

	void TargetTimeline::OnDrawTimelineInfoColumn()
	{
		TimelineBase::OnDrawTimelineInfoColumn();

		auto drawList = Gui::GetWindowDrawList();
		std::array<vec2, EnumCount<ButtonType>()> iconCenters;

		for (size_t row = 0; row < EnumCount<ButtonType>(); row++)
		{
			const auto y = row * rowHeight;
			const auto start = vec2(0.0f, y) + infoColumnRegion.GetTL();
			const auto end = vec2(infoColumnWidth, y + rowHeight) + infoColumnRegion.GetTL();

			const auto center = vec2(start + end) / 2.0f;
			iconCenters[row] = center;

			targetYPositions[row] = center.y;
			drawList->AddLine(vec2(start.x, end.y), end, Gui::GetColorU32(ImGuiCol_Border));
		}

		for (size_t row = 0; row < iconCenters.size(); row++)
		{
			const auto tempTarget = TimelineTarget(TimelineTick::Zero(), static_cast<ButtonType>(row));
			buttonIcons->DrawButtonIcon(drawList, tempTarget, iconCenters[row], iconScale);
		}
	}

	void TargetTimeline::OnDrawTimlineRows()
	{
		const auto timelineTL = timelineContentRegion.GetTL();
		const auto timelineWidth = vec2(timelineContentRegion.GetWidth(), 0.0f);

		for (size_t row = 0; row < EnumCount<ButtonType>(); row++)
		{
			const auto start = timelineTL + vec2(0.0f, row * rowHeight);
			const auto end = start + timelineWidth;

			baseDrawList->AddLine(start, end, GetColor(EditorColor_TimelineRowSeparator));
		}
	}

	void TargetTimeline::OnDrawTimlineDivisors()
	{
		const auto barColor = GetColor(EditorColor_Bar);
		const auto gridColor = GetColor(EditorColor_Grid);
		const auto gridAltColor = GetColor(EditorColor_GridAlt);

		const i32 songDurationTicks = TimeToTick(workingChart->DurationOrDefault()).Ticks();
		const i32 gridTickStep = GridDivisionTick().Ticks();

		const auto scrollX = GetScrollX();

		constexpr auto beatSpacingThreshold = 10.0f;
		constexpr auto barSpacingThreshold = 64.0f;

		auto lastBeatTimelineX = -beatSpacingThreshold;
		for (i32 tick = 0, divisions = 0; tick < songDurationTicks; tick += gridTickStep, divisions++)
		{
			const auto timelineX = GetTimelinePosition(TimelineTick(tick));
			if (const auto lastDrawnDistance = (timelineX - lastBeatTimelineX); lastDrawnDistance < beatSpacingThreshold)
				continue;
			lastBeatTimelineX = timelineX;

			const auto screenX = glm::round(timelineX - scrollX);
			const auto visiblity = GetTimelineVisibility(screenX);

			if (visiblity == TimelineVisibility::Left)
				continue;
			if (visiblity == TimelineVisibility::Right)
				break;

			const auto start = timelineContentRegion.GetTL() + vec2(screenX, -(timelineHeaderHeight * 0.35f));
			const auto end = timelineContentRegion.GetBL() + vec2(screenX, 0.0f);
			baseDrawList->AddLine(start, end, (divisions % 2 == 0 ? gridColor : gridAltColor));
		}

		auto lastBarTimelineX = -barSpacingThreshold;
		workingChart->TempoMap.ForEachBar([&](const TimelineTick barTick, const size_t barIndex)
		{
			const auto timelineX = GetTimelinePosition(barTick);
			if (const auto lastDrawnDistance = (timelineX - lastBarTimelineX); lastDrawnDistance < barSpacingThreshold)
				return false;
			lastBarTimelineX = timelineX;

			const auto screenX = glm::round(timelineX - scrollX);
			const auto visiblity = GetTimelineVisibility(screenX);

			if (visiblity == TimelineVisibility::Left)
				return false;
			if (visiblity == TimelineVisibility::Right)
				return true;

			char buffer[16];
			const auto start = timelineContentRegion.GetTL() + vec2(screenX, -(timelineHeaderHeight * 0.85f));
			const auto end = timelineContentRegion.GetBL() + vec2(screenX, 0.0f);
			baseDrawList->AddLine(start, end, barColor);
			baseDrawList->AddText(start + vec2(3.0f, -1.0f), barColor, buffer, buffer + sprintf_s(buffer, "%zu", barIndex));
			return false;
		});
	}

	void TargetTimeline::OnDrawTimlineBackground()
	{
		DrawOutOfBoundsBackground();
		DrawCheckUpdateWaveform();
		DrawTimelineTempoMap();
	}

	void TargetTimeline::OnDrawTimelineScrollBarRegion()
	{
		constexpr float timeDragTextOffset = 10.0f;
		constexpr float timeDragTextWidth = 60.0f + 26.0f;
		constexpr float gridDivisionButtonWidth = (72.0f * 2.0f);

		Gui::SetCursorPosX(Gui::GetCursorPosX() + timeDragTextOffset);

		Gui::PushStyleVar(ImGuiStyleVar_FramePadding, vec2(Gui::GetStyle().FramePadding.x, 0.0f));

		// NOTE: Time drag text
		{
			constexpr auto dragSpeed = 4.0f;
			auto cursorDragTicks = static_cast<f32>(GetCursorTick().Ticks());

			if (Gui::ComfyDragText("TimeDragText::TargetTimeline", cursorTime.FormatTime().data(), &cursorDragTicks, dragSpeed, 0.0f, 0.0f, timeDragTextWidth))
				SetCursorTick(std::max(TimelineTick::Zero(), RoundTickToGrid(TimelineTick::FromTicks(static_cast<i32>(cursorDragTicks)))));
		}

		{
			char buttonNameBuffer[32];
			sprintf_s(buttonNameBuffer, "Grid: 1 / %d", activeBarGridDivision);

			Gui::SameLine();
			if (Gui::Button(buttonNameBuffer, vec2(gridDivisionButtonWidth, timelineScrollbarSize.y)))
				SelectNextPresetGridDivision(+1);
			if (Gui::IsItemClicked(1))
				SelectNextPresetGridDivision(-1);
		}

		Gui::PopStyleVar(1);
	}

	void TargetTimeline::DrawOutOfBoundsBackground()
	{
		const auto outOfBoundsDimColor = GetColor(EditorColor_OutOfBoundsDim);
		const auto scrollX = GetScrollX();

		const auto preStart = timelineContentRegion.GetTL();
		const auto preEnd = timelineContentRegion.GetBL() + vec2(glm::round(GetTimelinePosition(TimelineTick(0)) - scrollX), 0.0f);
		if (preEnd.x - preStart.x > 0.0f)
			baseDrawList->AddRectFilled(preStart, preEnd, outOfBoundsDimColor);

		const auto postStart = timelineContentRegion.GetTL() + vec2(glm::round(GetTimelinePosition(workingChart->DurationOrDefault()) - scrollX), 0.0f);
		const auto postEnd = timelineContentRegion.GetBR();
		if (postEnd.x - postStart.x > 0.0f)
			baseDrawList->AddRectFilled(postStart, postEnd, outOfBoundsDimColor);
	}

	void TargetTimeline::DrawCheckUpdateWaveform()
	{
		if (zoomLevelChanged)
			waveformUpdatePending = true;

		if (waveformUpdatePending && waveformUpdateStopwatch.GetElapsed() >= waveformUpdateInterval)
		{
			const auto timePerPixel = GetTimelineTime(2.0f) - GetTimelineTime(1.0f);
			songWaveform.SetScale(timePerPixel);

			waveformUpdateStopwatch.Restart();
			waveformUpdatePending = false;
		}

		if (songWaveform.GetPixelCount() < 1)
			return;

#if COMFY_DEBUG && 0 // DEBUG: Quick validation test that the texture cached waveform is implemented correctly
		waveformDrawIndividualLines = Gui::IsKeyDown(Input::KeyCode_F1);
#endif

		if (waveformDrawIndividualLines)
			DrawWaveformIndividualVertexLines();
		else
			DrawTextureCachedWaveform();
	}

	void TargetTimeline::DrawTextureCachedWaveform()
	{
		// TODO: Come up with a neater solution for this (?)
		if (waveformUpdatePending)
			return;

		const auto timelineHeight = (static_cast<f32>(ButtonType::Count) * rowHeight);
		const auto scrollXStartOffset = GetScrollX() + GetTimelinePosition(workingChart->StartOffset);

		songTextureCachedWaveform.Draw(baseDrawList,
			timelineContentRegion.GetTL(),
			timelineContentRegion.GetTL() + vec2(timelineContentRegion.GetWidth(), timelineHeight),
			scrollXStartOffset);
	}

	void TargetTimeline::DrawWaveformIndividualVertexLines()
	{
		const auto scrollXStartOffset = GetScrollX() + GetTimelinePosition(workingChart->StartOffset);

		const auto leftMostVisiblePixel = static_cast<i64>(GetTimelinePosition(TimelineTick(0)));
		const auto rightMostVisiblePixel = leftMostVisiblePixel + static_cast<i64>(timelineContentRegion.GetWidth());
		const auto waveformPixelCount = static_cast<i64>(songWaveform.GetPixelCount());

		const auto timelineX = timelineContentRegion.GetTL().x;
		const auto timelineHeight = (static_cast<f32>(ButtonType::Count) * rowHeight);
		const auto timelineCenterY = timelineContentRegion.GetTL().y + (timelineHeight * 0.5f);

		const auto waveformColor = GetColor(EditorColor_Waveform);

		// NOTE: To try and mitigate "flashes" while resizing the timeline, optimally this should be equal to the average PCM of the last visible area
		const auto amplitudeDuringUpdate = (0.025f * timelineHeight);

		for (i64 screenPixel = leftMostVisiblePixel; screenPixel < waveformPixelCount && screenPixel < rightMostVisiblePixel; screenPixel++)
		{
			const auto timelinePixel = std::min(static_cast<i64>(glm::round(screenPixel + scrollXStartOffset)), static_cast<i64>(waveformPixelCount - 1));
			if (timelinePixel < 0)
				continue;

			constexpr u32 channelsToVisualize = 2;
			for (auto channel = 0; channel < channelsToVisualize; channel++)
			{
				const auto amplitude = waveformUpdatePending ? amplitudeDuringUpdate : (songWaveform.GetNormalizedPCMForPixel(timelinePixel, channel) * timelineHeight);
				if (amplitude < 1.0f)
					continue;

				const auto x = screenPixel + timelineX;
				const auto halfAmplitude = amplitude * 0.5f;

				const auto start = vec2(x, timelineCenterY - halfAmplitude);
				const auto end = vec2(x, timelineCenterY + halfAmplitude);

				baseDrawList->AddLine(start, end, waveformColor);
			}
		}
	}

	void TargetTimeline::DrawTimelineTempoMap()
	{
		const auto& tempoMap = workingChart->TempoMap;

		constexpr auto tempoChangePopupName = "##TempoChangePopup";
		float lastDrawnTimelineX = 0.0f;

		for (size_t i = 0; i < tempoMap.TempoChangeCount(); i++)
		{
			const auto& tempoChange = tempoMap.GetTempoChangeAt(i);

			const auto timelineX = GetTimelinePosition(tempoChange.Tick);
			const auto screenX = glm::round(timelineX - GetScrollX());
			const auto visiblity = GetTimelineVisibility(screenX);

			if (visiblity == TimelineVisibility::Left)
				continue;
			if (visiblity == TimelineVisibility::Right)
				break;

			const bool displaySignature = (i == 0 || tempoChange.Signature != tempoMap.GetTempoChangeAt(i - 1).Signature);
			const bool shortenText = (zoomLevel < 1.0f);

			char tempoBuffer[64];
			sprintf_s(tempoBuffer,
				displaySignature ? (shortenText ? "%.0f BPM %d/%d" : "%.2f BPM %d/%d") : (shortenText ? "%.0f BPM" : "%.2f BPM"),
				tempoChange.Tempo.BeatsPerMinute,
				tempoChange.Signature.Numerator, tempoChange.Signature.Denominator);

			const auto buttonPosition = tempoMapRegion.GetTL() + vec2(screenX + 1.0f, 0.0f);
			const auto buttonSize = vec2(Gui::CalcTextSize(tempoBuffer).x, tempoMapHeight);

			Gui::SetCursorScreenPos(buttonPosition);

			Gui::PushID(&tempoChange);
			Gui::InvisibleButton("##InvisibleTempoButton", buttonSize);
			Gui::PopID();

			if (Gui::IsItemHovered())
			{
				Gui::WideSetTooltip("Time: %s", TickToTime(tempoChange.Tick).FormatTime().data());

				baseDrawList->AddRect(buttonPosition, buttonPosition + buttonSize, Gui::GetColorU32(ImGuiCol_ChildBg));

				if (Gui::IsMouseClicked(0))
					SetCursorTick(tempoChange.Tick);

				if (Gui::IsMouseClicked(1))
				{
					Gui::OpenPopup(tempoChangePopupName);
					tempoPopupIndex = static_cast<int>(i);
				}
			}

			const auto tempoColor = GetColor(EditorColor_TempoChange);
			baseDrawList->AddLine(buttonPosition + vec2(-1.0f, -1.0f), buttonPosition + vec2(-1.0f, buttonSize.y - 1.0f), tempoColor);

			// NOTE: Just like with the bar / beat division culling this is far from perfect 
			//		 but at least crudely prevents any unreadable overlapping text until zoomed in close enough
			if (const auto lastDrawnDistance = (timelineX - lastDrawnTimelineX); lastDrawnDistance >= 0.0f)
				baseDrawList->AddText(Gui::GetFont(), tempoMapFontSize, (buttonPosition + tempoMapFontOffset), tempoColor, tempoBuffer);
			lastDrawnTimelineX = timelineX + buttonSize.x;
		}

		if (tempoPopupIndex >= 0)
			Gui::SetNextWindowSize(vec2(280.0f, 0.0f), ImGuiCond_Always);

		if (Gui::WideBeginPopup(tempoChangePopupName))
		{
			Gui::Text("Tempo Change:");

			GuiPropertyRAII::PropertyValueColumns columns;

			if (tempoPopupIndex >= 0)
			{
				const auto& tempoChange = tempoMap.GetTempoChangeAt(tempoPopupIndex);

				// TODO: Be able to move non-first tempo changes within the bounds of the previous and the next
				GuiProperty::PropertyLabelValueFunc("Time", [&]
				{
					Gui::TextDisabled("%s (Ticks: %d)", TickToTime(tempoChange.Tick).FormatTime().data(), tempoChange.Tick.Ticks());
					return false;
				});

				auto bpm = tempoChange.Tempo.BeatsPerMinute;
				if (GuiProperty::Input("Tempo##TempoChange", bpm, 1.0f, vec2(Tempo::MinBPM, Tempo::MaxBPM), "%.2f BPM"))
					undoManager.Execute<UpdateTempoChange>(*workingChart, TempoChange(tempoChange.Tick, std::clamp(bpm, Tempo::MinBPM, Tempo::MaxBPM), tempoChange.Signature));

				char signatureFormatBuffer[32];
				sprintf_s(signatureFormatBuffer, "%%d/%d", tempoChange.Signature.Denominator);

				i32 numerator = tempoChange.Signature.Numerator;
				if (GuiProperty::Input("Time Signature##TempoChange", numerator, 0.1f, ivec2(TimeSignature::MinValue, TimeSignature::MaxValue), signatureFormatBuffer))
					undoManager.Execute<UpdateTempoChange>(*workingChart, TempoChange(tempoChange.Tick, tempoChange.Tempo, TimeSignature(numerator, tempoChange.Signature.Denominator)));

				GuiProperty::PropertyLabelValueFunc("", [&]
				{
					const auto& style = Gui::GetStyle();
					Gui::PushStyleVar(ImGuiStyleVar_ItemSpacing, vec2(style.ItemInnerSpacing.x, style.ItemSpacing.y));
					const auto buttonWidth = (Gui::GetContentRegionAvailWidth() - style.ItemSpacing.x) / 2.0f;

					if (Gui::Button("x0.5", vec2(buttonWidth, 0.0f)))
						undoManager.Execute<UpdateTempoChange>(*workingChart, TempoChange(tempoChange.Tick, std::clamp(bpm * 0.5f, Tempo::MinBPM, Tempo::MaxBPM), tempoChange.Signature));
					Gui::SameLine();
					if (Gui::Button("x2.0", vec2(buttonWidth, 0.0f)))
						undoManager.Execute<UpdateTempoChange>(*workingChart, TempoChange(tempoChange.Tick, std::clamp(bpm * 2.0f, Tempo::MinBPM, Tempo::MaxBPM), tempoChange.Signature));
					Gui::PopStyleVar();

					return false;
				});

				GuiProperty::PropertyFuncValueFunc([&]
				{
					Gui::PushItemDisabledAndTextColorIf(tempoPopupIndex == 0);
					if (Gui::Button("Remove##TempoChange", vec2(Gui::GetContentRegionAvailWidth(), 0.0f)))
					{
						undoManager.ExecuteEndOfFrame<RemoveTempoChange>(*workingChart, tempoChange.Tick);
						Gui::CloseCurrentPopup();
					}
					Gui::PopItemDisabledAndTextColorIf(tempoPopupIndex == 0);
					return false;
				}, [&]
				{
					if (Gui::Button("Close##TempoChange", vec2(Gui::GetContentRegionAvailWidth(), 0.0f)))
						Gui::CloseCurrentPopup();
					return false;
				});
			}

			Gui::EndPopup();
		}
		else
		{
			tempoPopupIndex = -1;
		}
	}

	void TargetTimeline::DrawTimelineTargets()
	{
		auto windowDrawList = Gui::GetWindowDrawList();

		for (const auto& target : workingChart->Targets)
		{
			const auto buttonTime = TickToTime(target.Tick);
			const auto screenX = glm::round(GetTimelinePosition(buttonTime) - GetScrollX());

			const auto visiblity = GetTimelineVisibility(screenX);
			if (visiblity == TimelineVisibility::Left)
				continue;
			if (visiblity == TimelineVisibility::Right)
				break;

			const auto buttonIndex = static_cast<size_t>(target.Type);
			const auto center = vec2(screenX + timelineContentRegion.GetTL().x, targetYPositions[buttonIndex]);
			const auto scale = GetTimelineTargetScaleFactor(target, buttonTime) * iconScale;

			// TODO: Come up with a better visualization ... (?)
			const bool tooEarly = (target.Tick < TimelineTick::FromBars(1));
			constexpr auto tooEarlyOpacity = 0.35f;

			buttonIcons->DrawButtonIcon(windowDrawList, target, center, scale, tooEarly ? tooEarlyOpacity : GetButtonEdgeFadeOpacity(screenX));

			if (target.IsSelected)
				tempSelectedTargetPositionBuffer.push_back(center);
		}

		if (!tempSelectedTargetPositionBuffer.empty())
		{
			const f32 iconHitboxHalfSize = (iconHitboxSize / 2.0f);
			for (const auto& center : tempSelectedTargetPositionBuffer)
			{
				const auto tl = (center - iconHitboxHalfSize);
				const auto br = (center + iconHitboxHalfSize);

				windowDrawList->AddRectFilled(tl, br, GetColor(EditorColor_TimelineSelection));
				windowDrawList->AddRect(tl, br, GetColor(EditorColor_TimelineSelectionBorder));
			}
			tempSelectedTargetPositionBuffer.clear();
		}
	}

	f32 TargetTimeline::GetTimelineTargetScaleFactor(const TimelineTarget& target, TimeSpan buttonTime) const
	{
		if (GetIsPlayback())
		{
			const auto cursorTime = GetCursorTime();
			const auto timeUntilButton = (buttonTime - cursorTime);

			if (timeUntilButton <= TimeSpan::Zero() && timeUntilButton >= -buttonAnimationDuration)
			{
				const auto delta = static_cast<f32>(timeUntilButton.TotalSeconds() / -buttonAnimationDuration.TotalSeconds());
				return ImLerp(buttonAnimationScaleStart, buttonAnimationScaleEnd, delta);
			}
		}
		else
		{
			const auto& buttonAnimation = buttonAnimations[static_cast<size_t>(target.Type)];

			if (target.Tick == buttonAnimation.Tick &&
				buttonAnimation.ElapsedTime >= buttonAnimationStartTime &&
				buttonAnimation.ElapsedTime <= buttonAnimationDuration)
			{
				const auto delta = static_cast<f32>(buttonAnimation.ElapsedTime.TotalSeconds() / buttonAnimationDuration.TotalSeconds());
				return ImLerp(buttonAnimationScaleStart, buttonAnimationScaleEnd, delta);
			}
		}

		return 1.0f;
	}

	void TargetTimeline::DrawTimelineCursor()
	{
		if (GetIsPlayback())
		{
			const auto prePlaybackX = glm::round(GetTimelinePosition(chartEditor.GetPlaybackTimeOnPlaybackStart()) - GetScrollX());

			const auto start = timelineHeaderRegion.GetTL() + vec2(prePlaybackX, 0.0f);
			const auto end = timelineContentRegion.GetBL() + vec2(prePlaybackX, 0.0f);

			baseDrawList->AddLine(start, end, GetColor(EditorColor_CursorInner));
		}

		// TODO: Different cursor colors based on grid division (?)
		TimelineBase::DrawTimelineCursor();
	}

	void TargetTimeline::DrawRangeSelection()
	{
		if (!rangeSelection.IsActive)
			return;

		const auto startScreenX = glm::round(GetTimelinePosition(rangeSelection.StartTick) - GetScrollX()) + (!rangeSelection.HasEnd ? -1.0f : 0.0f);
		const auto endScreenX = glm::round(GetTimelinePosition(rangeSelection.EndTick) - GetScrollX()) + (!rangeSelection.HasEnd ? +2.0f : 1.0f);

		const auto start = vec2(timelineContentRegion.GetTL().x + startScreenX, timelineContentRegion.GetTL().y);
		const auto end = vec2(timelineContentRegion.GetTL().x + endScreenX, timelineContentRegion.GetBR().y);

		baseDrawList->AddRectFilled(start, end, GetColor(EditorColor_TimelineSelection, 0.3f));
		baseDrawList->AddRect(start, end, GetColor(EditorColor_TimelineSelectionBorder));
	}

	void TargetTimeline::DrawBoxSelection()
	{
		if (!boxSelection.IsActive || !boxSelection.IsSufficientlyLarge)
			return;

		const auto startScreenX = glm::round(GetTimelinePosition(boxSelection.StartTick) - GetScrollX());
		const auto endScreenX = glm::round(GetTimelinePosition(boxSelection.EndTick) - GetScrollX());

		const auto minY = timelineContentRegion.GetTL().y;
		const auto maxY = timelineContentRegion.GetBR().y;

		const auto start = vec2(timelineContentRegion.GetTL().x + startScreenX, glm::clamp(boxSelection.StartMouse.y, minY, maxY));
		const auto end = vec2(timelineContentRegion.GetTL().x + endScreenX, glm::clamp(boxSelection.EndMouse.y, minY, maxY));

		baseDrawList->AddRectFilled(start, end, GetColor(EditorColor_TimelineSelection));
		baseDrawList->AddRect(start, end, GetColor(EditorColor_TimelineSelectionBorder));

		// TODO: Move into common selection logic source file (?)
		if (boxSelection.Action != BoxSelectionData::ActionType::Clean)
		{
			constexpr f32 circleRadius = 6.0f;
			constexpr f32 symbolSize = 2.0f;

			const auto symbolPos = start;
			const auto symbolColor = Gui::GetColorU32(ImGuiCol_Text);

			baseDrawList->AddCircleFilled(symbolPos, circleRadius, Gui::GetColorU32(ImGuiCol_ChildBg));
			baseDrawList->AddCircle(symbolPos, circleRadius, GetColor(EditorColor_TimelineSelectionBorder));

			if (boxSelection.Action == BoxSelectionData::ActionType::Add || boxSelection.Action == BoxSelectionData::ActionType::Remove)
				baseDrawList->AddLine(symbolPos - vec2(symbolSize, 0.0f), start + vec2(symbolSize + 1.0f, 0.0f), symbolColor, 1.0f);

			if (boxSelection.Action == BoxSelectionData::ActionType::Add)
				baseDrawList->AddLine(symbolPos - vec2(0.0f, symbolSize), start + vec2(0.0f, symbolSize + 1.0f), symbolColor, 1.0f);
		}
	}

	void TargetTimeline::OnUpdateInput()
	{
		const auto deltaTime = TimeSpan::FromSeconds(Gui::GetIO().DeltaTime);
		for (auto& animationData : buttonAnimations)
			animationData.ElapsedTime += deltaTime;

		UpdateKeyboardCtrlInput();
		UpdateCursorKeyboardInput();

		UpdateInputSelectionDragging();
		UpdateInputCursorClick();
		UpdateInputCursorScrubbing();
		UpdateInputTargetPlacement();

		UpdateInputContextMenu();
		UpdateInputBoxSelection();
	}

	void TargetTimeline::OnDrawTimelineContents()
	{
		DrawRangeSelection();
		DrawTimelineTargets();
		DrawBoxSelection();
	}

	void TargetTimeline::UpdateKeyboardCtrlInput()
	{
		if (!Gui::IsWindowFocused())
			return;

		// TODO: Move into some general function to be called by all window owning editor components (?)
		if (Gui::GetIO().KeyCtrl)
		{
			if (Gui::IsKeyPressed(KeyBindings::Undo, true))
				undoManager.Undo();

			if (Gui::IsKeyPressed(KeyBindings::Redo, true))
				undoManager.Redo();
		}

		if (Gui::GetIO().KeyCtrl)
		{
			if (Gui::IsKeyPressed(KeyBindings::Cut, false))
				ClipboardCutSelection();

			if (Gui::IsKeyPressed(KeyBindings::Copy, false))
				ClipboardCopySelection();

			if (Gui::IsKeyPressed(KeyBindings::Paste, false))
				ClipboardPasteSelection();
		}
	}

	void TargetTimeline::UpdateCursorKeyboardInput()
	{
		if (Gui::IsWindowHovered() && Gui::IsMouseClicked(1))
			rangeSelection = {};

		if (!Gui::IsWindowFocused())
			return;

		constexpr bool allowRepeat = true;
		const bool useBeatStep = Gui::GetIO().KeyShift;

		if (Gui::IsKeyPressed(KeyBindings::MoveCursorLeft, allowRepeat))
			AdvanceCursorByGridDivisionTick(-1, useBeatStep);
		if (Gui::IsKeyPressed(KeyBindings::MoveCursorRight, allowRepeat))
			AdvanceCursorByGridDivisionTick(+1, useBeatStep);

		if (Gui::IsKeyPressed(KeyBindings::IncreaseGridPrecision, allowRepeat))
			SelectNextPresetGridDivision(-1);
		if (Gui::IsKeyPressed(KeyBindings::DecreaseGridPrecision, allowRepeat))
			SelectNextPresetGridDivision(+1);

		if (Gui::IsKeyPressed(KeyBindings::RangeSelection, false))
		{
			if (!rangeSelection.IsActive || rangeSelection.HasEnd)
			{
				rangeSelection.StartTick = rangeSelection.EndTick = GetCursorTick();
				rangeSelection.HasEnd = false;
				rangeSelection.IsActive = true;
			}
			else
			{
				rangeSelection.EndTick = GetCursorTick();
				rangeSelection.HasEnd = true;

				if (rangeSelection.EndTick == rangeSelection.StartTick)
					rangeSelection = {};
			}
		}
	}

	void TargetTimeline::UpdateInputSelectionDragging()
	{
		if (!selectionDrag.IsDragging)
		{
			if (boxSelection.IsActive || isCursorScrubbing)
				return;

			if (!Gui::IsWindowHovered(ImGuiHoveredFlags_AllowWhenBlockedByPopup) || !timelineContentRegion.Contains(Gui::GetMousePos()))
				return;
		}
		else if (Gui::IsMouseReleased(0))
		{
			selectionDrag = {};
		}

		selectionDrag.IsHovering = false;
		if (!selectionDrag.IsDragging)
		{
			// TODO: Min max visible tick range check optimization 
			const f32 iconHitboxHalfSize = (iconHitboxSize / 2.0f);
			for (auto& target : workingChart->Targets)
			{
				if (!target.IsSelected)
					continue;

				const auto center = vec2(GetTimelinePosition(target.Tick) - GetScrollX() + timelineContentRegion.GetTL().x, targetYPositions[static_cast<size_t>(target.Type)]);
				const auto hitbox = ImRect(center - iconHitboxHalfSize, center + iconHitboxHalfSize);

				if (!hitbox.Contains(Gui::GetMousePos()))
					continue;

				selectionDrag.IsHovering = true;
				if (Gui::IsMouseClicked(0))
				{
					selectionDrag.IsDragging = true;
					selectionDrag.TickOnPress = GetCursorMouseXTick(false);
					selectionDrag.TicksMovedSoFar = {};
				}
			}
		}

		if (selectionDrag.IsDragging || selectionDrag.IsHovering)
			Gui::SetMouseCursor(ImGuiMouseCursor_ResizeEW);

		selectionDrag.LastFrameMouseTick = selectionDrag.ThisFrameMouseTick;
		selectionDrag.ThisFrameMouseTick = GetCursorMouseXTick(false);

		if (selectionDrag.IsDragging)
		{
			Gui::SetActiveID(Gui::GetID(&selectionDrag), Gui::GetCurrentWindow());
			selectionDrag.TicksMovedSoFar += (selectionDrag.ThisFrameMouseTick - selectionDrag.LastFrameMouseTick);
		}

		const auto dragTickIncrement = FloorTickToGrid(selectionDrag.TicksMovedSoFar);

		// BUG: Need extra checks for moving chain fragments or at least update flags after the fact (allow moving partial sync pairs and always update flags (?))

		// NOTE: Prevent moving through any existing targets to avoid having to resort the target list
		if (selectionDrag.IsDragging && dragTickIncrement != TimelineTick::Zero() && !CheckIsAnySyncPairPartiallySelected() && CheckIsSelectionNotBlocked(dragTickIncrement))
		{
			selectionDrag.TicksMovedSoFar -= dragTickIncrement;
			const auto cursorTick = GetCursorTick();

			std::vector<i32> targetMoveIndices;
			targetMoveIndices.reserve(CountSelectedTargets());

			for (i32 i = 0; i < static_cast<i32>(workingChart->Targets.size()); i++)
			{
				const auto& target = workingChart->Targets[i];
				if (!target.IsSelected)
					continue;

				targetMoveIndices.push_back(i);
				if (const auto movedTick = (target.Tick + dragTickIncrement); movedTick == cursorTick)
					PlaySingleTargetButtonSoundAndAnimation(target, movedTick);
			}

			undoManager.Execute<MoveTargetList>(*workingChart, std::move(targetMoveIndices), dragTickIncrement, selectionDrag.TickOnPress);
		}
	}

	bool TargetTimeline::CheckIsAnySyncPairPartiallySelected() const
	{
		for (size_t i = 0; i < workingChart->Targets.size(); i++)
		{
			const auto& target = workingChart->Targets[i];
			if (target.IsSelected && target.Flags.IsSync)
			{
				bool entireSyncPairSelected = true;
				for (size_t s = 0; s < target.Flags.SyncPairCount; s++)
					entireSyncPairSelected &= workingChart->Targets[i - target.Flags.IndexWithinSyncPair + s].IsSelected;

				if (!entireSyncPairSelected)
					return true;
			}
		}

		return false;
	}

	bool TargetTimeline::CheckIsSelectionNotBlocked(TimelineTick increment) const
	{
		if (increment > TimelineTick::Zero())
		{
			for (i32 i = 0; i < static_cast<i32>(workingChart->Targets.size()); i++)
			{
				auto& target = workingChart->Targets[i];
				auto* nextTarget = IndexOrNull(i + 1, workingChart->Targets);

				if (target.IsSelected && nextTarget != nullptr && !nextTarget->IsSelected)
				{
					if (target.Tick + increment >= nextTarget->Tick)
						return false;
				}
			}
		}
		else
		{
			for (i32 i = static_cast<i32>(workingChart->Targets.size()) - 1; i >= 0; i--)
			{
				auto& target = workingChart->Targets[i];
				auto* prevTarget = IndexOrNull(i - 1, workingChart->Targets);

				if (target.Tick + increment < TimelineTick::Zero())
					return false;

				if (target.IsSelected && prevTarget != nullptr && !prevTarget->IsSelected)
				{
					if (target.Tick + increment <= prevTarget->Tick)
						return false;
				}
			}
		}

		return true;
	}

	void TargetTimeline::UpdateInputCursorClick()
	{
		if (!Gui::IsWindowHovered(ImGuiHoveredFlags_AllowWhenBlockedByPopup) || !timelineContentRegion.Contains(Gui::GetMousePos()))
			return;

		if (selectionDrag.IsDragging || selectionDrag.IsHovering)
			return;

		if (Gui::IsMouseClicked(0) && !Gui::GetIO().KeyShift)
		{
			const auto newMouseTick = GetCursorMouseXTick();

			SetCursorTick(newMouseTick);
			PlayCursorButtonSoundsAndAnimation(newMouseTick);
		}
	}

	void TargetTimeline::UpdateInputCursorScrubbing()
	{
		if (Gui::IsMouseReleased(0) || !Gui::IsWindowFocused())
			isCursorScrubbing = false;

		if (Gui::IsWindowHovered(ImGuiHoveredFlags_AllowWhenBlockedByPopup) && timelineHeaderRegion.Contains(Gui::GetMousePos()))
		{
			if (Gui::IsMouseClicked(0))
				isCursorScrubbing = true;
		}

		if (isCursorScrubbing)
		{
			// NOTE: Disallow playback scrubbing to prevent unreasonable button sound stacking
			const bool isPlayback = GetIsPlayback();
			if (isPlayback)
				isCursorScrubbing = false;

			const bool floorToGrid = Gui::GetIO().KeyAlt;
			const auto oldCursorTick = GetCursorTick();
			const auto newMouseTick = GetCursorMouseXTick(floorToGrid);

			SetCursorTick(newMouseTick);
			if (!isPlayback && (newMouseTick != oldCursorTick))
				PlayCursorButtonSoundsAndAnimation(newMouseTick);
		}
	}

	void TargetTimeline::UpdateInputTargetPlacement()
	{
		if (!Gui::IsWindowFocused())
			return;

		// NOTE: Mouse X buttons, increase / decrease grid division
		if (Gui::IsMouseClicked(3)) SelectNextPresetGridDivision(-1);
		if (Gui::IsMouseClicked(4)) SelectNextPresetGridDivision(+1);

		if (Gui::GetIO().KeyCtrl)
			return;

		for (const auto[buttonType, keyCode] : KeyBindings::TargetPlacements)
		{
			if (Gui::IsKeyPressed(keyCode, false))
			{
				if (Gui::GetIO().KeyShift && rangeSelection.IsActive && rangeSelection.HasEnd)
					FillInRangeSelectionTargets(buttonType);
				else
					PlaceOrRemoveTarget(RoundTickToGrid(GetCursorTick()), buttonType);
			}
		}

		if (Gui::IsKeyPressed(KeyBindings::DeleteSelection, false))
			RemoveAllSelectedTargets();
	}

	void TargetTimeline::UpdateInputContextMenu()
	{
		constexpr const char* contextMenuID = "TargetTimelineContextMenu";

		if (Gui::IsMouseReleased(1) && Gui::IsWindowHovered(ImGuiHoveredFlags_AllowWhenBlockedByPopup) && timelineContentRegion.Contains(Gui::GetMousePos()))
		{
			if (!Gui::IsAnyItemHovered() && !boxSelection.IsSufficientlyLarge && !selectionDrag.IsDragging && !isCursorScrubbing)
				Gui::OpenPopup(contextMenuID);
		}

		if (Gui::BeginPopup(contextMenuID, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoMove))
		{
			const auto selectionCount = CountSelectedTargets();

			if (Gui::BeginMenu("Grid Division"))
			{
				for (const auto barDivision : presetBarGridDivisions)
				{
					char nameBuffer[32];
					sprintf_s(nameBuffer, "Set 1 / %d", barDivision);

					bool alreadySelected = (activeBarGridDivision == barDivision);;
					if (Gui::MenuItem(nameBuffer, nullptr, &alreadySelected, !alreadySelected))
						activeBarGridDivision = barDivision;
				}

				Gui::EndMenu();
			}

			// TODO: Maybe move metronome settings to the timeline info column header (?)
			if (Gui::BeginMenu("Metronome"))
			{
				Gui::Checkbox("Enabled", &metronomeEnabled);
				Gui::PushItemDisabledAndTextColorIf(!metronomeEnabled);
				if (auto v = metronome.GetVolume(); Gui::SliderFloat("Volume", &v, 0.0f, 1.5f))
					metronome.SetVolume(v);
				Gui::PopItemDisabledAndTextColorIf(!metronomeEnabled);
				Gui::EndMenu();
			}

			if (Gui::MenuItem("Set Song End", "", nullptr, true))
				undoManager.Execute<ChangeSongDuration>(*workingChart, GetCursorTime());

			Gui::Separator();

			if (Gui::MenuItem("Cut", "Ctrl + X", nullptr, (selectionCount > 0)))
				ClipboardCutSelection();

			if (Gui::MenuItem("Copy", "Ctrl + C", nullptr, (selectionCount > 0)))
				ClipboardCopySelection();

			if (Gui::MenuItem("Paste", "Ctrl + V", nullptr, true))
				ClipboardPasteSelection();

			Gui::Separator();

#if 0 // TODO:
			if (Gui::MenuItem("Insert Tempo Change", "Ctrl + ?", nullptr, false)) {}
			if (Gui::MenuItem("Remove Tempo Change", "Ctrl + ?", nullptr, false)) {}

			Gui::Separator();
#endif

			// TODO: Various options to turn a straight target row into patterns (?)
			//		 OOOOO -> OXOXO, "compress" down to single type, etc.

			if (Gui::MenuItem("Select All", "", nullptr, (workingChart->Targets.size() > 0)))
				SelectAllTargets();

			if (Gui::MenuItem("Deselect", "", nullptr, (selectionCount > 0)))
				DeselectAllTargets();

			Gui::Separator();

			if (Gui::MenuItem("Remove Targets", "Del", nullptr, (selectionCount > 0)))
				RemoveAllSelectedTargets(selectionCount);

			Gui::EndPopup();
		}
	}

	void TargetTimeline::UpdateInputBoxSelection()
	{
		constexpr int boxSelectionButton = 1;

		if (Gui::IsWindowHovered(ImGuiHoveredFlags_AllowWhenBlockedByPopup) && timelineContentRegion.Contains(Gui::GetMousePos()))
		{
			if (Gui::IsMouseClicked(boxSelectionButton) && !boxSelection.IsActive && !selectionDrag.IsDragging && !selectionDrag.IsHovering && !isCursorScrubbing)
			{
				boxSelection.StartMouse = Gui::GetMousePos();
				boxSelection.StartTick = GetTimelineTick(ScreenToTimelinePosition(boxSelection.StartMouse.x));
				boxSelection.IsActive = true;
			}
		}

		if (boxSelection.IsActive && Gui::IsMouseDown(boxSelectionButton))
		{
			boxSelection.EndMouse = Gui::GetMousePos();
			boxSelection.EndTick = GetTimelineTick(ScreenToTimelinePosition(boxSelection.EndMouse.x));

			const auto& io = Gui::GetIO();
			boxSelection.Action = io.KeyShift ? BoxSelectionData::ActionType::Add : io.KeyAlt ? BoxSelectionData::ActionType::Remove : BoxSelectionData::ActionType::Clean;

			constexpr f32 sizeThreshold = 4.0f;
			const f32 selectionWidth = glm::abs(GetTimelinePosition(boxSelection.StartTick) - GetTimelinePosition(boxSelection.EndTick));
			const f32 selectionHeight = glm::abs(boxSelection.StartMouse.y - boxSelection.EndMouse.y);

			boxSelection.IsSufficientlyLarge = (selectionWidth >= sizeThreshold) || (selectionHeight >= sizeThreshold);
			if (boxSelection.IsSufficientlyLarge)
				Gui::SetActiveID(Gui::GetID(&boxSelection), Gui::GetCurrentWindow());
		}

		if (Gui::IsMouseReleased(boxSelectionButton) && boxSelection.IsActive)
		{
			if (boxSelection.IsSufficientlyLarge)
			{
				const f32 minY = std::min(boxSelection.StartMouse.y, boxSelection.EndMouse.y);
				const f32 maxY = std::max(boxSelection.StartMouse.y, boxSelection.EndMouse.y);

				const auto minTick = std::min(boxSelection.StartTick, boxSelection.EndTick);
				const auto maxTick = std::max(boxSelection.StartTick, boxSelection.EndTick);

				auto isTargetInSelectionRange = [&](const auto& target)
				{
					const f32 y = targetYPositions[static_cast<size_t>(target.Type)];
					return (y >= minY && y <= maxY) && (target.Tick >= minTick && target.Tick <= maxTick);
				};

				switch (boxSelection.Action)
				{
				case BoxSelectionData::ActionType::Clean:
					for (auto& target : workingChart->Targets)
						target.IsSelected = isTargetInSelectionRange(target);
					break;

				case BoxSelectionData::ActionType::Add:
					for (auto& target : workingChart->Targets)
					{
						if (isTargetInSelectionRange(target))
							target.IsSelected = true;
					}
					break;

				case BoxSelectionData::ActionType::Remove:
					for (auto& target : workingChart->Targets)
					{
						if (isTargetInSelectionRange(target))
							target.IsSelected = false;
					}
					break;

				default:
					assert(false);
				}
			}

			boxSelection.IsActive = false;
			boxSelection.IsSufficientlyLarge = false;
		}
	}

	size_t TargetTimeline::CountSelectedTargets() const
	{
		return std::count_if(
			workingChart->Targets.begin(),
			workingChart->Targets.end(),
			[](const auto& t) { return t.IsSelected; });
	}

	void TargetTimeline::SelectAllTargets()
	{
		std::for_each(workingChart->Targets.begin(), workingChart->Targets.end(), [](auto& t) { t.IsSelected = true; });
	}

	void TargetTimeline::DeselectAllTargets()
	{
		std::for_each(workingChart->Targets.begin(), workingChart->Targets.end(), [](auto& t) { t.IsSelected = false; });
	}

	void TargetTimeline::ClipboardCutSelection()
	{
		const auto selectionCount = CountSelectedTargets();
		if (selectionCount < 1)
			return;

		std::vector<TimelineTarget> selectedTargets;
		selectedTargets.reserve(selectionCount);
		std::copy_if(workingChart->Targets.begin(), workingChart->Targets.end(), std::back_inserter(selectedTargets), [&](auto& t) { return t.IsSelected; });

		clipboardHelper.TimelineCopySelectedTargets(selectedTargets);
		undoManager.Execute<CutTargetList>(*workingChart, std::move(selectedTargets));
	}

	void TargetTimeline::ClipboardCopySelection()
	{
		const auto selectionCount = CountSelectedTargets();
		if (selectionCount < 1)
			return;

		std::vector<TimelineTarget> selectedTargets;
		selectedTargets.reserve(selectionCount);
		std::copy_if(workingChart->Targets.begin(), workingChart->Targets.end(), std::back_inserter(selectedTargets), [&](auto& t) { return t.IsSelected; });

		clipboardHelper.TimelineCopySelectedTargets(selectedTargets);
	}

	void TargetTimeline::ClipboardPasteSelection()
	{
		auto optionalPasteTargets = clipboardHelper.TimelineTryGetPasteTargets();
		if (!optionalPasteTargets.has_value() || optionalPasteTargets->empty())
			return;

		auto pasteTargets = std::move(optionalPasteTargets.value());

		const auto baseTick = FloorTickToGrid(GetCursorTick()) - pasteTargets.front().Tick;
		for (auto& target : pasteTargets)
			target.Tick += baseTick;

		auto targetAlreadyExists = [&](const auto& t) { return (workingChart->Targets.FindIndex(t.Tick, t.Type) > -1); };
		pasteTargets.erase(std::remove_if(pasteTargets.begin(), pasteTargets.end(), targetAlreadyExists), pasteTargets.end());

		if (!pasteTargets.empty())
		{
			PlaySingleTargetButtonSoundAndAnimation(pasteTargets.front());
			undoManager.Execute<PasteTargetList>(*workingChart, std::move(pasteTargets));
		}
	}

	void TargetTimeline::FillInRangeSelectionTargets(ButtonType type)
	{
		const auto startTick = RoundTickToGrid(std::min(rangeSelection.StartTick, rangeSelection.EndTick));
		const auto endTick = RoundTickToGrid(std::max(rangeSelection.StartTick, rangeSelection.EndTick));

		// TODO: Come up with better chain placement controls, maybe shift + direction to place chain start, move cursor then press again to "confirm" end placement (?)
		const bool placeChain = IsSlideButtonType(type);

		const auto gridDivisionTick = placeChain ? (TimelineTick::FromBars(1) / 32) : GridDivisionTick();
		const auto targetCount = ((endTick - startTick).Ticks() / gridDivisionTick.Ticks()) + 1;

		std::vector<TimelineTarget> targets;
		targets.reserve(targetCount);

		for (i32 i = 0; i < targetCount; i++)
		{
			const auto tick = TimelineTick(startTick.Ticks() + (i * gridDivisionTick.Ticks()));
			if (workingChart->Targets.FindIndex(tick, type) <= -1)
			{
				auto& target = targets.emplace_back(tick, type);

				if (placeChain)
					target.Flags.IsChain = true;
			}
		}

		if (!targets.empty())
		{
			PlaySingleTargetButtonSoundAndAnimation(targets.front());
			undoManager.Execute<AddTargetList>(*workingChart, std::move(targets));
		}
	}

	void TargetTimeline::PlaceOrRemoveTarget(TimelineTick tick, ButtonType type)
	{
		const auto existingTargetIndex = workingChart->Targets.FindIndex(tick, type);
		const bool targetExists = (existingTargetIndex > -1);

		// NOTE: Double hit sound if a target gets placed in front of the cursor position.
		//		 Keeping it this way could make it easier to notice when real time targets are not placed accurately to the beat (?)
		if (!buttonSoundOnSuccessfulPlacementOnly)
			PlayTargetButtonTypeSound(type);

		if (targetExists)
		{
			if (!GetIsPlayback())
			{
				if (buttonSoundOnSuccessfulPlacementOnly)
					PlayTargetButtonTypeSound(type);

				const auto existingTarget = workingChart->Targets[existingTargetIndex];
				undoManager.Execute<RemoveTarget>(*workingChart, existingTarget);
			}
		}
		else
		{
			const bool isPartOfExistingSyncPair = (workingChart->Targets.FindIndex(tick) > -1);
			if (buttonSoundOnSuccessfulPlacementOnly && (!GetIsPlayback() || !isPartOfExistingSyncPair))
				PlayTargetButtonTypeSound(type);

			undoManager.Execute<AddTarget>(*workingChart, TimelineTarget(tick, type));
		}

		const auto buttonIndex = static_cast<size_t>(type);
		buttonAnimations[buttonIndex].Tick = tick;
		buttonAnimations[buttonIndex].ElapsedTime = TimeSpan::Zero();
	}

	void TargetTimeline::RemoveAllSelectedTargets(std::optional<size_t> preCalculatedSelectionCount)
	{
		const auto selectionCount = [&] { return (preCalculatedSelectionCount.has_value()) ? preCalculatedSelectionCount.value() : CountSelectedTargets(); }();
		if (selectionCount < 1)
			return;

		std::vector<TimelineTarget> targetsToRemove;
		targetsToRemove.reserve(selectionCount);

		std::copy_if(
			workingChart->Targets.begin(),
			workingChart->Targets.end(),
			std::back_inserter(targetsToRemove),
			[](const auto& t) { return t.IsSelected; });

		assert(targetsToRemove.size() == selectionCount);
		undoManager.Execute<RemoveTargetList>(*workingChart, std::move(targetsToRemove));
	}

	void TargetTimeline::PlayTargetButtonTypeSound(ButtonType type)
	{
		if (IsSlideButtonType(type))
			buttonSoundController.PlaySlideSound();
		else
			buttonSoundController.PlayButtonSound();
	}

	void TargetTimeline::PlayCursorButtonSoundsAndAnimation(TimelineTick cursorTick)
	{
		for (const auto& target : workingChart->Targets)
		{
			if (target.Tick == cursorTick)
				PlaySingleTargetButtonSoundAndAnimation(target);
		}
	}

	void TargetTimeline::PlaySingleTargetButtonSoundAndAnimation(const TimelineTarget& target, std::optional<TimelineTick> buttonTick)
	{
		// NOTE: During playback the sound will be handled automatically already
		if (!GetIsPlayback())
			PlayTargetButtonTypeSound(target.Type);

		const auto buttonIndex = static_cast<size_t>(target.Type);
		buttonAnimations[buttonIndex].Tick = buttonTick.value_or(target.Tick);
		buttonAnimations[buttonIndex].ElapsedTime = TimeSpan::Zero();
	}

	TimeSpan TargetTimeline::GetCursorTime() const
	{
		if (chartEditor.GetIsPlayback())
			return chartEditor.GetPlaybackTimeAsync();
		else
			return TickToTime(pausedCursorTick);
	}

	void TargetTimeline::SetCursorTime(const TimeSpan newTime)
	{
		pausedCursorTick = TimeToTick(newTime);
		chartEditor.SetPlaybackTime(newTime);

		PlaybackStateChangeSyncButtonSoundCursorTime(newTime);
	}

	TimelineTick TargetTimeline::GetCursorTick() const
	{
		if (chartEditor.GetIsPlayback())
			return TimeToTick(chartEditor.GetPlaybackTimeAsync());
		else
			return pausedCursorTick;
	}

	void TargetTimeline::SetCursorTick(const TimelineTick newTick)
	{
		const auto newTime = TickToTime(newTick);

		pausedCursorTick = newTick;
		chartEditor.SetPlaybackTime(newTime);

		PlaybackStateChangeSyncButtonSoundCursorTime(newTime);
	}

	bool TargetTimeline::GetIsPlayback() const
	{
		return chartEditor.GetIsPlayback();
	}

	void TargetTimeline::PausePlayback()
	{
		chartEditor.PausePlayback();

		const auto playbackTime = chartEditor.GetPlaybackTimeAsync();
		pausedCursorTick = TimeToTick(playbackTime);

		PlaybackStateChangeSyncButtonSoundCursorTime(playbackTime);
	}

	void TargetTimeline::ResumePlayback()
	{
		const auto playbackTime = TickToTime(pausedCursorTick);
		chartEditor.SetPlaybackTime(playbackTime);
		chartEditor.ResumePlayback();

		PlaybackStateChangeSyncButtonSoundCursorTime(playbackTime);
	}

	void TargetTimeline::StopPlayback()
	{
		chartEditor.StopPlayback();

		const auto playbackTime = chartEditor.GetPlaybackTimeAsync();
		pausedCursorTick = TimeToTick(playbackTime);

		PlaybackStateChangeSyncButtonSoundCursorTime(playbackTime);
	}

	i32 TargetTimeline::FindGridDivisionPresetIndex() const
	{
		for (i32 i = 0; i < static_cast<i32>(presetBarGridDivisions.size()); i++)
		{
			if (presetBarGridDivisions[i] == activeBarGridDivision)
				return i;
		}

		return -1;
	}

	void TargetTimeline::SelectNextPresetGridDivision(i32 direction)
	{
		const auto index = FindGridDivisionPresetIndex();
		const auto nextIndex = std::clamp(index + direction, 0, static_cast<int>(presetBarGridDivisions.size()) - 1);

		activeBarGridDivision = presetBarGridDivisions[nextIndex];
	}

	void TargetTimeline::AdvanceCursorByGridDivisionTick(i32 direction, bool beatStep)
	{
		const auto beatIncrement = TimelineTick::FromBeats(1);
		const auto gridIncrement = GridDivisionTick();

		const auto stepDistance = (beatStep ? std::max(beatIncrement, gridIncrement) : gridIncrement);

		const auto newCursorTick = RoundTickToGrid(GetCursorTick()) + (stepDistance * direction);
		const auto clampedCursorTick = std::max(newCursorTick, TimelineTick::Zero());

		const auto preCursorX = GetCursorTimelinePosition();

		SetCursorTick(clampedCursorTick);
		PlayCursorButtonSoundsAndAnimation(clampedCursorTick);

		// NOTE: Keep same relative cursor screen position though might only wanna scroll if the cursor is about to go off-screen (?)
		if (!GetIsPlayback())
			SetScrollX(GetScrollX() + (GetCursorTimelinePosition() - preCursorX));
	}

	void TargetTimeline::AdvanceCursorToNextTarget(i32 direction)
	{
		const auto& targets = workingChart->Targets;
		const auto cursorTick = GetCursorTick();

		auto tryFindIf = [](auto begin, auto end, auto pred) -> auto*
		{
			auto found = std::find_if(begin, end, pred);
			return (found != end) ? &(*found) : nullptr;
		};

		const auto* nextTarget =
			(direction > 0) ? tryFindIf(targets.begin(), targets.end(), [&](auto& t) { return (t.Tick > cursorTick); }) :
			(direction < 0) ? tryFindIf(targets.rbegin(), targets.rend(), [&](auto& t) { return (t.Tick < cursorTick); })
			: nullptr;

		if (nextTarget != nullptr)
		{
			const auto nextTick = nextTarget->Tick;

			SetCursorTick(nextTick);
			PlayCursorButtonSoundsAndAnimation(nextTick);
		}

		CenterCursor();
	}

	void TargetTimeline::PlaybackStateChangeSyncButtonSoundCursorTime(TimeSpan newCursorTime)
	{
		lastFrameButtonSoundCursorTime = thisFrameButtonSoundCursorTime = (newCursorTime - buttonSoundFutureOffset);
		buttonSoundController.PauseAllNegativeVoices();
		buttonSoundController.PauseAllChainSounds();
	}

	f32 TargetTimeline::GetTimelineSize() const
	{
		return GetTimelinePosition(workingChart->DurationOrDefault());
	}

	void TargetTimeline::OnTimelineBaseScroll()
	{
		const auto& io = Gui::GetIO();

		const auto beatIncrement = TimelineTick::FromBeats(1);
		const auto gridIncrement = GridDivisionTick();

		const auto scrollTickIncrement = (io.KeyShift ? std::max(beatIncrement, gridIncrement) : gridIncrement) * static_cast<i32>(io.MouseWheel);
		const auto newCursorTick = std::max(TimelineTick::Zero(), GetCursorTick() + scrollTickIncrement);

		if (const bool seekThroughSong = GetIsPlayback(); seekThroughSong)
		{
			const auto preCursorX = GetCursorTimelinePosition();

			// NOTE: Pause and resume to reset the on-playback start-time
			chartEditor.PausePlayback();
			{
				SetCursorTick(newCursorTick);
			}
			chartEditor.ResumePlayback();

			// NOTE: Keep the cursor at the same relative screen position to prevent potential disorientation
			SetScrollX(GetScrollX() + (GetCursorTimelinePosition() - preCursorX));
		}
		else if (const bool seekingScroll = false; seekingScroll)
		{
			// DEBUG: Neat idea but in practice very disorientating
			const auto preCursorX = GetCursorTimelinePosition();
			{
				SetCursorTick(newCursorTick);
				PlayCursorButtonSoundsAndAnimation(newCursorTick);
			}
			SetScrollX(GetScrollX() + (GetCursorTimelinePosition() - preCursorX));
		}
		else
		{
			TimelineBase::OnTimelineBaseScroll();
		}
	}
}
