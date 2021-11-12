#include "TargetTimeline.h"
#include "Editor/Chart/ChartEditor.h"
#include "Editor/Chart/ChartCommands.h"
#include "Editor/Chart/SortedTempoMap.h"
#include "Time/TimeSpan.h"
#include "ImGui/Extensions/PropertyEditor.h"
#include <FontIcons.h>

namespace Comfy::Studio::Editor
{
	namespace
	{
		struct BarDivisionMultiBindingPair
		{
			i32 BarDivision;
			const Input::MultiBinding* MultiBinding;
		};

		constexpr std::array<BarDivisionMultiBindingPair, 10> PresetBarGridDivisions =
		{
			BarDivisionMultiBindingPair { 4, &GlobalUserData.Input.TargetTimeline_SetGridDivision_4 },
			BarDivisionMultiBindingPair { 8, &GlobalUserData.Input.TargetTimeline_SetGridDivision_8 },
			BarDivisionMultiBindingPair { 12, &GlobalUserData.Input.TargetTimeline_SetGridDivision_12 },
			BarDivisionMultiBindingPair { 16, &GlobalUserData.Input.TargetTimeline_SetGridDivision_16 },
			BarDivisionMultiBindingPair { 24, &GlobalUserData.Input.TargetTimeline_SetGridDivision_24 },
			BarDivisionMultiBindingPair { 32, &GlobalUserData.Input.TargetTimeline_SetGridDivision_32 },
			BarDivisionMultiBindingPair { 48, &GlobalUserData.Input.TargetTimeline_SetGridDivision_48 },
			BarDivisionMultiBindingPair { 64, &GlobalUserData.Input.TargetTimeline_SetGridDivision_64 },
			BarDivisionMultiBindingPair { 96, &GlobalUserData.Input.TargetTimeline_SetGridDivision_96 },
			BarDivisionMultiBindingPair { 192, &GlobalUserData.Input.TargetTimeline_SetGridDivision_192 },
		};

		constexpr std::array<BarDivisionMultiBindingPair, 6> PresetBarChainSlideDivisions =
		{
			BarDivisionMultiBindingPair { 12, &GlobalUserData.Input.TargetTimeline_SetChainSlideGridDivision_12 },
			BarDivisionMultiBindingPair { 16, &GlobalUserData.Input.TargetTimeline_SetChainSlideGridDivision_16 },
			BarDivisionMultiBindingPair { 24, &GlobalUserData.Input.TargetTimeline_SetChainSlideGridDivision_24 },
			BarDivisionMultiBindingPair { 32, &GlobalUserData.Input.TargetTimeline_SetChainSlideGridDivision_32 },
			BarDivisionMultiBindingPair { 48, &GlobalUserData.Input.TargetTimeline_SetChainSlideGridDivision_48 },
			BarDivisionMultiBindingPair { 64, &GlobalUserData.Input.TargetTimeline_SetChainSlideGridDivision_64 },
		};
	}

	TargetTimeline::TargetTimeline(ChartEditor& parent, Undo::UndoManager& undoManager, ButtonSoundController& buttonSoundController)
		: chartEditor(parent), undoManager(undoManager), buttonSoundController(buttonSoundController)
	{
		mouseScrollSpeed = 2.5f;
		mouseScrollSpeedFast = 5.5f;
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
		const f64 gridTicks = static_cast<f64>(GridDivisionTick().Ticks());
		return BeatTick::FromTicks(static_cast<i32>(glm::floor(static_cast<f64>(tick.Ticks()) / gridTicks) * gridTicks));
	}

	BeatTick TargetTimeline::RoundTickToGrid(BeatTick tick) const
	{
		const f64 gridTicks = static_cast<f64>(GridDivisionTick().Ticks());
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
		return workingChart->TempoMap.TimeToTick(time);
	}

	BeatTick TargetTimeline::GetBeatTick(f32 position) const
	{
		return TimeToTick(GetTimelineTime(position));
	}

	TimeSpan TargetTimeline::TickToTime(BeatTick tick) const
	{
		return workingChart->TempoMap.TickToTime(tick);
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
		const auto clamped = Max(BeatTick::Zero(), gridAdjusted);

		return clamped;
	}

	f32 TargetTimeline::GetButtonEdgeFadeOpacity(f32 screenX) const
	{
		constexpr f32 fadeSpan = 35.0f;

#if 0 // NOTE: Left side fade
		if (screenX < 0.0f)
			return 0.0f;

		const f32 lowerThreshold = fadeSpan;
		if (screenX < lowerThreshold)
			return ImLerp(0.0f, 1.0f, screenX / lowerThreshold);
#endif

#if 0 // NOTE: Right side fade
		if (screenX > baseWindow->Size.x)
			return 0.0f;

		const f32 upperThreshold = baseWindow->Size.x - fadeSpan;
		if (screenX > upperThreshold)
			return ImLerp(0.0f, 1.0f, 1.0f - ((screenX - upperThreshold) / (baseWindow->Size.x - upperThreshold)));
#endif

		return 1.0f;
	}

	void TargetTimeline::OnUpdate()
	{
		smoothScrollTimeSec.x = GlobalUserData.TargetTimeline.SmoothScrollTimeSec;

		if (GlobalUserData.TargetTimeline.ScalingBehavior == TargetTimelineScalingBehavior::AutoFit)
		{
			const f32 minRowHeight = GlobalUserData.TargetTimeline.ScalingBehaviorAutoFit.MinRowHeight;
			const f32 maxRowHeight = GlobalUserData.TargetTimeline.ScalingBehaviorAutoFit.MaxRowHeight;

			const f32 currentHeight = regions.Content.GetHeight();
			const f32 maxHeight = (maxRowHeight * static_cast<f32>(EnumCount<ButtonType>()));

			// NOTE: Floor to multiple of two to avoid half-pixel hitboxes
			const f32 scaledRowHeightFlooredToMultipleOfTwo = glm::floor((maxRowHeight * (currentHeight / maxHeight)) / 2.0f) * 2.0f;
			rowHeight = Clamp(scaledRowHeightFlooredToMultipleOfTwo, minRowHeight, maxRowHeight);
			iconScale = (rowHeight / TargetTimelineDefaultRowHeight);
			iconHitboxSize = (rowHeight - TargetTimelineRowHeightHitboxOffset);
		}
		else if (GlobalUserData.TargetTimeline.ScalingBehavior == TargetTimelineScalingBehavior::FixedSize)
		{
			iconScale = GlobalUserData.TargetTimeline.ScalingBehaviorFixedSize.IconScale;
			rowHeight = GlobalUserData.TargetTimeline.ScalingBehaviorFixedSize.RowHeight;
			iconHitboxSize = (rowHeight - TargetTimelineRowHeightHitboxOffset);
		}
		else
		{
			assert(false);
		}

		UpdateOffsetChangeCursorTimeAdjustment();

		if (GetIsPlayback())
			UpdatePlaybackButtonSounds();
	}

