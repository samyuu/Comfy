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
	TargetTimeline::TargetTimeline(ChartEditor& parent, Undo::UndoManager& undoManager, ButtonSoundController& buttonSoundController)
		: chartEditor(parent), undoManager(undoManager), buttonSoundController(buttonSoundController)
	{
		scrollSpeed = 2.5f;
		scrollSpeedFast = 5.5f;
		autoScrollCursorOffsetPercentage = 0.35f;
		infoColumnWidth = 180.0f;
	}

	BeatTick TargetTimeline::GridDivisionTick() const
	{
		return BeatTick::FromBars(1) / activeBarGridDivision;
	}

	BeatTick TargetTimeline::ChainSlideDivisionTick() const
	{
		return BeatTick::FromBars(1) / activeBarChainSlideDivision;
	}

	BeatTick TargetTimeline::FloorTickToGrid(BeatTick tick) const
	{
		const auto gridTicks = static_cast<f64>(GridDivisionTick().Ticks());
		return BeatTick::FromTicks(static_cast<i32>(glm::floor(static_cast<f64>(tick.Ticks()) / gridTicks) * gridTicks));
	}

	BeatTick TargetTimeline::RoundTickToGrid(BeatTick tick) const
	{
		const auto gridTicks = static_cast<f64>(GridDivisionTick().Ticks());
		return BeatTick::FromTicks(static_cast<i32>(glm::round(static_cast<f64>(tick.Ticks()) / gridTicks) * gridTicks));
	}

	// TODO: Rename all of these to contain their conversion types in the name and remove Get prefix
	f32 TargetTimeline::GetTimelinePosition(TimeSpan time) const
	{
		return TimelineBase::GetTimelinePosition(time);
	}

	f32 TargetTimeline::GetTimelinePosition(BeatTick tick) const
	{
		return GetTimelinePosition(TickToTime(tick));
	}

	BeatTick TargetTimeline::TimeToTick(TimeSpan time) const
	{
		return workingChart->TimelineMap.GetTickAt(time);
	}

	BeatTick TargetTimeline::TimeToTickFixedTempo(TimeSpan time, Tempo tempo) const
	{
		return workingChart->TimelineMap.GetTickAtFixedTempo(time, tempo);
	}

	BeatTick TargetTimeline::GetBeatTick(f32 position) const
	{
		return TimeToTick(GetTimelineTime(position));
	}

	TimeSpan TargetTimeline::TickToTime(BeatTick tick) const
	{
		return workingChart->TimelineMap.GetTimeAt(tick);
	}

	TimeSpan TargetTimeline::GetTimelineTime(f32 position) const
	{
		return TimelineBase::GetTimelineTime(position);
	}

	BeatTick TargetTimeline::GetCursorMouseXTick(bool floorToGrid) const
	{
		const auto tickAtMousePosition = GetBeatTick(ScreenToTimelinePosition(Gui::GetMousePos().x));
		const auto gridAdjusted = floorToGrid ? FloorTickToGrid(tickAtMousePosition) : tickAtMousePosition;

		// NOTE: There should never be a need to click before the start of the timeline
		const auto clamped = std::max(BeatTick::Zero(), gridAdjusted);

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
#if 0 // NOTE: Using smooth timing here seems to always result in highly irregular sound intervals
		thisFrameButtonSoundCursorTime = (chartEditor.GetSongVoice().GetPositionSmooth() - workingChart->StartOffset);
#else
		thisFrameButtonSoundCursorTime = (chartEditor.GetSongVoice().GetPosition() - workingChart->StartOffset);
#endif

		const auto elapsedTime = (thisFrameButtonSoundCursorTime - lastFrameButtonSoundCursorTime);
		if (elapsedTime >= buttonSoundThresholdAtWhichPlayingSoundsMakesNoSense)
			return;

		const auto futureOffset = (buttonSoundFutureOffset * glm::min(chartEditor.GetSongVoice().GetPlaybackSpeed(), 1.0f));

		if (metronomeEnabled)
		{
			if (metronome.GetVolume() != GlobalUserData.System.Audio.MetronomeVolume)
				metronome.SetVolume(GlobalUserData.System.Audio.MetronomeVolume);
			metronome.UpdatePlaySounds(*workingChart, thisFrameButtonSoundCursorTime, lastFrameButtonSoundCursorTime, futureOffset);
		}

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
			const auto offsetButtonTime = buttonTime - futureOffset;

			if (offsetButtonTime >= lastFrameButtonSoundCursorTime && offsetButtonTime < thisFrameButtonSoundCursorTime)
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
						buttonSoundController.PlayChainSoundSuccess(chainSoundSlot, startTime, externalClock);

						// BUG: Sync chain slides with the left chain ending earlier than the right one. In practice this should rarely ever happen though
						buttonSoundController.FadeOutLastChainSound(chainSoundSlot, startTime);
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

	void TargetTimeline::OnEditorSpritesLoaded(const Graphics::SprSet* sprSet)
	{
		renderHelper.OnEditorSpritesLoaded(sprSet);
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

		Gui::PushStyleVar(ImGuiStyleVar_ItemSpacing, vec2(0.0f, 0.0f));
		Gui::PushStyleVar(ImGuiStyleVar_FramePadding, vec2(8.0f, 8.0f));

		constexpr vec4 transparent = vec4(0.0f);
		Gui::PushStyleColor(ImGuiCol_Button, transparent);
		{
			const f32 timelineEndPosition = GetTimelinePosition(workingChart->DurationOrDefault()) - timelineContentRegion.GetWidth() + 1.0f;
			const bool isPlayback = GetIsPlayback();

			const bool isCursorAtStart = (cursorTime <= TimeSpan::Zero());
			const bool isAtStartOfTimeline = (isCursorAtStart && GetScrollX() == 0.0f);

			const bool isCursorAtEnd = (cursorTime >= workingChart->DurationOrDefault());
			const bool isAtEndOfTimeline = (isCursorAtEnd && GetScrollX() == timelineEndPosition);

			constexpr f32 borderSize = 1.0f;
			Gui::SetCursorPosX(Gui::GetCursorPosX() + borderSize);

			Gui::PushItemDisabledAndTextColorIf(isAtStartOfTimeline);
			if (Gui::Button(ICON_FA_FAST_BACKWARD))
			{
				SetCursorTime(TimeSpan::Zero());
				SetScrollX(0.0f);
			}
			Gui::PopItemDisabledAndTextColorIf(isAtStartOfTimeline);
			Gui::SetWideItemTooltip("Go to Start of Timeline");

			Gui::SameLine();
			Gui::PushItemDisabledAndTextColorIf(isCursorAtStart);
			if (Gui::Button(ICON_FA_BACKWARD))
			{
				AdvanceCursorByGridDivisionTick(-1);
			}
			Gui::PopItemDisabledAndTextColorIf(isCursorAtStart);
			Gui::SetWideItemTooltip("Go to previous Grid Tick");

			Gui::SameLine();
			if (Gui::Button(isPlayback ? ICON_FA_PAUSE : ICON_FA_PLAY))
				isPlayback ? PausePlayback() : ResumePlayback();
			Gui::SetWideItemTooltip(isPlayback ? "Pause Playback" : "Resume Playback");

			Gui::SameLine();
			Gui::PushItemDisabledAndTextColorIf(!isPlayback);
			if (Gui::Button(ICON_FA_STOP))
			{
				StopPlayback();
			}
			Gui::PopItemDisabledAndTextColorIf(!isPlayback);
			Gui::SetWideItemTooltip("Stop Playback");

			Gui::SameLine();
			Gui::PushItemDisabledAndTextColorIf(isCursorAtEnd);
			if (Gui::Button(ICON_FA_FORWARD))
			{
				AdvanceCursorByGridDivisionTick(+1);
			}
			Gui::PopItemDisabledAndTextColorIf(isCursorAtEnd);
			Gui::SetWideItemTooltip("Go to next Grid Tick");

			Gui::SameLine();
			Gui::PushItemDisabledAndTextColorIf(isAtEndOfTimeline);
			if (Gui::Button(ICON_FA_FAST_FORWARD))
			{
				SetCursorTime(workingChart->DurationOrDefault());
				SetScrollX(timelineEndPosition);
			}
			Gui::PopItemDisabledAndTextColorIf(isAtEndOfTimeline);
			Gui::SetWideItemTooltip("Go to End of Timeline");
		}
		Gui::PopStyleColor(1);
		Gui::PopStyleVar(2);
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
			const auto tempTarget = TimelineTarget(BeatTick::Zero(), static_cast<ButtonType>(row));
			renderHelper.DrawButtonIcon(drawList, tempTarget, iconCenters[row], iconScale);
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
		const bool isTupletDivision = (activeBarGridDivision % 3 == 0);

		const u32 barColor = GetColor(EditorColor_Bar);
		const u32 beatColor = GetColor(EditorColor_Beat);
		const u32 gridColor = GetColor(isTupletDivision ? EditorColor_GridTuplet : EditorColor_Grid);
		const u32 gridAltColor = GetColor(isTupletDivision ? EditorColor_GridTupletAlt : EditorColor_GridAlt);
		const u32 barTextColor = Gui::GetColorU32(ImGuiCol_Text);
		const u32 barTimeColor = Gui::GetColorU32(ImGuiCol_Text, 0.5f);

		const i32 songDurationTicks = TimeToTick(workingChart->DurationOrDefault()).Ticks();
		const i32 gridTickStep = GridDivisionTick().Ticks();

		const f32 scrollX = GetScrollX();

		constexpr f32 beatSpacingThreshold = 10.0f;
		constexpr f32 barSpacingThreshold = 64.0f;

		f32 lastBeatTimelineX = -beatSpacingThreshold;
		for (i32 tick = 0, divisions = 0; tick < songDurationTicks; tick += gridTickStep, divisions++)
		{
			const f32 timelineX = GetTimelinePosition(BeatTick(tick));
			if (const auto lastDrawnDistance = (timelineX - lastBeatTimelineX); lastDrawnDistance < beatSpacingThreshold)
				continue;
			lastBeatTimelineX = timelineX;

			const f32 screenX = glm::round(timelineX - scrollX);
			const auto visiblity = GetTimelineVisibility(screenX);

			if (visiblity == TimelineVisibility::Left)
				continue;
			if (visiblity == TimelineVisibility::Right)
				break;

			const vec2 start = timelineContentRegion.GetTL() + vec2(screenX, -(timelineHeaderHeight * 0.35f));
			const vec2 end = timelineContentRegion.GetBL() + vec2(screenX, 0.0f);
			baseDrawList->AddLine(start, end, (tick % BeatTick::TicksPerBeat == 0) ? beatColor : (divisions % 2 == 0 ? gridColor : gridAltColor));
		}

		f32 lastBarTimelineX = -barSpacingThreshold;
		workingChart->TempoMap.ForEachBar([&](const BeatTick barTick, const size_t barIndex)
		{
			const f32 timelineX = GetTimelinePosition(barTick);
			if (const f32 lastDrawnDistance = (timelineX - lastBarTimelineX); lastDrawnDistance < barSpacingThreshold)
				return false;
			lastBarTimelineX = timelineX;

			const f32 screenX = glm::round(timelineX - scrollX);
			const auto visiblity = GetTimelineVisibility(screenX);

			if (visiblity == TimelineVisibility::Left)
				return false;
			if (visiblity == TimelineVisibility::Right)
				return true;

			char buffer[32];
			const vec2 start = timelineContentRegion.GetTL() + vec2(screenX, -(timelineHeaderHeight * 0.85f));
			const vec2 end = timelineContentRegion.GetBL() + vec2(screenX, 0.0f);
			baseDrawList->AddLine(start, end, barColor);
			baseDrawList->AddText(nullptr, 14.0f, start + vec2(3.0f, -4.0f), barTextColor, buffer, buffer + sprintf_s(buffer, "%zu", barIndex));
			baseDrawList->AddText(nullptr, 13.0f, start + vec2(3.0f, -4.0f + 9.0f), barTimeColor, workingChart->TimelineMap.GetTimeAt(barTick).FormatTime().data());
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
		const f32 timeDragTextOffset = Gui::GetStyle().FramePadding.x;
		const f32 timeDragTextWidth = (infoColumnWidth * 0.5f) - timeDragTextOffset;
		const f32 gridDivisionButtonWidth = (infoColumnWidth - timeDragTextWidth);

		Gui::PushStyleVar(ImGuiStyleVar_FramePadding, vec2(Gui::GetStyle().FramePadding.x, 0.0f));
		{
			constexpr f32 dragSpeed = 4.0f;
			f32 cursorDragTicks = static_cast<f32>(GetCursorTick().Ticks());

			Gui::SetCursorPosX(Gui::GetCursorPosX() + timeDragTextOffset);
			if (Gui::ComfyDragText("##TargetTimelineTimeDragText", cursorTime.FormatTime().data(), &cursorDragTicks, dragSpeed, 0.0f, 0.0f, timeDragTextWidth))
				SetCursorTick(std::max(BeatTick::Zero(), RoundTickToGrid(BeatTick::FromTicks(static_cast<i32>(cursorDragTicks)))));

			char buttonNameBuffer[32];
			sprintf_s(buttonNameBuffer, "Grid: 1 / %d", activeBarGridDivision);

			Gui::SameLine(0.0f, 0.0f);
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
		const auto preEnd = timelineContentRegion.GetBL() + vec2(glm::round(GetTimelinePosition(BeatTick::FromBars(1)) - scrollX), 0.0f);
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

		const auto leftMostVisiblePixel = static_cast<i64>(GetTimelinePosition(BeatTick(0)));
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

				ivec2 sig = { tempoChange.Signature.Numerator, tempoChange.Signature.Denominator };
				if (GuiProperty::InputFraction("Time Signature", sig, ivec2(TimeSignature::MinValue, TimeSignature::MaxValue)))
					undoManager.Execute<UpdateTempoChange>(*workingChart, TempoChange(tempoChange.Tick, tempoChange.Tempo, TimeSignature(sig[0], sig[1])));

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
			const bool tooEarly = (target.Tick < BeatTick::FromBars(1));
			constexpr auto tooEarlyOpacity = 0.5f;

			renderHelper.DrawButtonIcon(windowDrawList, target, center, scale, tooEarly ? tooEarlyOpacity : GetButtonEdgeFadeOpacity(screenX));

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

		UpdateInputSelectionDragging(undoManager, *workingChart);
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

		if (Gui::GetIO().KeyCtrl && Gui::GetActiveID() == 0)
		{
			if (Gui::IsKeyPressed(KeyBindings::Cut, false))
				ClipboardCutSelection(undoManager, *workingChart);

			if (Gui::IsKeyPressed(KeyBindings::Copy, false))
				ClipboardCopySelection(undoManager, *workingChart);

			if (Gui::IsKeyPressed(KeyBindings::Paste, false))
				ClipboardPasteSelection(undoManager, *workingChart);
		}
	}

	void TargetTimeline::UpdateCursorKeyboardInput()
	{
		if (Gui::IsWindowHovered() && Gui::IsMouseClicked(1))
			rangeSelection = {};

		if (!Gui::IsWindowFocused())
			return;

		const bool isPlayback = GetIsPlayback();

		const bool useBeatStep = Gui::GetIO().KeyShift || isPlayback;
		const i32 stepDistanceFactor = isPlayback ? 2 : 1;

		if (Gui::IsKeyPressed(KeyBindings::MoveCursorLeft, true))
			AdvanceCursorByGridDivisionTick(-1, useBeatStep, stepDistanceFactor);
		if (Gui::IsKeyPressed(KeyBindings::MoveCursorRight, true))
			AdvanceCursorByGridDivisionTick(+1, useBeatStep, stepDistanceFactor);

		if (Gui::IsKeyPressed(KeyBindings::IncreaseGridPrecision, true))
			SelectNextPresetGridDivision(-1);
		if (Gui::IsKeyPressed(KeyBindings::DecreaseGridPrecision, true))
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

		if (!Gui::GetIO().KeyCtrl)
		{
			auto songVoice = chartEditor.GetSongVoice();
			if (Gui::IsKeyPressed(KeyBindings::DecreasePlaybackSpeed, true))
				songVoice.SetPlaybackSpeed(std::clamp(songVoice.GetPlaybackSpeed() - playbackSpeedStep, playbackSpeedStepMin, playbackSpeedStepMax));
			if (Gui::IsKeyPressed(KeyBindings::IncreasePlaybackSpeed, true))
				songVoice.SetPlaybackSpeed(std::clamp(songVoice.GetPlaybackSpeed() + playbackSpeedStep, playbackSpeedStepMin, playbackSpeedStepMax));
		}

		if (Gui::IsKeyPressed(KeyBindings::ToggleMetronome, false))
			metronomeEnabled ^= true;

		if (Gui::IsKeyPressed(KeyBindings::ToggleTargetHolds, false))
			ToggleSelectedTargetsHolds(undoManager, *workingChart);
	}

	namespace
	{
		constexpr ButtonType GetNextClampedButtonType(const TimelineTarget& target, i32 incrementDirection)
		{
			const auto[min, max] =
				(target.Flags.IsChain || target.Flags.SameTypeSyncCount > 1) ? std::array { ButtonType::SlideL, ButtonType::SlideR } :
				(target.Flags.IsHold) ? std::array { ButtonType::Triangle, ButtonType::Circle } :
				std::array { ButtonType::Triangle, ButtonType::SlideR };

			return static_cast<ButtonType>(std::clamp(static_cast<i32>(target.Type) + incrementDirection, static_cast<i32>(min), static_cast<i32>(max)));
		}

		const TimelineTarget* RecursiveFindBlockingTargetForChangedType(const SortedTargetList& targets, BeatTick tick, ButtonType type, i32 incrementDirection, const TimelineTarget* syncPairBaseTarget)
		{
			const TimelineTarget* blockingTarget = nullptr;
			for (i32 i = 0; i < syncPairBaseTarget->Flags.SyncPairCount; i++)
			{
				if (syncPairBaseTarget[i].Tick == tick && syncPairBaseTarget[i].Type == type)
					blockingTarget = &syncPairBaseTarget[i];
			}

			if (blockingTarget == nullptr || !blockingTarget->IsSelected)
				return blockingTarget;

			if (const auto newType = GetNextClampedButtonType(*blockingTarget, incrementDirection); newType == type)
				return blockingTarget;
			else
				return RecursiveFindBlockingTargetForChangedType(targets, tick, newType, incrementDirection, syncPairBaseTarget);
		}

		bool IsChangedTargetButtonTypeNotBlocked(const SortedTargetList& targets, const TimelineTarget& target, ButtonType newType, i32 incrementDirection)
		{
			const auto& syncPairBaseTarget = (&target)[-target.Flags.IndexWithinSyncPair];
			assert(syncPairBaseTarget.Flags.IndexWithinSyncPair == 0);

			return (RecursiveFindBlockingTargetForChangedType(targets, target.Tick, newType, incrementDirection, &syncPairBaseTarget) == nullptr);
		}
	}

	void TargetTimeline::UpdateInputSelectionDragging(Undo::UndoManager& undoManager, Chart& chart)
	{
		if (!selectionDrag.IsDragging)
		{
			if (!Gui::IsWindowHovered(ImGuiHoveredFlags_AllowWhenBlockedByPopup) || !timelineContentRegion.Contains(Gui::GetMousePos()))
				return;
		}
		else if (Gui::IsMouseReleased(0))
		{
			selectionDrag = {};
		}

		selectionDrag.IsHovering = false;
		if (!selectionDrag.IsDragging && Gui::IsWindowHovered())
		{
			// TODO: Min max visible tick range check optimization 
			const f32 iconHitboxHalfSize = (iconHitboxSize / 2.0f);
			for (const auto& target : chart.Targets)
			{
				if (!target.IsSelected)
					continue;

				const auto center = vec2(GetTimelinePosition(target.Tick) - GetScrollX() + timelineContentRegion.GetTL().x, targetYPositions[static_cast<size_t>(target.Type)]);
				const auto hitbox = ImRect(center - iconHitboxHalfSize, center + iconHitboxHalfSize);

				if (!hitbox.Contains(Gui::GetMousePos()))
					continue;

				selectionDrag.IsHovering = true;
				selectionDrag.ChangeType = Gui::GetIO().KeyShift;

				if (Gui::IsMouseClicked(0))
				{
					selectionDrag.IsDragging = true;
					selectionDrag.TickOnPress = GetCursorMouseXTick(false);
					selectionDrag.TicksMovedSoFar = {};
					// NOTE: Account for the offset between the target center and the relative start mouse position
					selectionDrag.VerticalDistanceMovedSoFar = (Gui::GetMousePos().y - center.y);
					undoManager.DisallowMergeForLastCommand();
				}
			}
		}

		if (selectionDrag.IsDragging || selectionDrag.IsHovering)
			Gui::SetMouseCursor(selectionDrag.ChangeType ? ImGuiMouseCursor_ResizeNS : ImGuiMouseCursor_ResizeEW);

		selectionDrag.LastFrameMouseTick = selectionDrag.ThisFrameMouseTick;
		selectionDrag.ThisFrameMouseTick = GetCursorMouseXTick(false);

		if (selectionDrag.IsDragging)
		{
			Gui::SetActiveID(Gui::GetID(&selectionDrag), Gui::GetCurrentWindow());
			Gui::SetWindowFocus();

			undoManager.ResetMergeTimeThresholdStopwatch();
			if (selectionDrag.ChangeType)
				selectionDrag.VerticalDistanceMovedSoFar += Gui::GetIO().MouseDelta.y;
			else
				selectionDrag.TicksMovedSoFar += (selectionDrag.ThisFrameMouseTick - selectionDrag.LastFrameMouseTick);
		}

		if (selectionDrag.IsDragging)
		{
			const auto cursorTick = GetCursorTick();

			if (selectionDrag.ChangeType)
			{
				const auto heightPerType = (rowHeight - 2.0f);
				const auto typeIncrementDirection = std::clamp(static_cast<i32>(selectionDrag.VerticalDistanceMovedSoFar / heightPerType), -1, +1);
				selectionDrag.VerticalDistanceMovedSoFar -= (typeIncrementDirection * heightPerType);

				if (typeIncrementDirection != 0)
				{
					const auto selectedTargetCount = CountSelectedTargets();

					std::vector<ChangeTargetListTypes::Data> targetTypeData;
					targetTypeData.reserve(selectedTargetCount);

					for (const auto& target : chart.Targets)
					{
						if (!target.IsSelected)
							continue;

						const auto newType = GetNextClampedButtonType(target, typeIncrementDirection);
						auto& data = targetTypeData.emplace_back();
						data.ID = target.ID;
						data.NewValue = IsChangedTargetButtonTypeNotBlocked(chart.Targets, target, newType, typeIncrementDirection) ? newType : target.Type;

						if (target.Tick == cursorTick && data.NewValue != target.Type)
							PlaySingleTargetButtonSoundAndAnimation(data.NewValue, target.Tick);
					}

					if (!targetTypeData.empty())
						undoManager.Execute<ChangeTargetListTypes>(chart, std::move(targetTypeData));
				}
			}
			else
			{
				const auto dragTickIncrement = FloorTickToGrid(selectionDrag.TicksMovedSoFar);
				if (dragTickIncrement != BeatTick::Zero() && !CheckIsAnySyncPairPartiallySelected() && CheckIsSelectionNotBlocked(dragTickIncrement))
				{
					selectionDrag.TicksMovedSoFar -= dragTickIncrement;

					std::vector<IncrementTargetListTicks::Data> targetData;
					targetData.reserve(CountSelectedTargets());

					for (const auto& target : chart.Targets)
					{
						if (!target.IsSelected)
							continue;

						auto& data = targetData.emplace_back();
						data.ID = target.ID;
						data.NewValue = target.Tick + dragTickIncrement;

						if (data.NewValue == cursorTick)
							PlaySingleTargetButtonSoundAndAnimation(target.Type, data.NewValue);
					}

					undoManager.Execute<IncrementTargetListTicks>(chart, std::move(targetData));
				}
			}
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

	bool TargetTimeline::CheckIsSelectionNotBlocked(BeatTick increment) const
	{
		if (increment > BeatTick::Zero())
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

				if (target.IsSelected)
				{
					if (target.Tick + increment < BeatTick::Zero())
						return false;

					if (prevTarget != nullptr && !prevTarget->IsSelected)
					{
						if (target.Tick + increment <= prevTarget->Tick)
							return false;
					}
				}
			}
		}

		return true;
	}

	void TargetTimeline::UpdateInputCursorClick()
	{
		if (!Gui::IsWindowHovered() || !timelineContentRegion.Contains(Gui::GetMousePos()))
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

		if (isCursorScrubbing)
			Gui::SetActiveID(Gui::GetID(&isCursorScrubbing), Gui::GetCurrentWindow());

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
					FillInRangeSelectionTargets(undoManager, *workingChart, buttonType);
				else
					PlaceOrRemoveTarget(undoManager, *workingChart, RoundTickToGrid(GetCursorTick()), buttonType);
			}
		}

		if (Gui::IsKeyPressed(KeyBindings::DeleteSelection, false))
			RemoveAllSelectedTargets(undoManager, *workingChart);
	}

	void TargetTimeline::UpdateInputContextMenu()
	{
		constexpr const char* contextMenuID = "TargetTimelineContextMenu";

		if (Gui::IsMouseReleased(1) && Gui::IsWindowHovered(ImGuiHoveredFlags_AllowWhenBlockedByPopup) && timelineContentRegion.Contains(Gui::GetMousePos()))
		{
			if (!Gui::IsAnyItemHovered())
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

			if (Gui::BeginMenu("Chain Slide Division"))
			{
				for (const auto barDivision : presetBarChainSlideDivisions)
				{
					char nameBuffer[32];
					sprintf_s(nameBuffer, "Set 1 / %d", barDivision);

					bool alreadySelected = (activeBarChainSlideDivision == barDivision);;
					if (Gui::MenuItem(nameBuffer, nullptr, &alreadySelected, !alreadySelected))
						activeBarChainSlideDivision = barDivision;
				}

				Gui::EndMenu();
			}

			if (Gui::BeginMenu("Playback Speed"))
			{
				auto songVoice = chartEditor.GetSongVoice();
				f32 songPlaybackSpeed = songVoice.GetPlaybackSpeed();

				for (const auto presetSpeed : { 1.00f, 0.75f, 0.50f, 0.25f, })
				{
					char b[64]; sprintf_s(b, "Set %3.0f%%", presetSpeed * 100.0f);

					if (Gui::MenuItem(b, nullptr, (presetSpeed == songPlaybackSpeed), (presetSpeed != songPlaybackSpeed)))
						songVoice.SetPlaybackSpeed(presetSpeed);
				}

				if (Gui::BeginMenu("Set Exact"))
				{
					if (auto s = (songPlaybackSpeed * 100.0f); Gui::SliderFloat("##PlaybackSpeedSlider", &s, 10.0f, 200.0f, "%.0f%% Playback Speed"))
						songVoice.SetPlaybackSpeed(s / 100.0f);
					Gui::EndMenu();
				}

				Gui::Separator();
				if (Gui::MenuItem("Speed Down", Input::GetKeyCodeName(KeyBindings::DecreasePlaybackSpeed), nullptr, (songPlaybackSpeed > playbackSpeedStepMin)))
					songVoice.SetPlaybackSpeed(std::clamp(songPlaybackSpeed - playbackSpeedStep, playbackSpeedStepMin, playbackSpeedStepMax));
				if (Gui::MenuItem("Speed Up", Input::GetKeyCodeName(KeyBindings::IncreasePlaybackSpeed), nullptr, (songPlaybackSpeed < playbackSpeedStepMax)))
					songVoice.SetPlaybackSpeed(std::clamp(songPlaybackSpeed + playbackSpeedStep, playbackSpeedStepMin, playbackSpeedStepMax));

				Gui::EndMenu();
			}

			Gui::MenuItem("Metronome Enabled", Input::GetKeyCodeName(KeyBindings::ToggleMetronome), &metronomeEnabled);

			Gui::Separator();

			if (Gui::MenuItem("Toggle Target Holds", Input::GetKeyCodeName(KeyBindings::ToggleTargetHolds), nullptr, (selectionCount > 0)))
				ToggleSelectedTargetsHolds(undoManager, *workingChart);

			if (Gui::BeginMenu("Modify Targets", (selectionCount > 0)))
			{
				if (Gui::MenuItem("Mirror Types"))
					MirrorSelectedTargetTypes(undoManager, *workingChart);

				if (Gui::BeginMenu("Expand Time"))
				{
					if (Gui::MenuItem("2:1 (8th to 4th)"))
						CompressOrExpandSelectedTargetTimes(undoManager, *workingChart, { 2, 1 });
					if (Gui::MenuItem("3:2 (12th to 8th)"))
						CompressOrExpandSelectedTargetTimes(undoManager, *workingChart, { 3, 2 });
					if (Gui::MenuItem("4:3 (16th to 12th)"))
						CompressOrExpandSelectedTargetTimes(undoManager, *workingChart, { 4, 3 });
					Gui::EndMenu();
				}

				if (Gui::BeginMenu("Compress Time"))
				{
					if (Gui::MenuItem("1:2 (4th to 8th)"))
						CompressOrExpandSelectedTargetTimes(undoManager, *workingChart, { 1, 2 });
					if (Gui::MenuItem("2:3 (8th to 12th)"))
						CompressOrExpandSelectedTargetTimes(undoManager, *workingChart, { 2, 3 });
					if (Gui::MenuItem("3:4 (12th to 16th)"))
						CompressOrExpandSelectedTargetTimes(undoManager, *workingChart, { 3, 4 });
					Gui::EndMenu();
				}

				Gui::EndMenu();
			}

			Gui::Separator();

			if (Gui::MenuItem("Cut", "Ctrl + X", nullptr, (selectionCount > 0)))
				ClipboardCutSelection(undoManager, *workingChart);
			if (Gui::MenuItem("Copy", "Ctrl + C", nullptr, (selectionCount > 0)))
				ClipboardCopySelection(undoManager, *workingChart);
			if (Gui::MenuItem("Paste", "Ctrl + V", nullptr, true))
				ClipboardPasteSelection(undoManager, *workingChart);

			Gui::Separator();

			if (Gui::MenuItem("Select All", "", nullptr, (workingChart->Targets.size() > 0)))
				SelectAllTargets(*workingChart);
			if (Gui::MenuItem("Deselect", "", nullptr, (selectionCount > 0)))
				DeselectAllTargets(*workingChart);

			if (Gui::BeginMenu("Refine Selection", (selectionCount > 0)))
			{
				if (Gui::MenuItem("Select every 2nd Target")) SelectEveryNthTarget(*workingChart, 2);
				if (Gui::MenuItem("Select every 3rd Target")) SelectEveryNthTarget(*workingChart, 3);
				if (Gui::MenuItem("Select every 4th Target")) SelectEveryNthTarget(*workingChart, 4);
				Gui::Separator();
				if (Gui::MenuItem("Shift Selection Left")) ShiftTargetSelection(*workingChart, -1);
				if (Gui::MenuItem("Shift Selection Right")) ShiftTargetSelection(*workingChart, +1);
				Gui::Separator();
				if (Gui::MenuItem("Select all Single Targets")) RefineTargetSelectionBySingleTargetsOnly(*workingChart);
				if (Gui::MenuItem("Select all Sync Targets")) RefineTargetSelectionBySyncPairsOnly(*workingChart);
				if (Gui::MenuItem("Select all partially selected Sync Pairs")) SelectAllParticallySelectedSyncPairs(*workingChart);
				Gui::EndMenu();
			}

			Gui::Separator();

			if (Gui::MenuItem("Remove Targets", "Del", nullptr, (selectionCount > 0)))
				RemoveAllSelectedTargets(undoManager, *workingChart, selectionCount);

			Gui::EndPopup();
		}
	}

	void TargetTimeline::UpdateInputBoxSelection()
	{
		constexpr int boxSelectionButton = 1;

		if (Gui::IsWindowHovered(ImGuiHoveredFlags_AllowWhenBlockedByPopup) && timelineContentRegion.Contains(Gui::GetMousePos()))
		{
			if (Gui::IsMouseClicked(boxSelectionButton))
			{
				boxSelection.StartMouse = Gui::GetMousePos();
				boxSelection.StartTick = GetBeatTick(ScreenToTimelinePosition(boxSelection.StartMouse.x));
				boxSelection.IsActive = true;
			}
		}

		if (boxSelection.IsActive && Gui::IsMouseDown(boxSelectionButton))
		{
			boxSelection.EndMouse = Gui::GetMousePos();
			boxSelection.EndTick = GetBeatTick(ScreenToTimelinePosition(boxSelection.EndMouse.x));

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

	void TargetTimeline::ToggleSelectedTargetsHolds(Undo::UndoManager& undoManager, Chart& chart)
	{
		const auto selectedTargetCount = CountSelectedTargets();
		if (selectedTargetCount < 1)
			return;

		const auto cursorTick = GetCursorTick();
		bool hasAnyButtonSoundBeenPlayed = false;

		std::vector<ToggleTargetListIsHold::Data> commandData;
		commandData.reserve(selectedTargetCount);

		for (const auto& target : workingChart->Targets)
		{
			if (!target.IsSelected || IsSlideButtonType(target.Type))
				continue;

			auto& data = commandData.emplace_back();
			data.ID = target.ID;
			data.NewValue = !target.Flags.IsHold;

			if (target.Tick == cursorTick && target.Flags.IsHold != data.NewValue)
			{
				PlaySingleTargetButtonSoundAndAnimation(target.Type, target.Tick);
				hasAnyButtonSoundBeenPlayed = true;
			}
		}

		if (!commandData.empty())
		{
			if (!hasAnyButtonSoundBeenPlayed)
			{
				const auto& frontTarget = chart.Targets[chart.Targets.FindIndex(commandData.front().ID)];
				for (const auto& data : commandData)
				{
					const auto& target = chart.Targets[chart.Targets.FindIndex(data.ID)];
					if (target.Tick != frontTarget.Tick)
						break;

					PlaySingleTargetButtonSoundAndAnimation(target.Type, target.Tick);
				}
			}

			undoManager.Execute<ToggleTargetListIsHold>(*workingChart, std::move(commandData));
		}
	}

	void TargetTimeline::SelectAllTargets(Chart& chart)
	{
		std::for_each(chart.Targets.begin(), chart.Targets.end(), [](auto& t) { t.IsSelected = true; });
	}

	void TargetTimeline::DeselectAllTargets(Chart& chart)
	{
		std::for_each(chart.Targets.begin(), chart.Targets.end(), [](auto& t) { t.IsSelected = false; });
	}

	void TargetTimeline::SelectEveryNthTarget(Chart& chart, size_t n)
	{
		for (size_t i = 0, selectionIndex = 0; i < chart.Targets.size(); i++)
		{
			if (auto& target = chart.Targets[i]; target.IsSelected)
			{
				if (++selectionIndex % n != 0)
					target.IsSelected = false;
			}
		}
	}

	void TargetTimeline::ShiftTargetSelection(Chart& chart, i32 direction)
	{
		if (direction > 0)
		{
			for (i32 i = static_cast<i32>(chart.Targets.size()) - 1; i >= 0; i--)
			{
				auto& target = chart.Targets[i];
				auto* nextTarget = IndexOrNull(i + 1, chart.Targets);

				if (nextTarget != nullptr && target.IsSelected)
					nextTarget->IsSelected = true;
				target.IsSelected = false;
			}
		}
		else
		{
			for (i32 i = 0; i < static_cast<i32>(chart.Targets.size()); i++)
			{
				auto& target = chart.Targets[i];
				auto* prevTarget = IndexOrNull(i - 1, chart.Targets);

				if (prevTarget != nullptr && target.IsSelected)
					prevTarget->IsSelected = true;
				target.IsSelected = false;
			}
		}
	}

	void TargetTimeline::RefineTargetSelectionBySingleTargetsOnly(Chart& chart)
	{
		for (auto& target : chart.Targets)
		{
			if (target.IsSelected && target.Flags.IsSync)
				target.IsSelected = false;
		}
	}

	void TargetTimeline::RefineTargetSelectionBySyncPairsOnly(Chart& chart)
	{
		for (auto& target : chart.Targets)
		{
			if (target.IsSelected && !target.Flags.IsSync)
				target.IsSelected = false;
		}
	}

	void TargetTimeline::SelectAllParticallySelectedSyncPairs(Chart& chart)
	{
		for (i32 i = 0; i < static_cast<i32>(chart.Targets.size());)
		{
			const auto& target = chart.Targets[i];

			const bool anyWithinPairSelected = [&]()
			{
				for (i32 j = i; j < (i + target.Flags.SyncPairCount); j++)
					if (chart.Targets[j].IsSelected)
						return true;
				return false;
			}();

			if (anyWithinPairSelected)
			{
				for (i32 j = i; j < (i + target.Flags.SyncPairCount); j++)
					chart.Targets[j].IsSelected = true;
			}

			assert(target.Flags.SyncPairCount > 0);
			i += target.Flags.SyncPairCount;
		}
	}

	void TargetTimeline::ClipboardCutSelection(Undo::UndoManager& undoManager, Chart& chart)
	{
		const auto selectionCount = CountSelectedTargets();
		if (selectionCount < 1)
			return;

		std::vector<TimelineTarget> selectedTargets;
		selectedTargets.reserve(selectionCount);
		std::copy_if(chart.Targets.begin(), chart.Targets.end(), std::back_inserter(selectedTargets), [&](auto& t) { return t.IsSelected; });

		clipboardHelper.TimelineCopySelectedTargets(selectedTargets);
		undoManager.Execute<CutTargetList>(chart, std::move(selectedTargets));
	}

	void TargetTimeline::ClipboardCopySelection(Undo::UndoManager& undoManager, Chart& chart)
	{
		const auto selectionCount = CountSelectedTargets();
		if (selectionCount < 1)
			return;

		std::vector<TimelineTarget> selectedTargets;
		selectedTargets.reserve(selectionCount);
		std::copy_if(chart.Targets.begin(), chart.Targets.end(), std::back_inserter(selectedTargets), [&](auto& t) { return t.IsSelected; });

		clipboardHelper.TimelineCopySelectedTargets(selectedTargets);
	}

	void TargetTimeline::ClipboardPasteSelection(Undo::UndoManager& undoManager, Chart& chart)
	{
		auto optionalPasteTargets = clipboardHelper.TimelineTryGetPasteTargets();
		if (!optionalPasteTargets.has_value() || optionalPasteTargets->empty())
			return;

		auto pasteTargets = std::move(optionalPasteTargets.value());

		const auto baseTick = FloorTickToGrid(GetCursorTick()) - pasteTargets.front().Tick;
		for (auto& target : pasteTargets)
			target.Tick += baseTick;

		auto targetAlreadyExists = [&](const auto& t) { return (chart.Targets.FindIndex(t.Tick, t.Type) > -1); };
		pasteTargets.erase(std::remove_if(pasteTargets.begin(), pasteTargets.end(), targetAlreadyExists), pasteTargets.end());

		if (!pasteTargets.empty())
		{
			for (size_t i = 0; i < pasteTargets.size(); i++)
			{
				if (const auto& t = pasteTargets[i]; t.Tick == pasteTargets[0].Tick)
					PlaySingleTargetButtonSoundAndAnimation(t);
				else
					break;
			}

			undoManager.Execute<PasteTargetList>(chart, std::move(pasteTargets));
		}
	}

	void TargetTimeline::FillInRangeSelectionTargets(Undo::UndoManager& undoManager, Chart& chart, ButtonType type)
	{
		const auto startTick = RoundTickToGrid(std::min(rangeSelection.StartTick, rangeSelection.EndTick));
		const auto endTick = RoundTickToGrid(std::max(rangeSelection.StartTick, rangeSelection.EndTick));

		// TODO: Come up with better chain placement controls, maybe shift + direction to place chain start, move cursor then press again to "confirm" end placement (?)
		const bool placeChain = IsSlideButtonType(type);

		const auto divisionTick = placeChain ? ChainSlideDivisionTick() : GridDivisionTick();
		const auto targetCount = ((endTick - startTick).Ticks() / divisionTick.Ticks()) + 1;

		std::vector<TimelineTarget> targets;
		targets.reserve(targetCount);

		for (i32 i = 0; i < targetCount; i++)
		{
			const auto tick = BeatTick(startTick.Ticks() + (i * divisionTick.Ticks()));
			if (chart.Targets.FindIndex(tick, type) <= -1)
			{
				auto& target = targets.emplace_back(tick, type);

				if (placeChain)
					target.Flags.IsChain = true;
			}
		}

		if (!targets.empty())
		{
			PlaySingleTargetButtonSoundAndAnimation(targets.front());
			undoManager.Execute<AddTargetList>(chart, std::move(targets));
		}
	}

	void TargetTimeline::PlaceOrRemoveTarget(Undo::UndoManager& undoManager, Chart& chart, BeatTick tick, ButtonType type)
	{
		const auto existingTargetIndex = chart.Targets.FindIndex(tick, type);
		const auto* existingTarget = IndexOrNull(existingTargetIndex, chart.Targets);

		// NOTE: Double hit sound if a target gets placed in front of the cursor position.
		//		 Keeping it this way could make it easier to notice when real time targets are not placed accurately to the beat (?)
		if (!buttonSoundOnSuccessfulPlacementOnly)
			PlayTargetButtonTypeSound(type);

		auto isSlide = [](const auto& target) { return IsSlideButtonType(target.Type) && !target.Flags.IsChain; };
		auto isDoubleSync = [](const auto& target) { return (target.Flags.SameTypeSyncCount > 1); };

		if (existingTarget != nullptr && (!isSlide(*existingTarget) || isDoubleSync(*existingTarget)))
		{
			if (!GetIsPlayback())
			{
				if (buttonSoundOnSuccessfulPlacementOnly)
					PlayTargetButtonTypeSound(type);

				if (existingTarget->Flags.SameTypeSyncCount == 1)
				{
					undoManager.Execute<RemoveTarget>(chart, *existingTarget);
				}
				else
				{
					std::vector<TimelineTarget> sameTypeSyncTargets;
					sameTypeSyncTargets.reserve(existingTarget->Flags.SameTypeSyncCount);
					for (size_t i = 0; i < existingTarget->Flags.SameTypeSyncCount; i++)
						sameTypeSyncTargets.push_back(existingTarget[i]);

					undoManager.Execute<RemoveTargetList>(chart, std::move(sameTypeSyncTargets));
				}
			}
		}
		else
		{
			const bool isPartOfExistingSyncPair = (chart.Targets.FindIndex(tick) > -1);
			if (buttonSoundOnSuccessfulPlacementOnly && (!GetIsPlayback() || !isPartOfExistingSyncPair))
				PlayTargetButtonTypeSound(type);

			undoManager.Execute<AddTarget>(chart, TimelineTarget(tick, type));
		}

		const auto buttonIndex = static_cast<size_t>(type);
		buttonAnimations[buttonIndex].Tick = tick;
		buttonAnimations[buttonIndex].ElapsedTime = TimeSpan::Zero();
	}

	void TargetTimeline::RemoveAllSelectedTargets(Undo::UndoManager& undoManager, Chart& chart, std::optional<size_t> preCalculatedSelectionCount)
	{
		const auto selectionCount = [&] { return (preCalculatedSelectionCount.has_value()) ? preCalculatedSelectionCount.value() : CountSelectedTargets(); }();
		if (selectionCount < 1)
			return;

		std::vector<TimelineTarget> targetsToRemove;
		targetsToRemove.reserve(selectionCount);

		std::copy_if(
			chart.Targets.begin(),
			chart.Targets.end(),
			std::back_inserter(targetsToRemove),
			[](const auto& t) { return t.IsSelected; });

		assert(targetsToRemove.size() == selectionCount);
		undoManager.Execute<RemoveTargetList>(chart, std::move(targetsToRemove));
	}

	void TargetTimeline::MirrorSelectedTargetTypes(Undo::UndoManager& undoManager, Chart& chart)
	{
		const size_t selectionCount = CountSelectedTargets();
		if (selectionCount < 1)
			return;

		std::vector<MirrorTargetListTypes::Data> targetData;
		targetData.reserve(selectionCount);

		for (const auto& target : chart.Targets)
		{
			if (!target.IsSelected)
				continue;

			const auto mirroredType = MirrorButtonType(target.Type);

			const auto* existingTarget = IndexOrNull(chart.Targets.FindIndex(target.Tick, mirroredType), chart.Targets);
			if (existingTarget != nullptr && !existingTarget->IsSelected)
				continue;

			auto& data = targetData.emplace_back();
			data.ID = target.ID;
			data.NewValue = mirroredType;
		}

		if (!targetData.empty())
		{
			undoManager.DisallowMergeForLastCommand();
			undoManager.Execute<MirrorTargetListTypes>(chart, std::move(targetData));
		}
	}

	void TargetTimeline::CompressOrExpandSelectedTargetTimes(Undo::UndoManager& undoManager, Chart& chart, std::array<i32, 2> ratio)
	{
		assert(ratio[0] > 0 && ratio[1] > 0 && ratio[0] != ratio[1]);

		const size_t selectionCount = CountSelectedTargets();
		if (selectionCount < 1)
			return;

		std::optional<BeatTick> firstTick = {};

		std::vector<ChangeTargetListTicks::Data> targetData;
		targetData.reserve(selectionCount);

		for (const auto& target : chart.Targets)
		{
			if (!target.IsSelected)
				continue;

			if (!firstTick.has_value())
				firstTick = target.Tick;

			auto& data = targetData.emplace_back();
			data.ID = target.ID;
			data.NewValue = (((target.Tick - *firstTick) / ratio[1]) * ratio[0]) + *firstTick;
		}

		if (!targetData.empty())
		{
			undoManager.DisallowMergeForLastCommand();
			if (ratio[0] < ratio[1])
				undoManager.Execute<CompressTargetListTicks>(chart, std::move(targetData));
			else
				undoManager.Execute<ExpandTargetListTicks>(chart, std::move(targetData));
		}
	}

	void TargetTimeline::PlayTargetButtonTypeSound(ButtonType type)
	{
		if (IsSlideButtonType(type))
			buttonSoundController.PlaySlideSound();
		else
			buttonSoundController.PlayButtonSound();
	}

	void TargetTimeline::PlayCursorButtonSoundsAndAnimation(BeatTick cursorTick)
	{
		for (const auto& target : workingChart->Targets)
		{
			if (target.Tick == cursorTick)
				PlaySingleTargetButtonSoundAndAnimation(target);
		}
	}

	void TargetTimeline::PlaySingleTargetButtonSoundAndAnimation(const TimelineTarget& target)
	{
		PlaySingleTargetButtonSoundAndAnimation(target.Type, target.Tick);
	}

	void TargetTimeline::PlaySingleTargetButtonSoundAndAnimation(ButtonType buttonType, BeatTick buttonTick)
	{
		// NOTE: During playback the sound will be handled automatically already
		if (!GetIsPlayback())
			PlayTargetButtonTypeSound(buttonType);

		const auto buttonIndex = static_cast<size_t>(buttonType);
		buttonAnimations[buttonIndex].Tick = buttonTick;
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

	BeatTick TargetTimeline::GetCursorTick() const
	{
		if (chartEditor.GetIsPlayback())
			return TimeToTick(chartEditor.GetPlaybackTimeAsync());
		else
			return pausedCursorTick;
	}

	void TargetTimeline::SetCursorTick(const BeatTick newTick)
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

	void TargetTimeline::ResetScrollAndZoom()
	{
		SetScrollX(0.0f);
		zoomLevel = 2.0f;
	}

	TimelineMetronome& TargetTimeline::GetMetronome()
	{
		return metronome;
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

	void TargetTimeline::AdvanceCursorByGridDivisionTick(i32 direction, bool beatStep, i32 distanceFactor)
	{
		const auto beatIncrement = BeatTick::FromBeats(1);
		const auto gridIncrement = GridDivisionTick();

		const auto stepDistance = (beatStep ? std::max(beatIncrement, gridIncrement) : gridIncrement) * distanceFactor;

		const auto newCursorTick = RoundTickToGrid(GetCursorTick()) + (stepDistance * direction);
		const auto clampedCursorTick = std::max(newCursorTick, BeatTick::Zero());

		const auto preCursorX = GetCursorTimelinePosition();

		SetCursorTick(clampedCursorTick);
		PlayCursorButtonSoundsAndAnimation(clampedCursorTick);

		// NOTE: Keep same relative cursor screen position though might only wanna scroll if the cursor is about to go off-screen (?)
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
		metronome.PauseAllNegativeVoices();
	}

	f32 TargetTimeline::GetTimelineSize() const
	{
		return GetTimelinePosition(workingChart->DurationOrDefault());
	}

	void TargetTimeline::OnTimelineBaseScroll()
	{
		const auto& io = Gui::GetIO();

		const auto beatIncrement = BeatTick::FromBeats(1);
		const auto gridIncrement = GridDivisionTick();

		const auto scrollTickIncrement = (io.KeyShift ? std::max(beatIncrement, gridIncrement) : gridIncrement) * static_cast<i32>(io.MouseWheel);
		const auto newCursorTick = std::max(BeatTick::Zero(), GetCursorTick() + scrollTickIncrement);

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
