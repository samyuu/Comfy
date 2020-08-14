#include "TargetTimeline.h"
#include "Editor/Chart/ChartEditor.h"
#include "Editor/Chart/ChartCommands.h"
#include "Editor/Chart/SortedTempoMap.h"
#include "Time/TimeSpan.h"
#include "ImGui/Extensions/PropertyEditor.h"
#include <FontIcons.h>

namespace Comfy::Studio::Editor
{
	TargetTimeline::TargetTimeline(ChartEditor& parent, Undo::UndoManager& undoManager) : chartEditor(parent), undoManager(undoManager)
	{
		scrollSpeed = 2.5f;
		scrollSpeedFast = 5.5f;

		workingChart = chartEditor.GetChart();

		infoColumnWidth = 240.0f;
	}

	TimelineTick TargetTimeline::GridDivisionTick() const
	{
		return TimelineTick::FromTicks((TimelineTick::TicksPerBeat * 4) / activeGridDivision);
	}

	TimelineTick TargetTimeline::FloorTickToGrid(TimelineTick tick) const
	{
		const auto gridTicks = GridDivisionTick().TotalTicks();
		return TimelineTick::FromTicks(static_cast<i32>(glm::floor(tick.TotalTicks() / static_cast<f32>(gridTicks)) * gridTicks));
	}

	TimelineTick TargetTimeline::RoundTickToGrid(TimelineTick tick) const
	{
		const auto gridTicks = GridDivisionTick().TotalTicks();
		return TimelineTick::FromTicks(static_cast<i32>(glm::round(tick.TotalTicks() / static_cast<f32>(gridTicks)) * gridTicks));
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
		return workingChart->GetTimelineMap().GetTickAt(time);
	}

	TimelineTick TargetTimeline::TimeToTickFixedTempo(TimeSpan time, Tempo tempo) const
	{
		return workingChart->GetTimelineMap().GetTickAtFixedTempo(time, tempo);
	}

	TimelineTick TargetTimeline::GetTimelineTick(f32 position) const
	{
		return TimeToTick(GetTimelineTime(position));
	}

	TimeSpan TargetTimeline::TickToTime(TimelineTick tick) const
	{
		return workingChart->GetTimelineMap().GetTimeAt(tick);
	}

	TimeSpan TargetTimeline::GetTimelineTime(f32 position) const
	{
		return TimelineBase::GetTimelineTime(position);
	}

	TimelineTick TargetTimeline::GetCursorTick() const
	{
		return TimeToTick(GetCursorTime());
	}

	TimelineTick TargetTimeline::GetCursorMouseXTick() const
	{
		const auto tickAtMousePosition = GetTimelineTick(ScreenToTimelinePosition(Gui::GetMousePos().x));
		const auto gridAdjusted = FloorTickToGrid(tickAtMousePosition);

		// NOTE: There should never be a need to click before the start of the timeline
		const auto clamped = std::max(TimelineTick::Zero(), gridAdjusted);

		return clamped;
	}

