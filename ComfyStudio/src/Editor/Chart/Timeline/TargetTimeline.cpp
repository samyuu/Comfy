#include "TargetTimeline.h"
#include "Editor/Chart/ChartEditor.h"
#include "Editor/Chart/TempoMap.h"
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

		callbackReceiver = nullptr;

		callbackReceiver = std::make_unique<Audio::CallbackReceiver>([&]()
		{
			UpdateOnCallbackSounds();

			if (checkHitsoundsInCallback && updateInput)
				UpdateOnCallbackPlacementSounds();
		});
	}

	TimelineTick TargetTimeline::GetGridTick() const
	{
		return (TimelineTick::TicksPerBeat * 4) / gridDivision;
	}

	TimelineTick TargetTimeline::FloorToGrid(TimelineTick tick) const
	{
		const int gridTicks = GetGridTick().TotalTicks();
		return static_cast<int>(glm::floor(tick.TotalTicks() / static_cast<float>(gridTicks)) * gridTicks);
	}

	TimelineTick TargetTimeline::RoundToGrid(TimelineTick tick) const
	{
		const int gridTicks = GetGridTick().TotalTicks();
		return static_cast<int>(glm::round(tick.TotalTicks() / static_cast<float>(gridTicks)) * gridTicks);
	}

	float TargetTimeline::GetTimelinePosition(TimeSpan time) const
	{
		return TimelineBase::GetTimelinePosition(time);
	}

	float TargetTimeline::GetTimelinePosition(TimelineTick tick) const
	{
		return GetTimelinePosition(GetTimelineTime(tick));
	}

	TimelineTick TargetTimeline::GetTimelineTick(TimeSpan time) const
	{
		return workingChart->GetTimelineMap().GetTickAt(time);
	}

	TimelineTick TargetTimeline::GetTimelineTick(float position) const
	{
		return GetTimelineTick(GetTimelineTime(position));
	}

	TimeSpan TargetTimeline::GetTimelineTime(TimelineTick tick) const
	{
		return workingChart->GetTimelineMap().GetTimeAt(tick);
	}

	TimeSpan TargetTimeline::GetTimelineTime(float position) const
	{
		return TimelineBase::GetTimelineTime(position);
	}

	TimelineTick TargetTimeline::GetCursorTick() const
	{
		return GetTimelineTick(GetCursorTime());
	}

	TimelineTick TargetTimeline::GetCursorMouseXTick() const
	{
		return FloorToGrid(GetTimelineTick(ScreenToTimelinePosition(Gui::GetMousePos().x)));
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

	float TargetTimeline::GetButtonTransparency(float screenX) const
	{
		constexpr float fadeSpan = 35.0f;

		if (/*screenX < 0.0f ||*/ screenX > baseWindow->Size.x)
			return 0.0f;

		// const float lowerThreshold = fadeSpan;
		// if (screenX < lowerThreshold)
		// 	return ImLerp(0.0f, 1.0f, screenX / lowerThreshold);

		const float upperThreshold = baseWindow->Size.x - fadeSpan;
		if (screenX > upperThreshold)
			return ImLerp(0.0f, 1.0f, 1.0f - ((screenX - upperThreshold) / (baseWindow->Size.x - upperThreshold)));

		return 1.0f;
	}

	int TargetTimeline::GetButtonIconIndex(const TimelineTarget& target) const
	{
		int type = target.Type;

		if ((target.Flags & TargetFlags_Chain) && (type == TargetType_SlideL || type == TargetType_SlideR))
			type += 2;

		if ((target.Flags & TargetFlags_Sync))
			return type;

		return type + 8;
	}

	void TargetTimeline::DrawButtonIcon(ImDrawList* drawList, const TimelineTarget& target, vec2 position, float scale, float transparency)
	{
		const float width = buttonIconWidth * scale;
		const float height = buttonIconWidth * scale;

		position.x = glm::round(position.x - (width * 0.5f));
		position.y = glm::round(position.y - (height * 0.5f));

		vec2 bottomRight = vec2(position.x + width, position.y + height);
		ImRect textureCoordinates = buttonIconsTextureCoordinates[GetButtonIconIndex(target)];

		const ImU32 color = IM_COL32(0xFF, 0xFF, 0xFF, 0xFF * transparency);
		if (buttonIconsTexture != nullptr)
			drawList->AddImage(*buttonIconsTexture, position, bottomRight, textureCoordinates.GetBL(), textureCoordinates.GetTR(), color);

		if ((target.Flags & TargetFlags_Hold))
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
	}

	void TargetTimeline::UpdateTimelineMap()
	{
		workingChart->GetTimelineMap().CalculateMapTimes(workingChart->GetTempoMap());
	}

	void TargetTimeline::UpdateOnCallbackSounds()
	{
		if (!GetIsPlayback())
			return;

		for (int i = 0; i < buttonSoundTimesList.size(); ++i)
		{
			if (chartEditor.GetPlaybackTime() >= buttonSoundTimesList[i])
			{
				buttonSoundController.PlayButtonSound();
				buttonSoundTimesList.erase(buttonSoundTimesList.begin() + (i--));
			}
			else
			{
				break;
			}
		}
	}

	void TargetTimeline::UpdateOnCallbackPlacementSounds()
	{
		for (size_t i = 0; i < std::size(buttonPlacementMapping); i++)
		{
			buttonPlacementKeyStates[i].WasDown = buttonPlacementKeyStates[i].Down;
			buttonPlacementKeyStates[i].Down = Gui::IsKeyDown(buttonPlacementMapping[i].Key);

			if (buttonPlacementKeyStates[i].Down && !buttonPlacementKeyStates[i].WasDown)
			{
				Audio::Engine::GetInstance().EnsureStreamRunning();
				buttonSoundController.PlayButtonSound();
			}
		}
	}

	void TargetTimeline::InitializeButtonIcons()
	{
		// NOTE:
		// sankaku		| shikaku		| batsu		 | maru		 | slide_l		| slide_r	   | slide_chain_l		| slide_chain_r
		// sankaku_sync | shikaku_sync  | batsu_sync | maru_sync | slide_l_sync | slide_r_sync | slide_chain_l_sync | slide_chain_r_sync
		if (const auto sprFileEntry = System::Data.FindFile("spr/spr_comfy_editor.bin"); sprFileEntry != nullptr)
		{
			auto sprFileBuffer = std::make_unique<u8[]>(sprFileEntry->Size);
			System::Data.ReadEntryIntoBuffer(*sprFileEntry, sprFileBuffer.get());

			sprSet = std::make_unique<Graphics::SprSet>();
			sprSet->Parse(sprFileBuffer.get(), sprFileEntry->Size);

			if (sprSet != nullptr && !sprSet->TexSet->Textures.empty())
				buttonIconsTexture = sprSet->TexSet->Textures.front();
		}
		else
		{
			return;
		}

		const auto texelSize = vec2(1.0f, 1.0f) / vec2(buttonIconsTexture->GetSize());

		const float width = buttonIconWidth * texelSize.x;
		const float height = buttonIconWidth * texelSize.y;

		for (size_t i = 0; i < buttonIconsTextureCoordinates.size(); i++)
		{
			const float x = (buttonIconWidth * (i % buttonIconsTypeCount)) * texelSize.x;
			const float y = (buttonIconWidth * (i / buttonIconsTypeCount)) * texelSize.y;

			buttonIconsTextureCoordinates[i] = ImRect(x, y, x + width, y + height);
		}
	}

	void TargetTimeline::OnPlaybackResumed()
	{
		Audio::Engine::GetInstance().EnsureStreamRunning();

		buttonSoundTimesList.clear();
		for (const auto& target : workingChart->GetTargets())
		{
			TimeSpan buttonTime = GetTimelineTime(target.Tick);
			if (buttonTime >= chartEditor.GetPlaybackTime())
				buttonSoundTimesList.push_back(buttonTime);
		}
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
		const auto& io = Gui::GetIO();

		Gui::PushStyleVar(ImGuiStyleVar_ItemSpacing, vec2(2.0f, 0.0f));

		ImGuiStyle& style = Gui::GetStyle();
		Gui::PushStyleVar(ImGuiStyleVar_FramePadding, vec2(8.0f, style.FramePadding.y));

		cursorTime.FormatTime(timeInputBuffer, sizeof(timeInputBuffer));

		constexpr float timeWidgetWidth = 138;
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

		Gui::SameLine();
		Gui::PushItemWidth(280);
		Gui::SliderFloat(ICON_FA_SEARCH, &zoomLevel, ZOOM_MIN, ZOOM_MAX);
		Gui::PopItemWidth();

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
	}

	void TargetTimeline::OnDrawTimelineInfoColumn()
	{
		TimelineBase::OnDrawTimelineInfoColumn();

		auto drawList = Gui::GetWindowDrawList();
		for (int i = 0; i < TargetType_Max; i++)
		{
			float y = i * rowHeight;
			auto start = vec2(0.0f, y) + infoColumnRegion.GetTL();
			auto end = vec2(infoColumnWidth, y + rowHeight) + infoColumnRegion.GetTL();

			auto center = vec2((start.x + end.x) / 2.0f, (start.y + end.y) / 2.0f);
			targetYPositions[i] = center.y;

			TimelineTarget target(0, static_cast<TargetType>(i));
			DrawButtonIcon(drawList, target, center, iconScale);
		}
	}

	void TargetTimeline::OnDrawTimlineRows()
	{
		vec2 timelineTL = timelineContentRegion.GetTL();
		vec2 timelineWidth = vec2(timelineContentRegion.GetWidth(), 0.0f);

		ImU32 rowColor = GetColor(EditorColor_TimelineRowSeparator);

		// Timeline Target Region rows
		// ---------------------------
		for (int t = 0; t <= TargetType_Max; t++)
		{
			vec2 start = timelineTL + vec2(0.0f, t * rowHeight);
			vec2 end = start + timelineWidth;

			baseDrawList->AddLine(start, end, rowColor);
		}
	}

	void TargetTimeline::OnDrawTimlineDivisors()
	{
		// Timeline Bar / Beat lines
		// -------------------------
		{
			ImU32 barColor = GetColor(EditorColor_Bar);
			ImU32 gridColor = GetColor(EditorColor_Grid);
			ImU32 gridAltColor = GetColor(EditorColor_GridAlt);

			char barStrBuffer[16];
			int barCount = -1;

			const int totalTicks = GetTimelineTick(workingChart->GetDuration()).TotalTicks();
			const int tickStep = GetGridTick().TotalTicks();

			const float scrollX = GetScrollX();

			for (int tick = 0, divisions = 0; tick < totalTicks; tick += tickStep)
			{
				bool isBar = tick % (TimelineTick::TicksPerBeat * 4) == 0;
				if (isBar)
					barCount++;

				float screenX = glm::round(GetTimelinePosition(TimelineTick(tick)) - scrollX);
				TimelineVisibility visiblity = GetTimelineVisibility(screenX);

				if (visiblity == TimelineVisibility::Left)
					continue;
				if (visiblity == TimelineVisibility::Right)
					break;

				const float startYOffset = timelineHeaderHeight * (isBar ? 0.85f : 0.35f);
				vec2 start = timelineContentRegion.GetTL() + vec2(screenX, -startYOffset);
				vec2 end = timelineContentRegion.GetBL() + vec2(screenX, 0.0f);

				const ImU32 color = isBar ? barColor : (divisions++ % 2 == 0 ? gridColor : gridAltColor);
				baseDrawList->AddLine(start, end, color);

				if (isBar)
				{
					sprintf_s(barStrBuffer, sizeof(barStrBuffer), "%d", barCount);

					start += vec2(3.0f, -1.0f);
					baseDrawList->AddText(start, color, barStrBuffer);
				}
			}
		}
	}

	void TargetTimeline::OnDrawTimlineBackground()
	{
		DrawWaveform();
		DrawTimelineTempoMap();
	}

	void TargetTimeline::DrawWaveform()
	{
		if (zoomLevelChanged)
			updateWaveform = true;

		if (updateWaveform)
		{
			const TimeSpan timePerPixel = GetTimelineTime(2.0f) - GetTimelineTime(1.0f);
			songWaveform.SetScale(timePerPixel);

			updateWaveform = false;
		}

		if (songWaveform.GetPixelCount() < 1)
			return;

		float scrollXStartOffset = GetScrollX() + GetTimelinePosition(workingChart->GetStartOffset());

		i64 leftMostVisiblePixel = static_cast<i64>(GetTimelinePosition(TimelineTick(0)));
		i64 rightMostVisiblePixel = leftMostVisiblePixel + static_cast<i64>(timelineContentRegion.GetWidth());
		i64 waveformPixelCount = static_cast<i64>(songWaveform.GetPixelCount());

		float timelineX = timelineContentRegion.GetTL().x;
		float timelineHeight = (TargetType_Max * rowHeight);
		float timelineCenterY = timelineContentRegion.GetTL().y + (timelineHeight * 0.5f);

		i64 waveformPixelsDrawn = 0;
		ImU32 waveformColor = GetColor(EditorColor_GridAlt);

		for (i64 screenPixel = leftMostVisiblePixel; screenPixel < waveformPixelCount && screenPixel < rightMostVisiblePixel; screenPixel++)
		{
			i64 timelinePixel = std::min(static_cast<i64>(screenPixel + scrollXStartOffset), static_cast<i64>(waveformPixelCount - 1));

			if (timelinePixel < 0)
				continue;

			// TODO: Try visualizing by interpolating inbetween pixels (2x AA)
			float amplitude = songWaveform.GetNormalizedPCMForPixel(timelinePixel) * timelineHeight;

			float x = screenPixel + timelineX;
			float halfAmplitude = amplitude * 0.5f;

			vec2 start = vec2(x, timelineCenterY - halfAmplitude);
			vec2 end = vec2(x, timelineCenterY + halfAmplitude);

			baseDrawList->AddLine(start, end, waveformColor);
			waveformPixelsDrawn++;
		}
	}

	void TargetTimeline::DrawTimelineTempoMap()
	{
		// Tempo Map Region / Tempo Changes
		// --------------------------------
		{
			char tempoStr[16];
			static int tempoPopupIndex = -1;

			for (size_t i = 0; i < workingChart->GetTempoMap().TempoChangeCount(); i++)
			{
				TempoChange& tempoChange = workingChart->GetTempoMap().GetTempoChangeAt(i);

				float screenX = glm::round(GetTimelinePosition(tempoChange.Tick) - GetScrollX());

				TimelineVisibility visiblity = GetTimelineVisibility(screenX);
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
					Gui::WideSetTooltip("TIME: %s", GetTimelineTime(tempoChange.Tick).ToString().c_str());

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

			// Test Popup
			// ----------
			if (Gui::WideBeginPopup("##ChangeTempoPopup"))
			{
				Gui::Text("Change Tempo:");

				if (tempoPopupIndex >= 0)
				{
					TempoChange& tempoChange = workingChart->GetTempoMap().GetTempoChangeAt(tempoPopupIndex);
					float bpm = tempoChange.Tempo.BeatsPerMinute;

					if (Gui::DragFloat("##TempoDragFloat", &bpm, 1.0f, Tempo::MinBPM, Tempo::MaxBPM, "%.2f BPM"))
					{
						tempoChange.Tempo = bpm;
						UpdateTimelineMap();
					}
				}

				Gui::EndPopup();
			}
			// ----------
		}
	}

	void TargetTimeline::DrawTimelineTargets()
	{
		ImGuiWindow* window = Gui::GetCurrentWindow();
		ImDrawList* windowDrawList = window->DrawList;

		for (const auto& target : workingChart->GetTargets())
		{
			TimeSpan buttonTime = GetTimelineTime(target.Tick);
			float screenX = GetTimelinePosition(buttonTime) - GetScrollX();

			TimelineVisibility visiblity = GetTimelineVisibility(screenX);
			if (visiblity == TimelineVisibility::Left)
				continue;
			if (visiblity == TimelineVisibility::Right)
				break;

			vec2 center = vec2(screenX + timelineContentRegion.GetTL().x, targetYPositions[target.Type]);

			float scale = iconScale;

			if (GetIsPlayback())
			{
				const TimeSpan cursorTime = GetCursorTime();
				const TimeSpan timeUntilButton = buttonTime - cursorTime;

				if (timeUntilButton <= TimeSpan::FromSeconds(0.0) && timeUntilButton >= -buttonAnimationDuration)
				{
					float t = static_cast<float>(timeUntilButton.TotalSeconds() / -buttonAnimationDuration.TotalSeconds());
					scale *= ImLerp(buttonAnimationScale, 1.0f, t);
				}
			}
			else
			{
				if (target.Tick == buttonAnimations[target.Type].Tick)
				{
					if (buttonAnimations[target.Type].ElapsedTime >= buttonAnimationStartTime && buttonAnimations[target.Type].ElapsedTime <= buttonAnimationDuration)
					{
						float t = static_cast<float>(buttonAnimations[target.Type].ElapsedTime.TotalSeconds() / buttonAnimationDuration.TotalSeconds());
						scale *= ImLerp(buttonAnimationScale, 1.0f, t);
					}
				}
			}

			float transparency = GetButtonTransparency(screenX);
			DrawButtonIcon(windowDrawList, target, center, scale, transparency);
		}
	}

	void TargetTimeline::DrawTimelineCursor()
	{
		if (GetIsPlayback())
		{
			float prePlaybackX = glm::round(GetTimelinePosition(chartEditor.GetPlaybackTimeOnPlaybackStart()) - GetScrollX());

			vec2 start = timelineHeaderRegion.GetTL() + vec2(prePlaybackX, 0.0f);
			vec2 end = timelineContentRegion.GetBL() + vec2(prePlaybackX, 0.0f);

			baseDrawList->AddLine(start, end, GetColor(EditorColor_CursorInner));
		}

		TimelineBase::DrawTimelineCursor();
	}

	void TargetTimeline::DrawTimeSelection()
	{
		// Test out selection box
		// ----------------------
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

		if (!timeSelectionActive)
			return;

		const float scrollX = GetScrollX();

		float startScreenX = GetTimelinePosition(timeSelectionStart) - scrollX;
		float endScreenX = GetTimelinePosition(timeSelectionEnd) - scrollX;

		vec2 start = timelineContentRegion.GetTL() + vec2(startScreenX, 0.0f);
		vec2 end = timelineContentRegion.GetBL() + vec2(endScreenX, 0.0f);

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

		// NOTE: Cursor Mouse Click:
		if (Gui::IsMouseClicked(0) && !io.KeyShift) // Gui::IsMouseReleased(0)
		{
			const bool wasPlaying = GetIsPlayback();
			if (wasPlaying)
				chartEditor.PausePlayback();

			const TimelineTick cursorMouseTick = GetCursorMouseXTick();
			TimeSpan previousTime = GetCursorTime();
			TimeSpan newTime = GetTimelineTime(cursorMouseTick);

			if (previousTime == newTime)
				return;

			chartEditor.SetPlaybackTime(newTime);

			if (wasPlaying)
			{
				chartEditor.ResumePlayback();
			}
			else // NOTE: Play a button sound if a target exists at the cursor tick
			{
				for (const auto& target : workingChart->GetTargets())
				{
					if (target.Tick == cursorMouseTick)
					{
						Audio::Engine::GetInstance().EnsureStreamRunning();
						buttonSoundController.PlayButtonSound();

						buttonAnimations[target.Type].Tick = target.Tick;
						buttonAnimations[target.Type].ElapsedTime = TimeSpan::FromSeconds(0.0);
					}
				}
			}
		}

		// NOTE: Cursor Mouse Drag:
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
	}

	void TargetTimeline::UpdateInputTargetPlacement()
	{
		const auto& io = Gui::GetIO();

		if (!Gui::IsWindowFocused())
			return;

		// NOTE: Mouse X buttons, increase / decrease grid division
		if (Gui::IsMouseClicked(3)) SelectNextGridDivision(-1);
		if (Gui::IsMouseClicked(4)) SelectNextGridDivision(+1);

		for (int type = 0; type < TargetType_Max; type++)
			buttonAnimations[type].ElapsedTime += TimeSpan::FromSeconds(io.DeltaTime);

		TimelineTick cursorTick = RoundToGrid(GetCursorTick());
		for (size_t i = 0; i < std::size(buttonPlacementMapping); i++)
		{
			if (Gui::IsKeyPressed(buttonPlacementMapping[i].Key, false))
			{
				if (!checkHitsoundsInCallback)
				{
					Audio::Engine::GetInstance().EnsureStreamRunning();
					buttonSoundController.PlayButtonSound();
				}
				PlaceOrRemoveTarget(cursorTick, buttonPlacementMapping[i].Type);
			}
		}
	}

	void TargetTimeline::PlaceOrRemoveTarget(TimelineTick tick, TargetType type)
	{
		i64 existingTargetIndex = workingChart->GetTargets().FindIndex(tick, type);

		if (existingTargetIndex > -1)
		{
			if (!GetIsPlayback())
				workingChart->GetTargets().Remove(existingTargetIndex);
		}
		else
		{
			workingChart->GetTargets().Add(tick, type);
		}

		buttonAnimations[type].Tick = tick;
		buttonAnimations[type].ElapsedTime = TimeSpan::FromSeconds(0.0);
	}

	void TargetTimeline::SelectNextGridDivision(int direction)
	{
		int nextIndex = ImClamp(gridDivisionIndex + direction, 0, static_cast<int>(gridDivisions.size()) - 1);
		gridDivision = gridDivisions[nextIndex];
	}

	TimeSpan TargetTimeline::GetCursorTime() const
	{
		return chartEditor.GetPlaybackTime();
	}

	bool TargetTimeline::GetIsPlayback() const
	{
		return chartEditor.GetIsPlayback();
	}

	void TargetTimeline::PausePlayback()
	{
		chartEditor.PausePlayback();
	}

	void TargetTimeline::ResumePlayback()
	{
		chartEditor.ResumePlayback();
	}

	void TargetTimeline::StopPlayback()
	{
		chartEditor.StopPlayback();
	}

	float TargetTimeline::GetTimelineSize() const
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
			chartEditor.SetPlaybackTime(chartEditor.GetPlaybackTime() + increment);

			if (chartEditor.GetPlaybackTime() < TimeSpan::FromSeconds(0.0))
				chartEditor.SetPlaybackTime(TimeSpan::FromSeconds(0.0));

			chartEditor.ResumePlayback();

			CenterCursor();
		}
		else
		{
			TimelineBase::OnTimelineBaseScroll();
		}
	}
}
