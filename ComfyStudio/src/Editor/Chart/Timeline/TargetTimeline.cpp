#include "TargetTimeline.h"
#include "Editor/Chart/ChartEditor.h"
#include "Editor/Chart/SortedTempoMap.h"
#include "Time/TimeSpan.h"
#include "System/ComfyData.h"
#include <FontIcons.h>

namespace Comfy::Studio::Editor
{
	TargetTimeline::TargetTimeline(ChartEditor& parent) : chartEditor(parent)
	{
		scrollSpeed = 2.5f;
		scrollSpeedFast = 5.5f;

		workingChart = chartEditor.GetChart();

		infoColumnWidth = 240.0f;
	}

	TimelineTick TargetTimeline::GetGridTick() const
	{
		return TimelineTick::FromTicks((TimelineTick::TicksPerBeat * 4) / gridDivision);
	}

	TimelineTick TargetTimeline::FloorTickToGrid(TimelineTick tick) const
	{
		const auto gridTicks = GetGridTick().TotalTicks();
		return TimelineTick::FromTicks(static_cast<i32>(glm::floor(tick.TotalTicks() / static_cast<f32>(gridTicks)) * gridTicks));
	}

	TimelineTick TargetTimeline::RoundTickToGrid(TimelineTick tick) const
	{
		const auto gridTicks = GetGridTick().TotalTicks();
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
		return FloorTickToGrid(GetTimelineTick(ScreenToTimelinePosition(Gui::GetMousePos().x)));
	}