	int TargetTimeline::FindGridDivisionPresetIndex() const
	{
		for (int i = 0; i < presetGridDivisions.size(); i++)
		{
			if (presetGridDivisions[i] == activeGridDivision)
				return i;
		}

		return -1;
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

	void TargetTimeline::OnInitialize()
	{
		buttonIcons = std::make_unique<TimelineButtonIcons>();
		UpdateTimelineMapTimes();
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
		thisFrameStartOffset = workingChart->GetStartOffset();

		// NOTE: Cancel out the cursor being moved by a change of offset because this just feels more intuitive to use
		//		 and always automatically applying the start offset to the playback time makes other calculations easier
		if (thisFrameStartOffset != lastFrameStartOffset)
			SetCursorTime(GetCursorTime() + (thisFrameStartOffset - lastFrameStartOffset));
	}

	void TargetTimeline::UpdatePlaybackButtonSounds()
	{
		lastFrameButtonSoundCursorTime = thisFrameButtonSoundCursorTime;
		thisFrameButtonSoundCursorTime = GetCursorTime();

		// TODO: Implement metronome the same way, refactor ButtonSoundController to support any user controlled voices generically and rename to SoundVoicePool (?)

		// NOTE: Play back button sounds in the future with a negative offset to achieve sample perfect accuracy
		for (const auto& target : workingChart->GetTargets())
		{
			// DEBUG: Stacked button sounds should be handled by the button sound controller automatically but it seems there might be a bug here somewhere... (?)
			//		  Doing an additional sync check here has the advantage of offloading audio engine work though special care needs to be taken for slide and normal buttons
			if (target.Flags.IsSync && target.Flags.IndexWithinSyncPair > 0)
				continue;

			const auto buttonTime = workingChart->GetTimelineMap().GetTimeAt(target.Tick);
			const auto offsetButtonTime = buttonTime - buttonSoundFutureOffset;

			if (offsetButtonTime >= lastFrameButtonSoundCursorTime && offsetButtonTime <= thisFrameButtonSoundCursorTime)
			{
				const auto startTime = (thisFrameButtonSoundCursorTime - buttonTime);
				const auto externalClock = buttonTime;

				// NOTE: Don't wanna cause any audio cutoffs. If this happens the future threshold is either set too low for the current frame time
				//		 or playback was started on top of an existing target
				if (startTime >= TimeSpan::Zero())
					buttonSoundController.PlayButtonSound(TimeSpan::Zero(), externalClock);
				else
					buttonSoundController.PlayButtonSound(startTime, externalClock);
			}
		}
	}

	void TargetTimeline::UpdateTimelineMapTimes()
	{
		workingChart->GetTimelineMap().CalculateMapTimes(workingChart->GetTempoMap());
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

	void TargetTimeline::OnSongLoaded()
	{
		Audio::AudioEngine::GetInstance().EnsureStreamRunning();

		if (const auto sampleProvider = Audio::AudioEngine::GetInstance().GetSharedSource(chartEditor.GetSongSource()); sampleProvider != nullptr)
			songWaveform.SetSource(sampleProvider);
		else
			songWaveform.Clear();

		waveformUpdatePending = true;
	}

	void TargetTimeline::OnDrawTimelineHeaderWidgets()
	{
		return;
	}

	void TargetTimeline::OnDrawTimelineInfoColumnHeader()
	{
		TimelineBase::OnDrawTimelineInfoColumnHeader();

		static constexpr const char* settingsPopupName = "TimelineSettingsPopup::ChartTimeline";

		Gui::PushStyleVar(ImGuiStyleVar_ItemSpacing, vec2(0.0f, 0.0f));
		Gui::PushStyleVar(ImGuiStyleVar_FramePadding, vec2(8.0f, 8.0f));

		constexpr vec4 transparent = vec4(0.0f);
		Gui::PushStyleColor(ImGuiCol_Button, transparent);
		{
			const bool isFirstFrame = (cursorTime <= TimeSpan::Zero());
			const bool isLastFrame = (cursorTime >= workingChart->GetDuration());
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
					SetCursorTime(std::clamp(TickToTime(RoundTickToGrid(GetCursorTick()) - GridDivisionTick()), TimeSpan::Zero(), workingChart->GetDuration()));
				}
				Gui::PopItemDisabledAndTextColorIf(isFirstFrame);
				Gui::SetWideItemTooltip("Go to previous beat");
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
					SetCursorTime(std::clamp(TickToTime(RoundTickToGrid(GetCursorTick()) + GridDivisionTick()), TimeSpan::Zero(), workingChart->GetDuration()));
				}
				Gui::PopItemDisabledAndTextColorIf(isLastFrame);
				Gui::SetWideItemTooltip("Go to next beat");
			}

			// NOTE: Last frame button
			{
				Gui::SameLine();
				Gui::PushItemDisabledAndTextColorIf(isLastFrame);
				if (Gui::Button(ICON_FA_FAST_FORWARD))
				{
					SetCursorTime(workingChart->GetDuration());
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
		for (size_t row = 0; row < EnumCount<ButtonType>(); row++)
		{
			const auto y = row * rowHeight;
			const auto start = vec2(0.0f, y) + infoColumnRegion.GetTL();
			const auto end = vec2(infoColumnWidth, y + rowHeight) + infoColumnRegion.GetTL();

			const auto center = vec2(start + end) / 2.0f;
			targetYPositions[row] = center.y;

			const auto target = TimelineTarget(TimelineTick::Zero(), static_cast<ButtonType>(row));
			buttonIcons->DrawButtonIcon(drawList, target, center, iconScale);

			drawList->AddLine(vec2(start.x, end.y), end, Gui::GetColorU32(ImGuiCol_Border));
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

		const i32 songDurationTicks = TimeToTick(workingChart->GetDuration()).TotalTicks();
		const i32 gridTickStep = GridDivisionTick().TotalTicks();

		const auto scrollX = GetScrollX();

		constexpr auto beatSpacingThreshold = 10.0f;
		constexpr auto barSpacingThreshold = 74.0f;

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

		// TODO: Implement more efficiently for multiple tempo changes by looping over the tempo changes instead (?)
		auto& tempoMap = workingChart->GetTempoMap();

		auto lastBarTimelineX = -barSpacingThreshold;
		for (i32 ticks = 0, barIndex = 0; ticks < songDurationTicks; ticks += TimelineTick::TicksPerBeat * tempoMap.FindTempoChangeAtTick(TimelineTick(ticks)).Signature.Numerator, barIndex++)
		{
			const auto timelineX = GetTimelinePosition(TimelineTick(ticks));

			if (const auto lastDrawnDistance = (timelineX - lastBarTimelineX); lastDrawnDistance < barSpacingThreshold)
				continue;
			lastBarTimelineX = timelineX;

			const auto screenX = glm::round(timelineX - scrollX);
			const auto visiblity = GetTimelineVisibility(screenX);

			if (visiblity == TimelineVisibility::Left)
				continue;
			if (visiblity == TimelineVisibility::Right)
				break;

			char buffer[16];
			sprintf_s(buffer, sizeof(buffer), "%d", barIndex);

			const auto start = timelineContentRegion.GetTL() + vec2(screenX, -(timelineHeaderHeight * 0.85f));
			const auto end = timelineContentRegion.GetBL() + vec2(screenX, 0.0f);
			baseDrawList->AddLine(start, end, barColor);
			baseDrawList->AddText(start + vec2(3.0f, -1.0f), barColor, buffer);
		}
	}

	void TargetTimeline::OnDrawTimlineBackground()
	{
		DrawWaveform();
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
			constexpr auto dragSpeed = 16.0f;
			auto cursorDragTicks = static_cast<f32>(GetCursorTick().TotalTicks());

			if (Gui::ComfyDragText("TimeDragText::AetTimeline", cursorTime.FormatTime().data(), &cursorDragTicks, dragSpeed, 0.0f, 0.0f, timeDragTextWidth))
				SetCursorTime(TickToTime(RoundTickToGrid(TimelineTick::FromTicks(static_cast<int>(cursorDragTicks)))));
		}

		{
			char buttonNameBuffer[64];
			sprintf_s(buttonNameBuffer, "Grid: 1 / %d", activeGridDivision);

			Gui::SameLine();
			if (Gui::Button(buttonNameBuffer, vec2(gridDivisionButtonWidth, timelineScrollbarSize.y)))
				SelectNextPresetGridDivision(+1);
			if (Gui::IsItemClicked(1))
				SelectNextPresetGridDivision(-1);
		}

		Gui::PopStyleVar(1);
	}

	void TargetTimeline::DrawWaveform()
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

		const auto scrollXStartOffset = GetScrollX() + GetTimelinePosition(workingChart->GetStartOffset());

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
			const auto timelinePixel = std::min(static_cast<i64>(screenPixel + scrollXStartOffset), static_cast<i64>(waveformPixelCount - 1));
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
		constexpr auto tempoChangePopupName = "##TempoChangePopup";

		const auto& tempoMap = workingChart->GetTempoMap();
		for (size_t i = 0; i < tempoMap.TempoChangeCount(); i++)
		{
			const auto& tempoChange = tempoMap.GetTempoChangeAt(i);

			const auto screenX = glm::round(GetTimelinePosition(tempoChange.Tick) - GetScrollX());
			const auto visiblity = GetTimelineVisibility(screenX);

			if (visiblity == TimelineVisibility::Left)
				continue;
			if (visiblity == TimelineVisibility::Right)
				break;

			const bool signatureChanged = (i == 0 || tempoChange.Signature != tempoMap.GetTempoChangeAt(i - 1).Signature);

			char tempoBuffer[64];
			sprintf_s(tempoBuffer, sizeof(tempoBuffer),
				signatureChanged ? "%.2f BPM %d/%d" : "%.2f BPM",
				tempoChange.Tempo.BeatsPerMinute,
				tempoChange.Signature.Numerator, tempoChange.Signature.Denominator);

			const auto buttonPosition = tempoMapRegion.GetTL() + vec2(screenX + 1.0f, 0.0f);
			const auto buttonSize = vec2(Gui::CalcTextSize(tempoBuffer).x, tempoMapHeight);

			Gui::SetCursorScreenPos(buttonPosition);

			Gui::PushID(&tempoChange);
			Gui::InvisibleButton("##InvisibleTempoButton", buttonSize);
			Gui::PopID();

			// TODO: Prevent overlapping tempo changes
			if (Gui::IsWindowFocused() && Gui::IsItemHovered())
			{
				Gui::WideSetTooltip("Time: %s", TickToTime(tempoChange.Tick).FormatTime().data());

				baseDrawList->AddRect(buttonPosition, buttonPosition + buttonSize, Gui::GetColorU32(ImGuiCol_ChildBg));

				if (Gui::IsMouseClicked(0))
					SetCursorTime(TickToTime(tempoChange.Tick));

				if (Gui::IsMouseClicked(1))
				{
					Gui::OpenPopup(tempoChangePopupName);
					tempoPopupIndex = static_cast<int>(i);
				}
			}

			const auto tempoFgColor = 0xFF1DBFB2;

			baseDrawList->AddLine(buttonPosition + vec2(-1.0f, -1.0f), buttonPosition + vec2(-1.0f, buttonSize.y - 1.0f), tempoFgColor);
			baseDrawList->AddText(Gui::GetFont(), tempoMapFontSize, buttonPosition + tempoMapFontOffset, tempoFgColor, tempoBuffer);
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
					Gui::TextDisabled("%s (Ticks: %d)", TickToTime(tempoChange.Tick).FormatTime().data(), tempoChange.Tick.TotalTicks());
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
					if (Gui::Button("Remove##TempoChange", vec2(Gui::GetContentRegionAvailWidth(), 0.0f)))
					{
						undoManager.ExecuteEndOfFrame<RemoveTempoChange>(*workingChart, tempoChange.Tick);
						Gui::CloseCurrentPopup();
					}
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

		for (const auto& target : workingChart->GetTargets())
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

			buttonIcons->DrawButtonIcon(windowDrawList, target, center, scale, GetButtonEdgeFadeOpacity(screenX));
		}
	}

	f32 TargetTimeline::GetTimelineTargetScaleFactor(const TimelineTarget& target, TimeSpan buttonTime) const
	{
		if (GetIsPlayback())
		{
			const auto cursorTime = GetCursorTime();
			const auto timeUntilButton = buttonTime - cursorTime;

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

		TimelineBase::DrawTimelineCursor();
	}

	void TargetTimeline::DrawTimeSelection()
	{
#if 1 // DEBUG: Test out selection box
		{
			constexpr int selectionBoxButton = 1;
			static ImRect dragRect;

			if (Gui::IsMouseClicked(selectionBoxButton) && Gui::IsWindowFocused() && !Gui::IsAnyItemHovered())
				dragRect.Min = Gui::GetMousePos();
			if (Gui::IsMouseReleased(selectionBoxButton))
				dragRect.Min = dragRect.Max = vec2(0.0f, 0.0f);

			if (!Gui::IsAnyItemHovered() && Gui::IsMouseDragging(selectionBoxButton) && dragRect.Min.x != 0)
			{
				dragRect.Max = Gui::GetMousePos();
				baseDrawList->AddRectFilled(dragRect.GetTL(), dragRect.GetBR(), GetColor(EditorColor_Selection));
			}
		}
#endif

		if (!timeSelectionActive)
			return;

		const auto scrollX = GetScrollX();

		const auto startScreenX = GetTimelinePosition(timeSelectionStart) - scrollX;
		const auto endScreenX = GetTimelinePosition(timeSelectionEnd) - scrollX;

		const auto start = timelineContentRegion.GetTL() + vec2(startScreenX, 0.0f);
		const auto end = timelineContentRegion.GetBL() + vec2(endScreenX, 0.0f);

		baseDrawList->AddRectFilled(start, end, GetColor(EditorColor_Selection));
		// Gui::SetMouseCursor(ImGuiMouseCursor_Hand);
	}

	void TargetTimeline::OnUpdateInput()
	{
		UpdateInputCursorClick();
		UpdateInputTargetPlacement();
	}

	void TargetTimeline::OnDrawTimelineContents()
	{
		DrawTimelineTargets();
		DrawTimeSelection();
	}

	void TargetTimeline::UpdateInputCursorClick()
	{
		if (!Gui::IsWindowFocused() || !timelineContentRegion.Contains(Gui::GetMousePos()))
			return;

		const auto& io = Gui::GetIO();

		if (Gui::IsMouseClicked(0) && !io.KeyShift)
		{
			const auto newMouseTick = GetCursorMouseXTick();
			const auto newMouseTime = TickToTime(newMouseTick);

			SetCursorTime(newMouseTime);

			for (const auto& target : workingChart->GetTargets())
			{
				if (target.Tick != newMouseTick)
					continue;

				// NOTE: During playback the sound will handled automatically already
				if (!GetIsPlayback())
					buttonSoundController.PlayButtonSound();

				const auto buttonIndex = static_cast<size_t>(target.Type);
				buttonAnimations[buttonIndex].Tick = target.Tick;
				buttonAnimations[buttonIndex].ElapsedTime = TimeSpan::Zero();
			}
		}

#if 0 // DEBUG: Cursor Mouse Drag:
		if (!GetIsPlayback())
		{
			if (Gui::IsMouseClicked(0))
			{
				if (io.KeyShift) // && timeSelectionActive)
				{
					timeSelectionEnd = GetCursorMouseXTick();
				}
				else
				{
					timeSelectionActive = false;
					timeSelectionStart = GetCursorMouseXTick();
				}
			}
			if (Gui::IsMouseDragging(0))
			{
				if (glm::abs(timeSelectionStart.TotalTicks() - timeSelectionEnd.TotalTicks()) > (GridDivisionTick().TotalTicks() * 2))
					timeSelectionActive = true;

				timeSelectionEnd = GetCursorMouseXTick();
			}
		}
#endif
	}

	void TargetTimeline::UpdateInputTargetPlacement()
	{
		const auto& io = Gui::GetIO();

		if (!Gui::IsWindowFocused())
			return;

		// NOTE: Mouse X buttons, increase / decrease grid division
		if (Gui::IsMouseClicked(3)) SelectNextPresetGridDivision(-1);
		if (Gui::IsMouseClicked(4)) SelectNextPresetGridDivision(+1);

		const auto deltaTime = TimeSpan::FromSeconds(io.DeltaTime);
		for (auto& animationData : buttonAnimations)
			animationData.ElapsedTime += deltaTime;

		for (auto[buttonType, keyCode] : targetPlacementInputKeyMappings)
		{
			if (Gui::IsKeyPressed(keyCode, false))
				PlaceOrRemoveTarget(RoundTickToGrid(GetCursorTick()), buttonType);
		}
	}

	void TargetTimeline::PlaceOrRemoveTarget(TimelineTick tick, ButtonType type)
	{
		const auto existingTargetIndex = workingChart->GetTargets().FindIndex(tick, type);

		// NOTE: Double hit sound if a target gets placed in front of the cursor position.
		//		 Keeping it this way could make it easier to notice when real time targets are not placed accurately to the beat (?)
		if (!buttonSoundOnSuccessfulPlacementOnly)
			buttonSoundController.PlayButtonSound();

		if (existingTargetIndex > -1)
		{
			if (!GetIsPlayback())
			{
				if (buttonSoundOnSuccessfulPlacementOnly)
					buttonSoundController.PlayButtonSound();

				undoManager.Execute<RemoveTarget>(*workingChart, tick, type);
			}
		}
		else
		{
			if (buttonSoundOnSuccessfulPlacementOnly)
				buttonSoundController.PlayButtonSound();

			undoManager.Execute<AddTarget>(*workingChart, tick, type);
		}

		const auto buttonIndex = static_cast<size_t>(type);
		buttonAnimations[buttonIndex].Tick = tick;
		buttonAnimations[buttonIndex].ElapsedTime = TimeSpan::Zero();
	}

	void TargetTimeline::SelectNextPresetGridDivision(int direction)
	{
		const auto index = FindGridDivisionPresetIndex();
		const auto nextIndex = std::clamp(index + direction, 0, static_cast<int>(presetGridDivisions.size()) - 1);

		activeGridDivision = presetGridDivisions[nextIndex];
	}

	TimeSpan TargetTimeline::GetCursorTime() const
	{
		return chartEditor.GetPlaybackTimeAsync();
	}

	void TargetTimeline::SetCursorTime(TimeSpan value)
	{
		chartEditor.SetPlaybackTime(value);
		PlaybackStateChangeSyncButtonSoundCursorTime(value);
	}

	bool TargetTimeline::GetIsPlayback() const
	{
		return chartEditor.GetIsPlayback();
	}

	void TargetTimeline::PausePlayback()
	{
		chartEditor.PausePlayback();
		PlaybackStateChangeSyncButtonSoundCursorTime(GetCursorTime());
	}

	void TargetTimeline::ResumePlayback()
	{
		chartEditor.ResumePlayback();
		PlaybackStateChangeSyncButtonSoundCursorTime(GetCursorTime());
	}

	void TargetTimeline::StopPlayback()
	{
		chartEditor.StopPlayback();
		PlaybackStateChangeSyncButtonSoundCursorTime(GetCursorTime());
	}

	void TargetTimeline::PlaybackStateChangeSyncButtonSoundCursorTime(TimeSpan newCursorTime)
	{
		lastFrameButtonSoundCursorTime = thisFrameButtonSoundCursorTime = (newCursorTime - buttonSoundFutureOffset);
		buttonSoundController.PauseAllNegativeVoices();
	}

	f32 TargetTimeline::GetTimelineSize() const
	{
		return GetTimelinePosition(workingChart->GetDuration());
	}

	void TargetTimeline::OnTimelineBaseScroll()
	{
		const auto& io = Gui::GetIO();

		if (GetIsPlayback()) // NOTE: Seek through song
		{
			// NOTE: Pause and resume playback to reset the on playback start time
			chartEditor.PausePlayback();
			{
				const auto scrollTimeIncrement = TimeSpan((io.KeyShift ? 1.0f : 0.5f) * io.MouseWheel);
				SetCursorTime(chartEditor.GetPlaybackTimeAsync() + scrollTimeIncrement);

				if (chartEditor.GetPlaybackTimeAsync() < TimeSpan::Zero())
					SetCursorTime(TimeSpan::Zero());
			}
			chartEditor.ResumePlayback();
			CenterCursor();
		}
		else
		{
			TimelineBase::OnTimelineBaseScroll();
		}
	}
}