	void TargetTimeline::UpdateOffsetChangeCursorTimeAdjustment()
	{
		lastFrameStartOffset = thisFrameStartOffset;
		thisFrameStartOffset = workingChart->SongOffset;

		// NOTE: Cancel out the cursor being moved by a change of song offset because this just feels more intuitive to use
		//		 and always automatically applying the song offset to the playback time makes other calculations easier
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
		thisFrameButtonSoundCursorTime = (chartEditor.GetSongVoice().GetPositionSmooth() - workingChart->SongOffset);
#else
		thisFrameButtonSoundCursorTime = (chartEditor.GetSongVoice().GetPosition() - workingChart->SongOffset);
#endif

		const auto elapsedTime = (thisFrameButtonSoundCursorTime - lastFrameButtonSoundCursorTime);
		if (elapsedTime >= buttonSoundThresholdAtWhichPlayingSoundsMakesNoSense)
			return;

		const auto futureOffset = (buttonSoundFutureOffset * Min(chartEditor.GetSongVoice().GetPlaybackSpeed(), 1.0f));

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

			const auto buttonTime = TickToTime(target.Tick);
			const auto offsetButtonTime = buttonTime - futureOffset;

			if (offsetButtonTime >= lastFrameButtonSoundCursorTime && offsetButtonTime < thisFrameButtonSoundCursorTime)
			{
				// NOTE: Don't wanna cause any audio cutoffs. If this happens the future threshold is either set too low for the current frame time
				//		 or playback was started on top of an existing target
				const auto startTime = Min((thisFrameButtonSoundCursorTime - buttonTime), TimeSpan::Zero());
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
			auto approxmiatelySame = [](f32 a, f32 b, f32 threshold) -> bool { return glm::abs(a - b) < threshold; };

			const f32 timelineEndPosition = GetTimelinePosition(workingChart->DurationOrDefault()) - regions.Content.GetWidth() + 1.0f;
			const bool isPlayback = GetIsPlayback();

			const bool isCursorAtStart = (cursorTime <= TimeSpan::Zero());
			const bool isAtStartOfTimeline = (isCursorAtStart && approxmiatelySame(GetScrollTargetX(), 0.0f, 0.01f));

			const bool isCursorAtEnd = (cursorTime >= workingChart->DurationOrDefault());
			const bool isAtEndOfTimeline = (isCursorAtEnd && approxmiatelySame(GetScrollTargetX(), timelineEndPosition, 0.01f));

			constexpr f32 borderSize = 1.0f;
			Gui::SetCursorPosX(Gui::GetCursorPosX() + borderSize);

			Gui::PushItemDisabledAndTextColorIf(isAtStartOfTimeline);
			if (Gui::Button(ICON_FA_FAST_BACKWARD))
			{
				SetCursorTime(TimeSpan::Zero());
				SetScrollTargetX(0.0f);
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
				SetScrollTargetX(timelineEndPosition);
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

		auto* drawList = Gui::GetWindowDrawList();
		std::array<vec2, EnumCount<ButtonType>()> iconCenters;

		for (size_t row = 0; row < EnumCount<ButtonType>(); row++)
		{
			const f32 y = row * rowHeight;
			const vec2 start = vec2(0.0f, y) + regions.InfoColumnContent.GetTL();
			const vec2 end = vec2(infoColumnWidth, y + rowHeight) + regions.InfoColumnContent.GetTL();

			const vec2 center = vec2(start + end) / 2.0f;
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
		const vec2 timelineTL = regions.Content.GetTL();
		const vec2 timelineWidth = vec2(regions.Content.GetWidth(), 0.0f);

		for (size_t row = 0; row < EnumCount<ButtonType>(); row++)
		{
			const vec2 start = timelineTL + vec2(0.0f, row * rowHeight);
			const vec2 end = start + timelineWidth;

			baseWindowDrawList->AddLine(start, end, GetColor(EditorColor_TimelineRowSeparator));
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
			if (const f32 lastDrawnDistance = (timelineX - lastBeatTimelineX); lastDrawnDistance < beatSpacingThreshold)
				continue;
			lastBeatTimelineX = timelineX;

			const f32 screenX = glm::round(timelineX - scrollX);
			const auto visiblity = GetTimelineVisibility(screenX);

			if (visiblity == TimelineVisibility::Left)
				continue;
			if (visiblity == TimelineVisibility::Right)
				break;

			const vec2 start = regions.Content.GetTL() + vec2(screenX, -(timelineHeaderHeight * 0.35f));
			const vec2 end = regions.Content.GetBL() + vec2(screenX, 0.0f);
			baseWindowDrawList->AddLine(start, end, (tick % BeatTick::TicksPerBeat == 0) ? beatColor : (divisions % 2 == 0 ? gridColor : gridAltColor));
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
			const vec2 start = regions.Content.GetTL() + vec2(screenX, -(timelineHeaderHeight * 0.85f));
			const vec2 end = regions.Content.GetBL() + vec2(screenX, 0.0f);
			baseWindowDrawList->AddLine(start, end, barColor);
			baseWindowDrawList->AddText(nullptr, 14.0f, start + vec2(3.0f, -4.0f), barTextColor, buffer, buffer + sprintf_s(buffer, "%zu", barIndex));
			baseWindowDrawList->AddText(nullptr, 13.0f, start + vec2(3.0f, -4.0f + 9.0f), barTimeColor, TickToTime(barTick).FormatTime().data());
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
		const auto& style = Gui::GetStyle();
		const f32 timeInputWidth = infoColumnWidth / 3.0f;
		const f32 speedButtonWidth = infoColumnWidth / 3.75f;
		const f32 gridDivisionButtonWidth = infoColumnWidth / 2.5f;
		assert((timeInputWidth + speedButtonWidth + gridDivisionButtonWidth) == infoColumnWidth);

		Gui::PushStyleVar(ImGuiStyleVar_FramePadding, vec2(style.FramePadding.x, 0.0f));
		{
			ImGuiInputTextFlags inputTextFlags = ImGuiInputTextFlags_AutoSelectAll;
			if (GetIsPlayback()) // NOTE: Prevent the playback cursor getting stuck
				inputTextFlags |= ImGuiInputTextFlags_EnterReturnsTrue;

			TimeSpan timeInputTime = GetCursorTime();
			Gui::PushStyleColor(ImGuiCol_FrameBg, infoColumnTimeInput.WidgetActiveLastFrame ? Gui::GetColorU32(ImGuiCol_ButtonHovered) : 0);
			if (Gui::InputFormattedTimeSpan("##TimeInput", &timeInputTime, vec2(timeInputWidth, 0.0f), inputTextFlags))
			{
				const auto newTime = Clamp(timeInputTime, TimeSpan::Zero(), workingChart->DurationOrDefault());
				const auto newTimeTickSnapped = TickToTime(TimeToTick(newTime));

				// HACK: Same as used by the sync window to prevent issues for when the focus is lost on the same frame that the cursor was moved via a mouse click
				infoColumnTimeInput.LastFrameCursorTick = infoColumnTimeInput.ThisFrameCursorTick;
				infoColumnTimeInput.ThisFrameCursorTick = RoundTickToGrid(GetCursorTick());
				const bool ignoreInput = !GetIsPlayback() && (infoColumnTimeInput.ThisFrameCursorTick != infoColumnTimeInput.LastFrameCursorTick);

				if (!ignoreInput && newTimeTickSnapped != GetCursorTime())
				{
					const f32 preCursorX = GetCursorTimelinePosition();

					SetCursorTime(newTimeTickSnapped);
					PlayCursorButtonSoundsAndAnimation(GetCursorTick());

					SetScrollTargetX(GetScrollTargetX() + (GetCursorTimelinePosition() - preCursorX));
				}
			}
			Gui::PopStyleColor(1);
			infoColumnTimeInput.WidgetActiveLastFrame = (Gui::IsItemHovered() || Gui::IsItemActive());

			char playbackSpeedText[64];
			sprintf_s(playbackSpeedText, "%.0f%%", ToPercent(chartEditor.GetSongVoice().GetPlaybackSpeed()));

			Gui::SameLine(0.0f, 0.0f);
			if (Gui::ButtonEx(playbackSpeedText, vec2(speedButtonWidth, 0.0f), ImGuiButtonFlags_PressedOnClick))
				SelectNextPlaybackSpeedLevel(+1);
			else if (Gui::IsItemClicked(1))
				SelectNextPlaybackSpeedLevel(-1);

			char buttonNameBuffer[64];
			sprintf_s(buttonNameBuffer, "Grid: 1 / %d", activeBarGridDivision);

			Gui::SameLine(0.0f, 0.0f);
			if (Gui::ButtonEx(buttonNameBuffer, vec2(gridDivisionButtonWidth, scrollbarSize.y), ImGuiButtonFlags_PressedOnClick))
				SelectNextPresetGridDivision(+1);
			else if (Gui::IsItemClicked(1))
				SelectNextPresetGridDivision(-1);
		}
		Gui::PopStyleVar(1);
	}

	void TargetTimeline::DrawOutOfBoundsBackground()
	{
		const u32 outOfBoundsDimColor = GetColor(EditorColor_OutOfBoundsDim);
		const f32 scrollX = GetScrollX();

		const vec2 preStart = regions.Content.GetTL();
		const vec2 preEnd = regions.Content.GetBL() + vec2(glm::round(GetTimelinePosition(BeatTick::FromBars(1)) - scrollX), 0.0f);
		if (preEnd.x - preStart.x > 0.0f)
			baseWindowDrawList->AddRectFilled(preStart, preEnd, outOfBoundsDimColor);

		const vec2 postStart = regions.Content.GetTL() + vec2(glm::round(GetTimelinePosition(workingChart->DurationOrDefault()) - scrollX), 0.0f);
		const vec2 postEnd = regions.Content.GetBR();
		if (postEnd.x - postStart.x > 0.0f)
			baseWindowDrawList->AddRectFilled(postStart, postEnd, outOfBoundsDimColor);
	}

	void TargetTimeline::DrawCheckUpdateWaveform()
	{
		if (zoomLevelChangedThisFrame)
			waveformUpdatePending = true;

		if (waveformUpdatePending && waveformUpdateStopwatch.GetElapsed() >= waveformUpdateInterval)
		{
			if (const bool waveformLoadedForNewSong = (songWaveform.GetPixelCount() <= 0); waveformLoadedForNewSong)
			{
				waveformFadeInStopwatch.Restart();
				waveformExpandStopwatch.Restart();
			}

			const TimeSpan timePerPixel = GetTimelineTime(2.0f) - GetTimelineTime(1.0f);
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

		const f32 timelineHeight = (static_cast<f32>(ButtonType::Count) * rowHeight);
		const f32 scrollXSongOffset = GetScrollX() + GetTimelinePosition(workingChart->SongOffset);

		const f32 fadeInProgress = static_cast<f32>(ConvertRangeClamped<f64>(0.0, waveformFadeInDuration.TotalSeconds(), 0.0, 1.0, waveformFadeInStopwatch.GetElapsed().TotalSeconds()));
		const f32 expandProgress = static_cast<f32>(ConvertRangeClamped<f64>(0.0, waveformExpandDuration.TotalSeconds(), 0.0, 1.0, waveformExpandStopwatch.GetElapsed().TotalSeconds()));

		const vec4 waveformTint = { 1.0f, 1.0f, 1.0f, fadeInProgress };
		const f32 waveformHeightFactor = expandProgress;

		songTextureCachedWaveform.Draw(baseWindowDrawList,
			regions.Content.GetTL(),
			regions.Content.GetTL() + vec2(regions.Content.GetWidth(), timelineHeight),
			scrollXSongOffset,
			Gui::ColorConvertFloat4ToU32(waveformTint),
			waveformHeightFactor);
	}

	void TargetTimeline::DrawWaveformIndividualVertexLines()
	{
		const f32 scrollXSongOffset = GetScrollX() + GetTimelinePosition(workingChart->SongOffset);

		const i64 leftMostVisiblePixel = static_cast<i64>(GetTimelinePosition(BeatTick(0)));
		const i64 rightMostVisiblePixel = leftMostVisiblePixel + static_cast<i64>(regions.Content.GetWidth());
		const i64 waveformPixelCount = static_cast<i64>(songWaveform.GetPixelCount());

		const f32 timelineX = regions.Content.GetTL().x;
		const f32 timelineHeight = (static_cast<f32>(ButtonType::Count) * rowHeight);
		const f32 timelineCenterY = regions.Content.GetTL().y + (timelineHeight * 0.5f);

		const u32 waveformColor = GetColor(EditorColor_Waveform);

		// NOTE: To try and mitigate "flashes" while resizing the timeline, optimally this should be equal to the average PCM of the last visible area
		const f32 amplitudeDuringUpdate = (0.025f * timelineHeight);

		for (i64 screenPixel = leftMostVisiblePixel; screenPixel < waveformPixelCount && screenPixel < rightMostVisiblePixel; screenPixel++)
		{
			const i64 timelinePixel = Min(static_cast<i64>(glm::round(screenPixel + scrollXSongOffset)), static_cast<i64>(waveformPixelCount - 1));
			if (timelinePixel < 0)
				continue;

			constexpr u32 channelsToVisualize = 2;
			for (u32 channel = 0; channel < channelsToVisualize; channel++)
			{
				const f32 amplitude = waveformUpdatePending ? amplitudeDuringUpdate : (songWaveform.GetNormalizedPCMForPixel(timelinePixel, channel) * timelineHeight);
				if (amplitude < 1.0f)
					continue;

				const f32 x = screenPixel + timelineX;
				const f32 halfAmplitude = amplitude * 0.5f;

				const vec2 start = vec2(x, timelineCenterY - halfAmplitude);
				const vec2 end = vec2(x, timelineCenterY + halfAmplitude);

				baseWindowDrawList->AddLine(start, end, waveformColor);
			}
		}
	}

	void TargetTimeline::DrawTimelineTempoMap()
	{
		const auto& tempoMap = workingChart->TempoMap;

		constexpr const char* tempoChangePopupName = "##TempoChangePopup";
		vec2 tempoPopupWindowStartPosition = {};
		vec2 tempoPopupWindowStartPivot = vec2(0.5f, 1.0f);
		vec2 tempoPopupWindowStartSize = vec2(280.0f, 0.0f);

		f32 lastDrawnTimelineX = 0.0f;

		for (size_t i = 0; i < tempoMap.TempoChangeCount(); i++)
		{
			const auto& tempoChange = tempoMap.GetTempoChangeAt(i);
			const auto& lastTempoChange = tempoMap.GetTempoChangeAt((i == 0) ? i : (i - 1));

			const f32 timelineX = GetTimelinePosition(tempoChange.Tick);
			const f32 screenX = glm::round(timelineX - GetScrollX());
			const auto visiblity = GetTimelineVisibility(screenX);

			if (visiblity == TimelineVisibility::Left)
				continue;
			if (visiblity == TimelineVisibility::Right)
				break;

			const bool shortenText = (zoomLevel < 1.0f);

			char tempoChangeTextBuffer[64];
			sprintf_s(tempoChangeTextBuffer, shortenText ? "%.0f BPM" : "%.2f BPM", tempoChange.Tempo.BeatsPerMinute);

			const bool displaySignature = (i == 0 || tempoChange.Signature != lastTempoChange.Signature);
			if (displaySignature)
			{
				char b[32]; sprintf_s(b, " %d/%d", tempoChange.Signature.Numerator, tempoChange.Signature.Denominator);
				strcat_s(tempoChangeTextBuffer, b);
			}

			const bool displayFlyingTime = (i == 0 || tempoChange.FlyingTime != lastTempoChange.FlyingTime);
			if (displayFlyingTime)
			{
#if 0 // NOTE: Maybe more readable but takes up too much space..?
				char b[32]; sprintf_s(b, " %.2f %%", ToPercent(tempoChange.FlyingTime.Factor));
#else
				char b[32]; sprintf_s(b, " %.3gx", tempoChange.FlyingTime.Factor);
#endif
				strcat_s(tempoChangeTextBuffer, b);
			}

			const vec2 buttonPosition = regions.TempoMap.GetTL() + vec2(screenX + 1.0f, 0.0f);
			const vec2 buttonSize = vec2(Gui::CalcTextSize(tempoChangeTextBuffer).x, tempoMapHeight);

			Gui::SetCursorScreenPos(buttonPosition);

			Gui::PushID(&tempoChange);
			Gui::InvisibleButton("##InvisibleTempoButton", buttonSize);
			Gui::PopID();

			if (Gui::IsItemHovered())
			{
				Gui::WideSetTooltip("Time: %s", TickToTime(tempoChange.Tick).FormatTime().data());

				baseWindowDrawList->AddRect(buttonPosition, buttonPosition + buttonSize, Gui::GetColorU32(ImGuiCol_ChildBg));

				if (Gui::IsMouseClicked(ImGuiMouseButton_Left))
					SetCursorTick(tempoChange.Tick);

				if (Gui::IsMouseClicked(ImGuiMouseButton_Right))
				{
					Gui::OpenPopup(tempoChangePopupName);
					tempoPopupIndex = static_cast<i32>(i);

					// NOTE: Appear above the TempoChange on the TempoMap to not block the timeline content region
					tempoPopupWindowStartPosition = buttonPosition - vec2(0.0f, 8.0f);
					tempoPopupWindowStartPosition.x = Clamp(tempoPopupWindowStartPosition.x,
						regions.TempoMap.Min.x + (tempoPopupWindowStartSize.x * tempoPopupWindowStartPivot.x),
						regions.TempoMap.Max.x - (tempoPopupWindowStartSize.x * (1.0f - tempoPopupWindowStartPivot.x)));
				}
			}

			const u32 tempoColor = GetColor(EditorColor_TempoChange);
			baseWindowDrawList->AddLine(buttonPosition + vec2(-1.0f, -1.0f), buttonPosition + vec2(-1.0f, buttonSize.y - 1.0f), tempoColor);

			// NOTE: Just like with the bar / beat division culling this is far from perfect 
			//		 but at least crudely prevents any unreadable overlapping text until zoomed in close enough
			if (const f32 lastDrawnDistance = (timelineX - lastDrawnTimelineX); lastDrawnDistance >= 0.0f)
				baseWindowDrawList->AddText(Gui::GetFont(), tempoMapFontSize, (buttonPosition + tempoMapFontOffset), tempoColor, tempoChangeTextBuffer);
			lastDrawnTimelineX = timelineX + buttonSize.x;
		}

		if (tempoPopupIndex >= 0)
		{
			Gui::SetNextWindowPos(tempoPopupWindowStartPosition, ImGuiCond_Appearing, tempoPopupWindowStartPivot);
			Gui::SetNextWindowSize(tempoPopupWindowStartSize, ImGuiCond_Always);
		}

		if (Gui::WideBeginPopup(tempoChangePopupName))
		{
			const f32 closeButtonSize = Gui::GetFontSize();
			const ImRect windowBB = { Gui::GetWindowPos(), Gui::GetWindowPos() + Gui::GetWindowSize() };

			Gui::Text("Edit Tempo Change");
			if (Gui::CloseButton(Gui::GetID("TempoChangePopupCloseButton"), vec2(windowBB.Max.x - Gui::GetStyle().FramePadding.x * 2.0f - closeButtonSize, windowBB.Min.y)))
				Gui::CloseCurrentPopup();

			// HACK: Raw old columns API usage here just to quickly disable resizing
			// GuiPropertyRAII::PropertyValueColumns columns;
			Gui::BeginColumns(nullptr, 2, ImGuiOldColumnFlags_NoResize);

			if (tempoPopupIndex >= 0)
			{
				const auto& thisTempoChange = tempoMap.GetTempoChangeAt(tempoPopupIndex);
				const auto* prevTempoChange = (tempoPopupIndex > 0 && tempoPopupIndex < tempoMap.TempoChangeCount()) ? &tempoMap.GetTempoChangeAt(tempoPopupIndex - 1) : nullptr;
				const auto* nextTempoChange = (tempoPopupIndex + 1 < tempoMap.TempoChangeCount()) ? &tempoMap.GetTempoChangeAt(tempoPopupIndex + 1) : nullptr;

				Gui::PushID(&thisTempoChange);
				auto updatedTempoChange = thisTempoChange;

				const bool tempoChangeCanBeMoved = (tempoPopupIndex > 0 && prevTempoChange != nullptr);
				const BeatTick minTickBetweenTempoChanges = GridDivisionTick();
				const BeatTick prevTempoChangeTickMin = (prevTempoChange != nullptr) ? Min((prevTempoChange->Tick + minTickBetweenTempoChanges), thisTempoChange.Tick) : minTickBetweenTempoChanges;
				const BeatTick nextTempoChangeTickMax = (nextTempoChange != nullptr) ? Max((nextTempoChange->Tick - minTickBetweenTempoChanges), thisTempoChange.Tick) : BeatTick::FromTicks(std::numeric_limits<i32>::max());

				// TODO: Also be able to move tempo changes using the mouse similarly to targets (?) although then the "left click to jump" behavior would have to change (?)
				// TODO: Maybe the timeline should automatically be scrolled while moving a tempo change (?) or at least if the tempo change is moved off-screen
				GuiProperty::PropertyFuncValueFunc([&]
				{
					if (tempoChangeCanBeMoved)
					{
						const i32 gridDivisionTicks = GridDivisionTick().Ticks();
						i32 ticksInGridUnits = (thisTempoChange.Tick.Ticks() / gridDivisionTicks);
						i32 ticksMinInGridUnits = (prevTempoChangeTickMin.Ticks() / gridDivisionTicks);
						i32 ticksMaxInGridUnits = (nextTempoChangeTickMax.Ticks() / gridDivisionTicks);

						// NOTE: Scale with grid division so that the apparent drag speed always stays the same 
						//		 and with BPM+Zoom so that the rate of change roughly matches that of the mouse cursor in screenspace
						const f32 prevBPM = ((prevTempoChange != nullptr) ? prevTempoChange->Tempo : thisTempoChange.Tempo).BeatsPerMinute;
						const f32 dragSpeed = (1.0f / (static_cast<f32>(gridDivisionTicks) / static_cast<f32>(BeatTick::TicksPerBeat))) * (1.0f / 37.5f) * (prevBPM / 240.0f) / zoomLevel;

						if (GuiProperty::Detail::DragTextT<i32>("Time and Beat", ticksInGridUnits, dragSpeed, &ticksMinInGridUnits, &ticksMaxInGridUnits, 0.0f))
						{
							const BeatTick ticksInGridUnitsBackToTick = Clamp(BeatTick::FromTicks(ticksInGridUnits * gridDivisionTicks), prevTempoChangeTickMin, nextTempoChangeTickMax);
							if (tempoChangeCanBeMoved && ticksInGridUnitsBackToTick != thisTempoChange.Tick)
								undoManager.Execute<ChangeTempoChangeTick>(*workingChart, tempoPopupIndex, ticksInGridUnitsBackToTick);
						}
					}
					else
					{
						Gui::TextDisabled("Time and Beat");
					}

					return false;
				}, [&]
				{
					Gui::PushItemDisabledAndTextColorIf(!tempoChangeCanBeMoved);
					{
						TimeSpan tempoChangeTime = TickToTime(thisTempoChange.Tick);
						if (Gui::InputFormattedTimeSpan("##Time", &tempoChangeTime, vec2(Gui::GetContentRegionAvail().x * 0.5f, 0.0f)))
						{
							const BeatTick newTempoChangeTick = Clamp(TimeToTick(tempoChangeTime), prevTempoChangeTickMin, nextTempoChangeTickMax);
							if (tempoChangeCanBeMoved && newTempoChangeTick != thisTempoChange.Tick)
								undoManager.Execute<ChangeTempoChangeTick>(*workingChart, tempoPopupIndex, newTempoChangeTick);
						}
					}
					Gui::PopItemDisabledAndTextColorIf(!tempoChangeCanBeMoved);
					Gui::SameLine();
					Gui::PushItemDisabledAndTextColor();
					{
						char textBuffer[64];
						sprintf_s(textBuffer, "%.4g", thisTempoChange.Tick.BeatsFraction());
						Gui::SetNextItemWidth(-1.0f);
						Gui::InputText("##Beat", textBuffer, sizeof(textBuffer), ImGuiInputTextFlags_ReadOnly);
					}
					Gui::PopItemDisabledAndTextColor();

					return false;
				});

				f32 bpm = thisTempoChange.Tempo.BeatsPerMinute;
				if (GuiProperty::Input("Tempo", bpm, 1.0f, vec2(Tempo::MinBPM, Tempo::MaxBPM), "%.2f BPM"))
				{
					updatedTempoChange.Tempo = Clamp(bpm, Tempo::MinBPM, Tempo::MaxBPM);
					undoManager.Execute<UpdateTempoChange>(*workingChart, updatedTempoChange);
				}

				f32 flyingTimeFactor = ToPercent(thisTempoChange.FlyingTime.Factor);
				if (GuiProperty::Input("Flying Time", flyingTimeFactor, 1.0f, ToPercent(vec2(FlyingTimeFactor::Min, FlyingTimeFactor::Max)), "%.2f %%"))
				{
					updatedTempoChange.FlyingTime.Factor = FromPercent(flyingTimeFactor);
					undoManager.Execute<UpdateTempoChange>(*workingChart, updatedTempoChange);
				}

				ivec2 sig = { thisTempoChange.Signature.Numerator, thisTempoChange.Signature.Denominator };
				if (GuiProperty::InputFraction("Time Signature", sig, ivec2(TimeSignature::MinValue, TimeSignature::MaxValue)))
				{
					updatedTempoChange.Signature = TimeSignature(sig[0], sig[1]);
					undoManager.Execute<UpdateTempoChange>(*workingChart, updatedTempoChange);
				}

				GuiProperty::PropertyFuncValueFunc([&]
				{
					return false;
				}, [&]
				{
					Gui::PushStyleColor(ImGuiCol_Text, GetColor(EditorColor_RedText, 0.75f));
					Gui::PushItemDisabledAndTextColorIf(tempoPopupIndex == 0);
					if (Gui::Button("Remove", vec2(Gui::GetContentRegionAvail().x, 0.0f)))
					{
						undoManager.ExecuteEndOfFrame<RemoveTempoChange>(*workingChart, thisTempoChange.Tick);
						Gui::CloseCurrentPopup();
					}
					Gui::PopItemDisabledAndTextColorIf(tempoPopupIndex == 0);
					Gui::PopStyleColor();

					return false;
				});

				Gui::PopID();
			}

			Gui::EndColumns();
			Gui::EndPopup();
		}
		else
		{
			tempoPopupIndex = -1;
		}
	}

	void TargetTimeline::DrawTimelineTargets()
	{
		auto* windowDrawList = Gui::GetWindowDrawList();

		for (const auto& target : workingChart->Targets)
		{
			const auto buttonTime = TickToTime(target.Tick);
			const f32 screenX = glm::round(GetTimelinePosition(buttonTime) - GetScrollX());

			const auto visiblity = GetTimelineVisibility(screenX);
			if (visiblity == TimelineVisibility::Left)
				continue;
			if (visiblity == TimelineVisibility::Right)
				break;

			const size_t buttonIndex = static_cast<size_t>(target.Type);
			const vec2 center = vec2(screenX + regions.Content.GetTL().x, targetYPositions[buttonIndex]);
			const f32 scale = GetTimelineTargetScaleFactor(target, buttonTime) * iconScale;

			const bool tooEarly = (target.Tick < BeatTick::FromBars(1));
			const bool isSelected = target.IsSelected;

			constexpr f32 tooEarlyOpacity = 0.5f, selectionOpacity = 0.75f;
			const f32 edgeFadeOpacity = GetButtonEdgeFadeOpacity(screenX);
			const f32 finalOpacity = tooEarly ? tooEarlyOpacity : isSelected ? (edgeFadeOpacity * selectionOpacity) : edgeFadeOpacity;

			renderHelper.DrawButtonIcon(windowDrawList, target, center, scale, finalOpacity);
			if (target.IsSelected)
				tempSelectedTargetPositionBuffer.push_back(center);
		}

		if (!tempSelectedTargetPositionBuffer.empty())
		{
			const f32 iconHitboxHalfSize = (iconHitboxSize / 2.0f);
			for (const vec2& center : tempSelectedTargetPositionBuffer)
			{
				const vec2 tl = (center - iconHitboxHalfSize);
				const vec2 br = (center + iconHitboxHalfSize);

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
				const f32 delta = static_cast<f32>(timeUntilButton.TotalSeconds() / -buttonAnimationDuration.TotalSeconds());
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
				const f32 delta = static_cast<f32>(buttonAnimation.ElapsedTime.TotalSeconds() / buttonAnimationDuration.TotalSeconds());
				return ImLerp(buttonAnimationScaleStart, buttonAnimationScaleEnd, delta);
			}
		}

		return 1.0f;
	}

	void TargetTimeline::DrawTimelineCursor()
	{
		if (GetIsPlayback())
		{
			const f32 prePlaybackX = glm::round(GetTimelinePosition(chartEditor.GetPlaybackTimeOnPlaybackStart()) - GetScrollX());

			const vec2 start = regions.ContentHeader.GetTL() + vec2(prePlaybackX, 0.0f);
			const vec2 end = regions.Content.GetBL() + vec2(prePlaybackX, 0.0f);

			baseWindowDrawList->AddLine(start, end, GetColor(EditorColor_CursorInner));
		}

		// TODO: Different cursor colors based on grid division (?)
		TimelineBase::DrawTimelineCursor();
	}

	void TargetTimeline::DrawRangeSelection()
	{
		if (!rangeSelection.IsActive)
			return;

		const f32 startScreenX = glm::round(GetTimelinePosition(rangeSelection.StartTick) - GetScrollX()) + (!rangeSelection.HasEnd ? -1.0f : 0.0f);
		const f32 endScreenX = glm::round(GetTimelinePosition(rangeSelection.EndTick) - GetScrollX()) + (!rangeSelection.HasEnd ? +2.0f : 1.0f);

		const vec2 start = vec2(regions.Content.GetTL().x + startScreenX, regions.Content.GetTL().y);
		const vec2 end = vec2(regions.Content.GetTL().x + endScreenX, regions.Content.GetBR().y);

		baseWindowDrawList->AddRectFilled(start, end, GetColor(EditorColor_TimelineSelection, 0.3f));
		baseWindowDrawList->AddRect(start, end, GetColor(EditorColor_TimelineSelectionBorder));
	}

	void TargetTimeline::DrawBoxSelection()
	{
		if (!boxSelection.IsActive || !boxSelection.IsSufficientlyLarge)
			return;

		const f32 startScreenX = glm::round(GetTimelinePosition(boxSelection.StartTick) - GetScrollX());
		const f32 endScreenX = glm::round(GetTimelinePosition(boxSelection.EndTick) - GetScrollX());

		const f32 minY = regions.Content.GetTL().y;
		const f32 maxY = regions.Content.GetBR().y;

		const vec2 start = vec2(regions.Content.GetTL().x + startScreenX, Clamp(boxSelection.StartMouse.y, minY, maxY));
		const vec2 end = vec2(regions.Content.GetTL().x + endScreenX, Clamp(boxSelection.EndMouse.y, minY, maxY));

		baseWindowDrawList->AddRectFilled(start, end, GetColor(EditorColor_TimelineSelection));
		baseWindowDrawList->AddRect(start, end, GetColor(EditorColor_TimelineSelectionBorder));

		// TODO: Move into common selection logic source file (?)
		if (boxSelection.Action != BoxSelectionData::ActionType::Clean)
		{
			constexpr f32 circleRadius = 6.0f;
			constexpr f32 symbolSize = 2.0f;

			const vec2 symbolPos = start;
			const u32 symbolColor = Gui::GetColorU32(ImGuiCol_Text);

			baseWindowDrawList->AddCircleFilled(symbolPos, circleRadius, Gui::GetColorU32(ImGuiCol_ChildBg));
			baseWindowDrawList->AddCircle(symbolPos, circleRadius, GetColor(EditorColor_TimelineSelectionBorder));

			if (boxSelection.Action == BoxSelectionData::ActionType::Add || boxSelection.Action == BoxSelectionData::ActionType::Remove)
				baseWindowDrawList->AddLine(symbolPos - vec2(symbolSize, 0.0f), start + vec2(symbolSize + 1.0f, 0.0f), symbolColor, 1.0f);

			if (boxSelection.Action == BoxSelectionData::ActionType::Add)
				baseWindowDrawList->AddLine(symbolPos - vec2(0.0f, symbolSize), start + vec2(0.0f, symbolSize + 1.0f), symbolColor, 1.0f);
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
			if (Input::IsAnyPressed(GlobalUserData.Input.TargetTimeline_Cut, false))
				ClipboardCutSelection(undoManager, *workingChart);

			if (Input::IsAnyPressed(GlobalUserData.Input.TargetTimeline_Copy, false))
				ClipboardCopySelection(undoManager, *workingChart);

			if (Input::IsAnyPressed(GlobalUserData.Input.TargetTimeline_Paste, false))
				ClipboardPasteSelection(undoManager, *workingChart);
		}
	}

	void TargetTimeline::UpdateCursorKeyboardInput()
	{
		if (Gui::IsWindowHovered() && Gui::IsMouseClicked(ImGuiMouseButton_Right))
			rangeSelection = {};

		if (!Gui::IsWindowFocused())
			return;

		const bool isPlayback = GetIsPlayback();

		const bool useBeatStep = Gui::GetIO().KeyShift || isPlayback;
		const i32 stepDistanceFactor = isPlayback ? 2 : 1;

		if (Input::IsAnyPressed(GlobalUserData.Input.TargetTimeline_MoveCursorLeft, true, Input::ModifierBehavior_Relaxed))
			AdvanceCursorByGridDivisionTick(-1, useBeatStep, stepDistanceFactor);
		if (Input::IsAnyPressed(GlobalUserData.Input.TargetTimeline_MoveCursorRight, true, Input::ModifierBehavior_Relaxed))
			AdvanceCursorByGridDivisionTick(+1, useBeatStep, stepDistanceFactor);

		if (Input::IsAnyPressed(GlobalUserData.Input.TargetTimeline_DecreaseGridPrecision, true))
			SelectNextPresetGridDivision(-1);
		if (Input::IsAnyPressed(GlobalUserData.Input.TargetTimeline_IncreaseGridPrecision, true))
			SelectNextPresetGridDivision(+1);

		for (const auto& presetDivision : PresetBarGridDivisions)
		{
			if (Input::IsAnyPressed(*presetDivision.MultiBinding, false))
				activeBarGridDivision = presetDivision.BarDivision;
		}

		for (const auto& presetDivision : PresetBarChainSlideDivisions)
		{
			if (Input::IsAnyPressed(*presetDivision.MultiBinding, false))
				activeBarChainSlideDivision = presetDivision.BarDivision;
		}

		if (Input::IsAnyPressed(GlobalUserData.Input.TargetTimeline_StartEndRangeSelection, false))
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
			if (Input::IsAnyPressed(GlobalUserData.Input.TargetTimeline_DecreasePlaybackSpeed, true))
				songVoice.SetPlaybackSpeed(Clamp(songVoice.GetPlaybackSpeed() - playbackSpeedStep, playbackSpeedStepMin, playbackSpeedStepMax));
			if (Input::IsAnyPressed(GlobalUserData.Input.TargetTimeline_IncreasePlaybackSpeed, true))
				songVoice.SetPlaybackSpeed(Clamp(songVoice.GetPlaybackSpeed() + playbackSpeedStep, playbackSpeedStepMin, playbackSpeedStepMax));
		}

		if (Input::IsAnyPressed(GlobalUserData.Input.TargetTimeline_ToggleMetronome, false))
		{
			metronomeEnabled ^= true;
			PlayMetronomeToggleSound();
		}

		if (Input::IsAnyPressed(GlobalUserData.Input.TargetTimeline_ToggleTargetHolds, false))
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

			return static_cast<ButtonType>(Clamp(static_cast<i32>(target.Type) + incrementDirection, static_cast<i32>(min), static_cast<i32>(max)));
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
			if (!Gui::IsWindowHovered(ImGuiHoveredFlags_AllowWhenBlockedByPopup) || !regions.Content.Contains(Gui::GetMousePos()))
				return;
		}
		else if (Gui::IsMouseReleased(ImGuiMouseButton_Left))
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

				const vec2 center = vec2(GetTimelinePosition(target.Tick) - GetScrollX() + regions.Content.GetTL().x, targetYPositions[static_cast<size_t>(target.Type)]);
				const auto hitbox = ImRect(center - iconHitboxHalfSize, center + iconHitboxHalfSize);

				if (!hitbox.Contains(Gui::GetMousePos()))
					continue;

				selectionDrag.IsHovering = true;
				selectionDrag.ChangeType = Gui::GetIO().KeyShift;

				if (Gui::IsMouseClicked(ImGuiMouseButton_Left))
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
				const f32 heightPerType = (rowHeight - 2.0f);
				const i32 typeIncrementDirection = Clamp(static_cast<i32>(selectionDrag.VerticalDistanceMovedSoFar / heightPerType), -1, +1);
				selectionDrag.VerticalDistanceMovedSoFar -= (typeIncrementDirection * heightPerType);

				if (typeIncrementDirection != 0)
				{
					const size_t selectedTargetCount = CountSelectedTargets();

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
#if COMFY_DEBUG && 0 // TODO: Allow seperating sync pair targets, thanks to the new ID system
		return false;
#endif

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
#if COMFY_DEBUG && 0 // TODO: ...
		return true;
#endif

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
		if (!Gui::IsWindowHovered() || !regions.Content.Contains(Gui::GetMousePos()))
			return;

		if (Gui::IsMouseClicked(ImGuiMouseButton_Left) && !Gui::GetIO().KeyShift)
		{
			const auto newMouseTick = GetCursorMouseXTick();

			SetCursorTick(newMouseTick);
			PlayCursorButtonSoundsAndAnimation(newMouseTick);
		}
	}

	void TargetTimeline::UpdateInputCursorScrubbing()
	{
		if (Gui::IsMouseReleased(ImGuiMouseButton_Left) || !Gui::IsWindowFocused())
			isCursorScrubbing = false;

		if (isCursorScrubbing)
			Gui::SetActiveID(Gui::GetID(&isCursorScrubbing), Gui::GetCurrentWindow());

		if (Gui::IsWindowHovered(ImGuiHoveredFlags_AllowWhenBlockedByPopup) && regions.ContentHeader.Contains(Gui::GetMousePos()))
		{
			if (Gui::IsMouseClicked(ImGuiMouseButton_Left))
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

		if (Gui::GetIO().KeyCtrl)
			return;

		auto onButtonTypePressed = [&](ButtonType buttonType)
		{
			if (Gui::GetIO().KeyShift && rangeSelection.IsActive && rangeSelection.HasEnd)
				FillInRangeSelectionTargets(undoManager, *workingChart, buttonType);
			else
				PlaceOrRemoveTarget(undoManager, *workingChart, RoundTickToGrid(GetCursorTick()), buttonType);
		};

		if (Input::IsAnyPressed(GlobalUserData.Input.TargetTimeline_PlaceTriangle, false, Input::ModifierBehavior_Relaxed))
			onButtonTypePressed(ButtonType::Triangle);
		if (Input::IsAnyPressed(GlobalUserData.Input.TargetTimeline_PlaceSquare, false, Input::ModifierBehavior_Relaxed))
			onButtonTypePressed(ButtonType::Square);
		if (Input::IsAnyPressed(GlobalUserData.Input.TargetTimeline_PlaceCross, false, Input::ModifierBehavior_Relaxed))
			onButtonTypePressed(ButtonType::Cross);
		if (Input::IsAnyPressed(GlobalUserData.Input.TargetTimeline_PlaceCircle, false, Input::ModifierBehavior_Relaxed))
			onButtonTypePressed(ButtonType::Circle);
		if (Input::IsAnyPressed(GlobalUserData.Input.TargetTimeline_PlaceSlideL, false, Input::ModifierBehavior_Relaxed))
			onButtonTypePressed(ButtonType::SlideL);
		if (Input::IsAnyPressed(GlobalUserData.Input.TargetTimeline_PlaceSlideR, false, Input::ModifierBehavior_Relaxed))
			onButtonTypePressed(ButtonType::SlideR);

		if (Input::IsAnyPressed(GlobalUserData.Input.TargetTimeline_DeleteSelection, false))
			RemoveAllSelectedTargets(undoManager, *workingChart);
	}

	void TargetTimeline::UpdateInputContextMenu()
	{
		constexpr const char* contextMenuID = "TargetTimelineContextMenu";

		if (Gui::IsMouseReleased(ImGuiMouseButton_Right) && Gui::IsWindowHovered(ImGuiHoveredFlags_AllowWhenBlockedByPopup) && regions.Content.Contains(Gui::GetMousePos()))
		{
			if (!Gui::IsAnyItemHovered())
				Gui::OpenPopup(contextMenuID);
		}

		if (Gui::BeginPopup(contextMenuID, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoMove))
		{
			const size_t selectionCount = CountSelectedTargets();

			if (Gui::BeginMenu("Grid Division"))
			{
				for (const auto& presetDivision : PresetBarGridDivisions)
				{
					char nameBuffer[32];
					sprintf_s(nameBuffer, "Set 1 / %d", presetDivision.BarDivision);

					bool alreadySelected = (activeBarGridDivision == presetDivision.BarDivision);;
					if (Gui::MenuItem(nameBuffer, Input::ToString(*presetDivision.MultiBinding).data(), &alreadySelected, !alreadySelected))
						activeBarGridDivision = presetDivision.BarDivision;
				}

				Gui::EndMenu();
			}

			if (Gui::BeginMenu("Chain Slide Division"))
			{
				for (const auto& presetDivision : PresetBarChainSlideDivisions)
				{
					char nameBuffer[32];
					sprintf_s(nameBuffer, "Set 1 / %d", presetDivision.BarDivision);

					bool alreadySelected = (activeBarChainSlideDivision == presetDivision.BarDivision);;
					if (Gui::MenuItem(nameBuffer, Input::ToString(*presetDivision.MultiBinding).data(), &alreadySelected, !alreadySelected))
						activeBarChainSlideDivision = presetDivision.BarDivision;
				}

				Gui::EndMenu();
			}

			if (Gui::BeginMenu("Playback Speed"))
			{
				auto songVoice = chartEditor.GetSongVoice();
				f32 songPlaybackSpeed = songVoice.GetPlaybackSpeed();

				for (const auto presetSpeed : { 1.00f, 0.75f, 0.50f, 0.25f, })
				{
					char b[64]; sprintf_s(b, "Set %3.0f%%", ToPercent(presetSpeed));

					if (Gui::MenuItem(b, nullptr, (presetSpeed == songPlaybackSpeed), (presetSpeed != songPlaybackSpeed)))
						songVoice.SetPlaybackSpeed(presetSpeed);
				}

				if (Gui::BeginMenu("Set Exact"))
				{
					if (auto s = ToPercent(songPlaybackSpeed); Gui::SliderFloat("##PlaybackSpeedSlider", &s, 10.0f, 200.0f, "%.0f%% Playback Speed"))
						songVoice.SetPlaybackSpeed(FromPercent(s));
					Gui::EndMenu();
				}

				Gui::Separator();
				if (Gui::MenuItem("Speed Down", Input::ToString(GlobalUserData.Input.TargetTimeline_DecreasePlaybackSpeed).data(), nullptr, (songPlaybackSpeed > playbackSpeedStepMin)))
					SelectNextPlaybackSpeedLevel(-1);
				if (Gui::MenuItem("Speed Up", Input::ToString(GlobalUserData.Input.TargetTimeline_IncreasePlaybackSpeed).data(), nullptr, (songPlaybackSpeed < playbackSpeedStepMax)))
					SelectNextPlaybackSpeedLevel(+1);

				Gui::EndMenu();
			}

			if (Gui::MenuItem("Metronome Enabled", Input::ToString(GlobalUserData.Input.TargetTimeline_ToggleMetronome).data(), &metronomeEnabled))
				PlayMetronomeToggleSound();

			Gui::Separator();

			if (Gui::MenuItem("Toggle Target Holds", Input::ToString(GlobalUserData.Input.TargetTimeline_ToggleTargetHolds).data(), nullptr, (selectionCount > 0)))
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

			if (Gui::MenuItem("Cut", Input::ToString(GlobalUserData.Input.TargetTimeline_Cut).data(), nullptr, (selectionCount > 0)))
				ClipboardCutSelection(undoManager, *workingChart);
			if (Gui::MenuItem("Copy", Input::ToString(GlobalUserData.Input.TargetTimeline_Copy).data(), nullptr, (selectionCount > 0)))
				ClipboardCopySelection(undoManager, *workingChart);
			if (Gui::MenuItem("Paste", Input::ToString(GlobalUserData.Input.TargetTimeline_Paste).data(), nullptr, true))
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

			if (Gui::MenuItem("Remove Targets", Input::ToString(GlobalUserData.Input.TargetTimeline_DeleteSelection).data(), nullptr, (selectionCount > 0)))
				RemoveAllSelectedTargets(undoManager, *workingChart, selectionCount);

			Gui::EndPopup();
		}
	}

	void TargetTimeline::UpdateInputBoxSelection()
	{
		constexpr ImGuiMouseButton boxSelectionButton = ImGuiMouseButton_Right;

		if (Gui::IsWindowHovered(ImGuiHoveredFlags_AllowWhenBlockedByPopup) && regions.Content.Contains(Gui::GetMousePos()))
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
				const f32 minY = Min(boxSelection.StartMouse.y, boxSelection.EndMouse.y);
				const f32 maxY = Max(boxSelection.StartMouse.y, boxSelection.EndMouse.y);

				const auto minTick = Min(boxSelection.StartTick, boxSelection.EndTick);
				const auto maxTick = Max(boxSelection.StartTick, boxSelection.EndTick);

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
		const size_t selectedTargetCount = CountSelectedTargets();
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
		const size_t selectionCount = CountSelectedTargets();
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
		const size_t selectionCount = CountSelectedTargets();
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
		const auto startTick = RoundTickToGrid(Min(rangeSelection.StartTick, rangeSelection.EndTick));
		const auto endTick = RoundTickToGrid(Max(rangeSelection.StartTick, rangeSelection.EndTick));

		// TODO: Come up with better chain placement controls, maybe shift + direction to place chain start, move cursor then press again to "confirm" end placement (?)
		const bool placeChain = IsSlideButtonType(type);

		const auto divisionTick = placeChain ? ChainSlideDivisionTick() : GridDivisionTick();
		const i32 targetCount = ((endTick - startTick).Ticks() / divisionTick.Ticks()) + 1;

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

		const size_t buttonIndex = static_cast<size_t>(type);
		buttonAnimations[buttonIndex].Tick = tick;
		buttonAnimations[buttonIndex].ElapsedTime = TimeSpan::Zero();
	}

	void TargetTimeline::RemoveAllSelectedTargets(Undo::UndoManager& undoManager, Chart& chart, std::optional<size_t> preCalculatedSelectionCount)
	{
		const size_t selectionCount = [&] { return (preCalculatedSelectionCount.has_value()) ? preCalculatedSelectionCount.value() : CountSelectedTargets(); }();
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

	void TargetTimeline::PlayMetronomeToggleSound()
	{
		Audio::AudioEngine::GetInstance().EnsureStreamRunning();
		metronome.PlayTickSound(TimeSpan::Zero(), false);
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
		SetScrollTargetX(0.0f);
		zoomLevel = 2.0f;
	}

	TimelineMetronome& TargetTimeline::GetMetronome()
	{
		return metronome;
	}

	i32 TargetTimeline::FindGridDivisionPresetIndex() const
	{
		for (i32 i = 0; i < static_cast<i32>(PresetBarGridDivisions.size()); i++)
		{
			if (PresetBarGridDivisions[i].BarDivision == activeBarGridDivision)
				return i;
		}

		return -1;
	}

	void TargetTimeline::SelectNextPresetGridDivision(i32 direction)
	{
		const i32 index = FindGridDivisionPresetIndex();
		const i32 nextIndex = Clamp(index + direction, 0, static_cast<i32>(PresetBarGridDivisions.size()) - 1);

		activeBarGridDivision = PresetBarGridDivisions[nextIndex].BarDivision;
	}

	void TargetTimeline::SelectNextPlaybackSpeedLevel(i32 direction)
	{
		auto songVoice = chartEditor.GetSongVoice();
		const f32 songPlaybackSpeed = songVoice.GetPlaybackSpeed();

		if (direction < 0)
			songVoice.SetPlaybackSpeed(Clamp(songPlaybackSpeed - playbackSpeedStep, playbackSpeedStepMin, playbackSpeedStepMax));
		else
			songVoice.SetPlaybackSpeed(Clamp(songPlaybackSpeed + playbackSpeedStep, playbackSpeedStepMin, playbackSpeedStepMax));
	}

	void TargetTimeline::AdvanceCursorByGridDivisionTick(i32 direction, bool beatStep, i32 distanceFactor)
	{
		const auto beatIncrement = BeatTick::FromBeats(1);
		const auto gridIncrement = GridDivisionTick();

		const auto stepDistance = (beatStep ? Max(beatIncrement, gridIncrement) : gridIncrement) * distanceFactor;

		const auto newCursorTick = RoundTickToGrid(GetCursorTick()) + (stepDistance * direction);
		const auto clampedCursorTick = Max(newCursorTick, BeatTick::Zero());

		const f32 preCursorX = GetCursorTimelinePosition();

		SetCursorTick(clampedCursorTick);
		PlayCursorButtonSoundsAndAnimation(clampedCursorTick);

		// NOTE: Keep same relative cursor screen position though might only wanna scroll if the cursor is about to go off-screen (?)
		SetScrollTargetX(GetScrollTargetX() + (GetCursorTimelinePosition() - preCursorX));
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

		const auto scrollTickIncrement = (io.KeyShift ? Max(beatIncrement, gridIncrement) : gridIncrement) * static_cast<i32>(io.MouseWheel);
		const auto newCursorTick = Max(BeatTick::Zero(), GetCursorTick() + scrollTickIncrement);

		if (const bool seekThroughSong = GetIsPlayback(); seekThroughSong)
		{
			const f32 preCursorX = GetCursorTimelinePosition();

			// NOTE: Pause and resume to reset the on-playback start-time
			chartEditor.PausePlayback();
			{
				SetCursorTick(newCursorTick);
			}
			chartEditor.ResumePlayback();

			// NOTE: Keep the cursor at the same relative screen position to prevent potential disorientation
			SetScrollTargetX(GetScrollTargetX() + (GetCursorTimelinePosition() - preCursorX));
		}
		else if (const bool seekingScroll = false; seekingScroll)
		{
			// DEBUG: Neat idea but in practice very disorientating
			const f32 preCursorX = GetCursorTimelinePosition();
			{
				SetCursorTick(newCursorTick);
				PlayCursorButtonSoundsAndAnimation(newCursorTick);
			}
			SetScrollTargetX(GetScrollTargetX() + (GetCursorTimelinePosition() - preCursorX));
		}
		else
		{
			TimelineBase::OnTimelineBaseScroll();
		}
	}
}