	int TargetTimeline::GetGridDivisionIndex() const
	{
		for (int i = 0; i < gridDivisions.size(); i++)
		{
			if (gridDivisions[i] == gridDivision)
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

	size_t TargetTimeline::GetTargetButtonIconIndex(const TimelineTarget& target) const
	{
		auto index = static_cast<size_t>(target.Type);

		if (target.Flags.IsChain && (target.Type == ButtonType::SlideL || target.Type == ButtonType::SlideR))
			index += 2;

		if (target.Flags.IsSync)
			return index;

		return index + 8;
	}

	void TargetTimeline::DrawButtonIcon(ImDrawList* drawList, const TimelineTarget& target, vec2 position, f32 scale, f32 transparency)
	{
		const f32 width = buttonIconWidth * scale;
		const f32 height = buttonIconWidth * scale;

		position.x = glm::round(position.x - (width * 0.5f));
		position.y = glm::round(position.y - (height * 0.5f));

		const auto bottomRight = vec2(position.x + width, position.y + height);
		const auto textureCoordinates = buttonIconsTextureCoordinates[GetTargetButtonIconIndex(target)];

		const ImU32 color = IM_COL32(0xFF, 0xFF, 0xFF, 0xFF * transparency);
		if (buttonIconsTexture != nullptr)
			drawList->AddImage(*buttonIconsTexture, position, bottomRight, textureCoordinates.GetBL(), textureCoordinates.GetTR(), color);

		if (target.Flags.IsHold)
		{
			// TODO: draw tgt_txt
		}
	}

	void TargetTimeline::OnInitialize()
	{
		InitializeButtonIcons();
		UpdateTimelineMap();
	}

	void TargetTimeline::OnUpdate()
	{
		if (GetIsPlayback())
			UpdatePlaybackButtonSounds();
	}

	void TargetTimeline::UpdatePlaybackButtonSounds()
	{
		lastButtonSoundCursorTime = buttonSoundCursorTime;
		buttonSoundCursorTime = GetCursorTime();

		// NOTE: Do a simple check for all targets that have been passed between this and the last frame
		if (buttonSoundFutureOffset <= TimeSpan::Zero())
		{
			for (const auto& target : workingChart->GetTargets())
			{
				const auto buttonTime = workingChart->GetTimelineMap().GetTimeAt(target.Tick);
				if (buttonTime >= lastButtonSoundCursorTime && buttonTime <= buttonSoundCursorTime)
					buttonSoundController.PlayButtonSound();
			}
		}
		else // NOTE: Play back button sounds in the future with a negative offset to achieve sample perfect accuracy
		{
			for (const auto& target : workingChart->GetTargets())
			{
				const auto buttonTime = workingChart->GetTimelineMap().GetTimeAt(target.Tick);
				const auto offsetButtonTime = buttonTime - buttonSoundFutureOffset;

				if (offsetButtonTime >= lastButtonSoundCursorTime && offsetButtonTime <= buttonSoundCursorTime)
				{
					const auto startTime = buttonSoundCursorTime - buttonTime;
					const auto externalClock = buttonTime;

					// NOTE: Don't wanna cause any audio cutoffs. If this happens the future threshold is either set too low for the current frame time
					//		 or playback was started on top of an existing target.
					if (startTime >= TimeSpan::Zero())
						buttonSoundController.PlayButtonSound(TimeSpan::Zero(), externalClock);
					else
						buttonSoundController.PlayButtonSound(startTime, externalClock);
				}
			}
		}
	}

	void TargetTimeline::UpdateTimelineMap()
	{
		workingChart->GetTimelineMap().CalculateMapTimes(workingChart->GetTempoMap());
	}

	void TargetTimeline::InitializeButtonIcons()
	{
		// NOTE:
		// sankaku		| shikaku		| batsu		 | maru		 | slide_l		| slide_r	   | slide_chain_l		| slide_chain_r
		// sankaku_sync | shikaku_sync  | batsu_sync | maru_sync | slide_l_sync | slide_r_sync | slide_chain_l_sync | slide_chain_r_sync

		sprSet = System::Data.Load<Graphics::SprSet>(System::Data.FindFile("spr/spr_comfy_editor.bin"));
		if (sprSet == nullptr || sprSet->TexSet.Textures.empty())
			return;

		buttonIconsTexture = sprSet->TexSet.Textures.front();

		const auto texelSize = vec2(1.0f, 1.0f) / vec2(buttonIconsTexture->GetSize());

		const auto width = buttonIconWidth * texelSize.x;
		const auto height = buttonIconWidth * texelSize.y;

		for (size_t i = 0; i < buttonIconsTextureCoordinates.size(); i++)
		{
			const auto x = (buttonIconWidth * (i % buttonIconsTypeCount)) * texelSize.x;
			const auto y = (buttonIconWidth * (i / buttonIconsTypeCount)) * texelSize.y;

			buttonIconsTextureCoordinates[i] = ImRect(x, y, x + width, y + height);
		}
	}

	void TargetTimeline::OnPlaybackResumed()
	{
		Audio::Engine::GetInstance().EnsureStreamRunning();
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
		if (auto sampleProvider = Audio::Engine::GetInstance().GetSharedSource(chartEditor.GetSongSource()); sampleProvider != nullptr)
			songWaveform.SetSource(sampleProvider);
		else
			songWaveform.Clear();

		updateWaveform = true;
	}

	void TargetTimeline::OnDrawTimelineHeaderWidgets()
	{
#if 0
		return;
#endif

		const auto& io = Gui::GetIO();

		Gui::PushStyleVar(ImGuiStyleVar_ItemSpacing, vec2(2.0f, 0.0f));

		ImGuiStyle& style = Gui::GetStyle();
		Gui::PushStyleVar(ImGuiStyleVar_FramePadding, vec2(8.0f, style.FramePadding.y));

		cursorTime.FormatTime(timeInputBuffer, sizeof(timeInputBuffer));

		constexpr float timeWidgetWidth = 138.0f;
		Gui::PushItemWidth(timeWidgetWidth);
		Gui::InputTextWithHint("##TargetTimeline::TimeInput", "00:00.000", timeInputBuffer, sizeof(timeInputBuffer));
		Gui::PopItemWidth();

		Gui::SameLine();
		Gui::Button(ICON_FA_FAST_BACKWARD);
		if (Gui::IsItemActive()) { scrollDelta -= io.DeltaTime * 1000.0f; }

		Gui::PushStyleVar(ImGuiStyleVar_ItemSpacing, vec2(0.0f, 0.0f));

		// TODO: jump to last / next target
		Gui::SameLine();
		Gui::Button(ICON_FA_BACKWARD);
		if (Gui::IsItemActive()) { scrollDelta -= io.DeltaTime * 400.0f; }

		Gui::SameLine();

		if (GetIsPlayback())
		{
			if (Gui::Button(ICON_FA_PAUSE))
				PausePlayback();
		}
		else
		{
			if (Gui::Button(ICON_FA_PLAY))
				ResumePlayback();
		}

		Gui::SameLine();
		if (Gui::Button(ICON_FA_STOP) && GetIsPlayback())
			StopPlayback();

		Gui::SameLine();
		Gui::Button(ICON_FA_FORWARD);
		if (Gui::IsItemActive()) { scrollDelta += io.DeltaTime * 400.0f; }

		Gui::SameLine();
		Gui::Button(ICON_FA_FAST_FORWARD);
		if (Gui::IsItemActive()) { scrollDelta += io.DeltaTime * 1000.0f; }

		Gui::PopStyleVar(2);

#if 0
		Gui::SameLine();
		Gui::PushItemWidth(280);
		Gui::SliderFloat(ICON_FA_SEARCH, &zoomLevel, ZOOM_MIN, ZOOM_MAX);
		Gui::PopItemWidth();
#endif

		Gui::PopStyleVar(1);

		Gui::SameLine();
		Gui::PushItemWidth(80);
		{
			if (gridDivisions[gridDivisionIndex] != gridDivision)
				gridDivisionIndex = GetGridDivisionIndex();

			if (Gui::Combo("Grid Precision", &gridDivisionIndex, gridDivisionStrings.data(), static_cast<int>(gridDivisionStrings.size())))
				gridDivision = gridDivisions[gridDivisionIndex];
		}
		Gui::PopItemWidth();
	}

	void TargetTimeline::OnDrawTimelineInfoColumnHeader()
	{
		TimelineBase::OnDrawTimelineInfoColumnHeader();


#if 1
		static constexpr const char* settingsPopupName = "TimelineSettingsPopup::ChartTimeline";

		Gui::PushStyleVar(ImGuiStyleVar_ItemSpacing, vec2(0.0f, 0.0f));
		Gui::PushStyleVar(ImGuiStyleVar_FramePadding, vec2(8.0f, 8.0f));

		constexpr vec4 transparent = vec4(0.0f);
		Gui::PushStyleColor(ImGuiCol_Button, transparent);
		{
			const bool isFirstFrame = false; // (cursorTime <= GetTimelineTime(loopStartFrame));
			const bool isLastFrame = false; // (cursorTime >= GetTimelineTime(loopEndFrame));
			const bool isPlayback = GetIsPlayback();

			constexpr float borderSize = 1.0f;
			Gui::SetCursorPosX(Gui::GetCursorPosX() + borderSize);

			// NOTE: First frame button
			{
				Gui::PushItemDisabledAndTextColorIf(isFirstFrame || isPlayback);
				if (Gui::Button(ICON_FA_FAST_BACKWARD))
				{
					SetCursorTime(TimeSpan::Zero());
					CenterCursor();
					scrollDelta = -GetTimelineSize();
				}
				Gui::PopItemDisabledAndTextColorIf(isFirstFrame || isPlayback);
				Gui::SetWideItemTooltip("Go to first beat");
			}

			// NOTE: Previous frame button
			{
				Gui::SameLine();
				Gui::PushItemDisabledAndTextColorIf(isFirstFrame || isPlayback);
				if (Gui::Button(ICON_FA_BACKWARD))
				{
					SetCursorTime(TickToTime(RoundTickToGrid(GetCursorTick()) - GetGridTick()));
					// cursorTime = GetTimelineTime(GetCursorFrame() - 1.0f);
					// RoundCursorTimeToNearestFrame();
				}
				Gui::PopItemDisabledAndTextColorIf(isFirstFrame || isPlayback);
				Gui::SetWideItemTooltip("Go to previous beat");
			}

			// NOTE: Playback toggle button
			{
				Gui::SameLine();
				if (Gui::Button(isPlayback ? ICON_FA_PAUSE : ICON_FA_PLAY))
				{
					isPlayback ? PausePlayback() : ResumePlayback();
				}
				Gui::SetWideItemTooltip("Toggle playback");
			}

			// NOTE: Playback stop button
			{
				Gui::SameLine();
				Gui::PushItemDisabledAndTextColorIf(!isPlayback && isFirstFrame);
				if (Gui::Button(ICON_FA_STOP))
				{
					StopPlayback();
				}
				Gui::PopItemDisabledAndTextColorIf(!isPlayback && isFirstFrame);
				Gui::SetWideItemTooltip("Stop playback");
			}

			// NOTE: Next frame button
			{
				Gui::SameLine();
				Gui::PushItemDisabledAndTextColorIf(isLastFrame || isPlayback);
				if (Gui::Button(ICON_FA_FORWARD))
				{
					SetCursorTime(TickToTime(RoundTickToGrid(GetCursorTick()) + GetGridTick()));
					// cursorTime = GetTimelineTime(GetCursorFrame() + 1);
					// RoundCursorTimeToNearestFrame();
				}
				Gui::PopItemDisabledAndTextColorIf(isLastFrame || isPlayback);
				Gui::SetWideItemTooltip("Go to next beat");
			}

			// NOTE: Last frame button
			{
				Gui::SameLine();
				Gui::PushItemDisabledAndTextColorIf(isLastFrame || isPlayback);
				if (Gui::Button(ICON_FA_FAST_FORWARD))
				{
					SetCursorTime(workingChart->GetDuration());
					CenterCursor();

					// cursorTime = GetTimelineTime(loopEndFrame - 1.0f);
					// RoundCursorTimeToNearestFrame();
					//scrollDelta = +GetTimelineSize();
				}
				Gui::PopItemDisabledAndTextColorIf(isLastFrame || isPlayback);
				Gui::SetWideItemTooltip("Go to last beat");
			}

			// TODO: Filler button for now, what functionality should go here?
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
#endif
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
			DrawButtonIcon(drawList, target, center, iconScale);

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
		const i32 gridTickStep = GetGridTick().TotalTicks();

		const auto scrollX = GetScrollX();

		constexpr auto beatSpacingThreshold = 10.0f;
		constexpr auto barSpacingThreshold = 74.0f;

		// BUG: Ugly flickering when scrolling to the right
		auto lastBeatScreenX = -beatSpacingThreshold;
		for (i32 tick = 0, divisions = 0; tick < songDurationTicks; tick += gridTickStep, divisions++)
		{
			const auto screenX = glm::round(GetTimelinePosition(TimelineTick(tick)) - scrollX);
			const auto visiblity = GetTimelineVisibility(screenX);

			if (visiblity == TimelineVisibility::Left)
				continue;
			if (visiblity == TimelineVisibility::Right)
				break;

			const auto distanceToLastBeat = (screenX - lastBeatScreenX);
			if (distanceToLastBeat < beatSpacingThreshold)
				continue;

			lastBeatScreenX = screenX;

			const auto start = timelineContentRegion.GetTL() + vec2(screenX, -(timelineHeaderHeight * 0.35f));
			const auto end = timelineContentRegion.GetBL() + vec2(screenX, 0.0f);
			baseDrawList->AddLine(start, end, (divisions % 2 == 0 ? gridColor : gridAltColor));
		}

		// TODO: Maybe allow for drawing second divisions for example instead (?)

		auto lastBarScreenX = -barSpacingThreshold;
		for (i32 tick = 0, barIndex = 0; tick < songDurationTicks; tick += TimelineTick::TicksPerBeat * 4, barIndex++)
		{
			const auto screenX = glm::round(GetTimelinePosition(TimelineTick(tick)) - scrollX);
			const auto visiblity = GetTimelineVisibility(screenX);

			if (visiblity == TimelineVisibility::Left)
				continue;
			if (visiblity == TimelineVisibility::Right)
				break;

			const auto distanceToLastBar = (screenX - lastBarScreenX);
			if (distanceToLastBar < barSpacingThreshold)
				continue;

			lastBarScreenX = screenX;

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
#if 1
		constexpr float timeDragTextOffset = 10.0f;
		constexpr float timeDragTextWidth = 60.0f + 26.0f;
		Gui::SetCursorPosX(Gui::GetCursorPosX() + timeDragTextOffset);

		Gui::PushStyleVar(ImGuiStyleVar_FramePadding, vec2(Gui::GetStyle().FramePadding.x, 0.0f));

		// NOTE: Time drag text
		{
			char cursorTimeBuffer[TimeSpan::RequiredFormatBufferSize];
			cursorTime.FormatTime(cursorTimeBuffer, sizeof(cursorTimeBuffer));

			constexpr auto dragSpeed = 16.0f;
			auto cursorDragTicks = static_cast<f32>(GetCursorTick().TotalTicks());

			if (Gui::ComfyDragText("TimeDragText::AetTimeline", cursorTimeBuffer, &cursorDragTicks, dragSpeed, 0.0f, 0.0f, timeDragTextWidth))
				SetCursorTime(TickToTime(RoundTickToGrid(TimelineTick::FromTicks(static_cast<int>(cursorDragTicks)))));
		}

#if 0
		// NOTE: Mode buttons (Dopesheet / Curves)
		{
			const auto modeButton = [this](const char* label, const TimelineMode mode)
			{
				Gui::PushStyleColor(ImGuiCol_Button, Gui::GetStyleColorVec4(currentTimelineMode == mode ? ImGuiCol_ButtonHovered : ImGuiCol_Button));
				{
					constexpr float modeButtonWidth = 72.0f;
					if (Gui::Button(label, vec2(modeButtonWidth, timelineScrollbarSize.y)))
						currentTimelineMode = mode;
				}
				Gui::PopStyleColor(1);
			};

			Gui::SameLine();
			modeButton("Dope Sheet", TimelineMode::DopeSheet);
			Gui::SameLine();
			modeButton("Curves", TimelineMode::Curves);
		}
#endif

		Gui::PopStyleVar(1);
#endif
	}

	void TargetTimeline::DrawWaveform()
	{
		if (zoomLevelChanged)
			updateWaveform = true;

		if (updateWaveform)
		{
			const auto timePerPixel = GetTimelineTime(2.0f) - GetTimelineTime(1.0f);
			songWaveform.SetScale(timePerPixel);

			updateWaveform = false;
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

		for (i64 screenPixel = leftMostVisiblePixel; screenPixel < waveformPixelCount && screenPixel < rightMostVisiblePixel; screenPixel++)
		{
			const auto timelinePixel = std::min(static_cast<i64>(screenPixel + scrollXStartOffset), static_cast<i64>(waveformPixelCount - 1));
			if (timelinePixel < 0)
				continue;

			constexpr u32 channelsToVisualize = 2;
			for (auto channel = 0; channel < channelsToVisualize; channel++)
			{
				const auto amplitude = songWaveform.GetNormalizedPCMForPixel(timelinePixel, channel) * timelineHeight;
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
		char tempoStr[16];
		static int tempoPopupIndex = -1;

		for (size_t i = 0; i < workingChart->GetTempoMap().TempoChangeCount(); i++)
		{
			TempoChange& tempoChange = workingChart->GetTempoMap().GetTempoChangeAt(i);

			const auto screenX = glm::round(GetTimelinePosition(tempoChange.Tick) - GetScrollX());
			const auto visiblity = GetTimelineVisibility(screenX);

			if (visiblity == TimelineVisibility::Left)
				continue;
			if (visiblity == TimelineVisibility::Right)
				break;

			ImU32 tempoFgColor = IM_COL32(139, 56, 51, 255);

			sprintf_s(tempoStr, sizeof(tempoStr), "%.2f BPM", tempoChange.Tempo.BeatsPerMinute);

			vec2 buttonPosition = tempoMapRegion.GetTL() + vec2(screenX + 1.0f, 0.0f);
			vec2 buttonSize = vec2(Gui::CalcTextSize(tempoStr).x, tempoMapHeight);

			Gui::SetCursorScreenPos(buttonPosition);

			Gui::PushID(&tempoChange);
			Gui::InvisibleButton("##InvisibleTempoButton", buttonSize);
			Gui::PopID();

			// TODO: Prevent overlapping tempo changes
			//windowDrawList->AddRectFilled(buttonPosition, buttonPosition + buttonSize, TEMPO_MAP_BAR_COLOR);
			if (Gui::IsItemHovered() && Gui::IsWindowHovered())
			{
				Gui::WideSetTooltip("TIME: %s", TickToTime(tempoChange.Tick).ToString().c_str());

				baseDrawList->AddRect(buttonPosition, buttonPosition + buttonSize, Gui::GetColorU32(ImGuiCol_ChildBg));
				if (Gui::IsMouseDoubleClicked(0))
				{
					SetScrollX(screenX + GetScrollX());
					//SetScrollX(screenX - timelineContentRegion.GetTL().x - (windowWidth * .5f));
				}

				if (Gui::IsMouseClicked(1))
				{
					Gui::OpenPopup("##ChangeTempoPopup");
					tempoPopupIndex = static_cast<int>(i);
				}
			}

			baseDrawList->AddLine(buttonPosition + vec2(-1.0f, -1.0f), buttonPosition + vec2(-1.0f, buttonSize.y - 1.0f), tempoFgColor);
			baseDrawList->AddText(Gui::GetFont(), tempoMapHeight, buttonPosition, tempoFgColor, tempoStr);
		}

		if (Gui::WideBeginPopup("##ChangeTempoPopup"))
		{
			Gui::Text("Change Tempo:");

			if (tempoPopupIndex >= 0)
			{
				TempoChange& tempoChange = workingChart->GetTempoMap().GetTempoChangeAt(tempoPopupIndex);
				auto bpm = tempoChange.Tempo.BeatsPerMinute;

				if (Gui::DragFloat("##TempoDragFloat", &bpm, 1.0f, Tempo::MinBPM, Tempo::MaxBPM, "%.2f BPM"))
				{
					tempoChange.Tempo = bpm;
					UpdateTimelineMap();
				}
			}

			Gui::EndPopup();
		}
	}

	void TargetTimeline::DrawTimelineTargets()
	{
		auto windowDrawList = Gui::GetWindowDrawList();

		for (const auto& target : workingChart->GetTargets())
		{
			const auto buttonTime = TickToTime(target.Tick);
			const auto screenX = GetTimelinePosition(buttonTime) - GetScrollX();

			const auto visiblity = GetTimelineVisibility(screenX);
			if (visiblity == TimelineVisibility::Left)
				continue;
			if (visiblity == TimelineVisibility::Right)
				break;

			const auto buttonIndex = static_cast<size_t>(target.Type);
			const auto center = vec2(screenX + timelineContentRegion.GetTL().x, targetYPositions[buttonIndex]);
			const auto scale = GetTimelineTargetScaleFactor(target, buttonTime) * iconScale;

			DrawButtonIcon(windowDrawList, target, center, scale, GetButtonEdgeFadeOpacity(screenX));
		}
	}

	f32 TargetTimeline::GetTimelineTargetScaleFactor(const TimelineTarget& target, TimeSpan buttonTime) const
	{
		if (GetIsPlayback())
		{
			const auto cursorTime = GetCursorTime();
			const auto timeUntilButton = buttonTime - cursorTime;

			if (timeUntilButton <= TimeSpan::FromSeconds(0.0) && timeUntilButton >= -buttonAnimationDuration)
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
		//Gui::SetMouseCursor(ImGuiMouseCursor_Hand);
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
				buttonAnimations[buttonIndex].ElapsedTime = TimeSpan::FromSeconds(0.0);
			}
		}

#if 0 // DEBUG: Cursor Mouse Drag:
		if (!GetIsPlayback() && false)
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
				if (abs(timeSelectionStart.TotalTicks() - timeSelectionEnd.TotalTicks()) > (GetGridTick().TotalTicks() * 2))
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
		if (Gui::IsMouseClicked(3)) SelectNextGridDivision(-1);
		if (Gui::IsMouseClicked(4)) SelectNextGridDivision(+1);

		const auto deltaTime = TimeSpan::FromSeconds(io.DeltaTime);
		for (auto& animationData : buttonAnimations)
			animationData.ElapsedTime += deltaTime;

		for (auto[buttonType, keyCode] : targetPlacementInputKeyMappings)
		{
			if (Gui::IsKeyPressed(keyCode, false))
			{
				// NOTE: Double hit sound if a target gets placed in front of the cursor position.
				//		 Keeping it this way could make it easier to notice when real time targets are not placed accurately to the beat (?)
				buttonSoundController.PlayButtonSound();
				PlaceOrRemoveTarget(RoundTickToGrid(GetCursorTick()), buttonType);
			}
		}
	}

	void TargetTimeline::PlaceOrRemoveTarget(TimelineTick tick, ButtonType type)
	{
		const auto existingTargetIndex = workingChart->GetTargets().FindIndex(tick, type);

		if (existingTargetIndex > -1)
		{
			if (!GetIsPlayback())
				workingChart->GetTargets().RemoveAt(existingTargetIndex);
		}
		else
		{
			workingChart->GetTargets().Add(tick, type);
		}

		const auto buttonIndex = static_cast<size_t>(type);
		buttonAnimations[buttonIndex].Tick = tick;
		buttonAnimations[buttonIndex].ElapsedTime = TimeSpan::FromSeconds(0.0);
	}

	void TargetTimeline::SelectNextGridDivision(int direction)
	{
		const auto nextIndex = ImClamp(gridDivisionIndex + direction, 0, static_cast<int>(gridDivisions.size()) - 1);
		gridDivision = gridDivisions[nextIndex];
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
		lastButtonSoundCursorTime = buttonSoundCursorTime = (newCursorTime - buttonSoundFutureOffset);
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
			const TimeSpan increment = TimeSpan((io.KeyShift ? 1.0 : 0.5) * io.MouseWheel);

			chartEditor.PausePlayback();
			SetCursorTime(chartEditor.GetPlaybackTimeAsync() + increment);

			if (chartEditor.GetPlaybackTimeAsync() < TimeSpan::FromSeconds(0.0))
				SetCursorTime(TimeSpan::FromSeconds(0.0));

			chartEditor.ResumePlayback();

			CenterCursor();
		}
		else
		{
			TimelineBase::OnTimelineBaseScroll();
		}
	}
}
