#include "TargetTimeline.h"
#include "Editor/Chart/ChartEditor.h"
#include "Editor/Chart/TempoMap.h"
#include "Editor/Editor.h"
#include "Audio/AudioEngine.h"
#include "FileSystem/FileHelper.h"
#include "Core/TimeSpan.h"
#include <FontIcons.h>

namespace Editor
{
	static void EnsureStreamOpenAndRunning()
	{
		auto audioEngine = AudioEngine::GetInstance();

		if (!audioEngine->GetIsStreamOpen())
			audioEngine->OpenStream();
		if (!audioEngine->GetIsStreamRunning())
			audioEngine->StartStream();
	}

	TargetTimeline::TargetTimeline(ChartEditor* parentChartEditor)
	{
		scrollSpeed = 2.5f;
		scrollSpeedFast = 5.5f;

		chartEditor = parentChartEditor;
		chart = chartEditor->GetChart();
	}

	TargetTimeline::~TargetTimeline()
	{
		AudioEngine::GetInstance()->RemoveCallbackReceiver(this);
	}

	TimelineTick TargetTimeline::GetGridTick() const
	{
		return (TimelineTick::TICKS_PER_BEAT * 4) / gridDivision;
	}

	TimelineTick TargetTimeline::FloorToGrid(TimelineTick tick) const
	{
		int gridTicks = GetGridTick().TotalTicks();
		return static_cast<int>(glm::floor(tick.TotalTicks() / static_cast<float>(gridTicks)) * gridTicks);
	}

	TimelineTick TargetTimeline::RoundToGrid(TimelineTick tick) const
	{
		int gridTicks = GetGridTick().TotalTicks();
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
		return chart->GetTimelineMap().GetTickAt(time);
	}

	TimelineTick TargetTimeline::GetTimelineTick(float position) const
	{
		return GetTimelineTick(GetTimelineTime(position));
	}

	TimeSpan TargetTimeline::GetTimelineTime(TimelineTick tick) const
	{
		return chart->GetTimelineMap().GetTimeAt(tick);
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
		return FloorToGrid(GetTimelineTick(ScreenToTimelinePosition(ImGui::GetMousePos().x)));
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

		position.x = glm::round(position.x - (width * .5f));
		position.y = glm::round(position.y - (height * .5f));

		vec2 bottomRight = vec2(position.x + width, position.y + height);
		ImRect textureCoordinates = buttonIconsTextureCoordinates[GetButtonIconIndex(target)];

		const ImU32 color = IM_COL32(0xFF, 0xFF, 0xFF, 0xFF * transparency);
		drawList->AddImage(buttonIconsTexture->GetVoidTexture(), position, bottomRight, textureCoordinates.GetBL(), textureCoordinates.GetTR(), color);

		if ((target.Flags & TargetFlags_Hold))
		{
			// TODO: draw tgt_txt
		}
	}

	void TargetTimeline::Initialize()
	{
		InitializeTimelineGuiState();

		AudioEngine::GetInstance()->AddCallbackReceiver(this);
		audioController.Initialize();

		InitializeButtonIcons();
		UpdateTimelineMap();
	}

	void TargetTimeline::OnUpdate()
	{
	}

	void TargetTimeline::UpdateTimelineMap()
	{
		chart->GetTimelineMap().CalculateMapTimes(chart->GetTempoMap());
	}

	void TargetTimeline::UpdateOnCallbackSounds()
	{
		if (!GetIsPlayback())
			return;

		for (int i = 0; i < buttonSoundTimesList.size(); ++i)
		{
			if (chartEditor->GetPlaybackTime() >= buttonSoundTimesList[i])
			{
				audioController.PlayButtonSound();
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
		for (size_t i = 0; i < IM_ARRAYSIZE(buttonPlacementMapping); i++)
		{
			buttonPlacementKeyStates[i].WasDown = buttonPlacementKeyStates[i].Down;
			buttonPlacementKeyStates[i].Down = ImGui::IsKeyDown(buttonPlacementMapping[i].Key);

			if (buttonPlacementKeyStates[i].Down && !buttonPlacementKeyStates[i].WasDown)
			{
				EnsureStreamOpenAndRunning();
				audioController.PlayButtonSound();
			}
		}
	}

	void TargetTimeline::InitializeButtonIcons()
	{
		// sankaku		| shikaku		| batsu		 | maru		 | slide_l		| slide_r	   | slide_chain_l		| slide_chain_r
		// sankaku_sync | shikaku_sync  | batsu_sync | maru_sync | slide_l_sync | slide_r_sync | slide_chain_l_sync | slide_chain_r_sync
		{
			std::vector<uint8_t> sprFileBuffer;
			FileSystem::ReadAllBytes("rom/spr/spr_comfy_editor.bin", &sprFileBuffer);

			sprSet.Parse(sprFileBuffer.data());
			sprSet.TxpSet->UploadAll();

			buttonIconsTexture = sprSet.TxpSet->Textures.front()->GraphicsTexture.get();
		}

		const float texelWidth = 1.0f / buttonIconsTexture->GetWidth();
		const float texelHeight = 1.0f / buttonIconsTexture->GetHeight();

		const float width = buttonIconWidth * texelWidth;
		const float height = buttonIconWidth * texelHeight;

		for (size_t i = 0; i < buttonIconsTextureCoordinates.size(); i++)
		{
			float x = (buttonIconWidth * (i % buttonIconsTypeCount)) * texelWidth;
			float y = (buttonIconWidth * (i / buttonIconsTypeCount)) * texelHeight;

			buttonIconsTextureCoordinates[i] = ImRect(x, y, x + width, y + height);
		}
	}

	void TargetTimeline::OnPlaybackResumed()
	{
		EnsureStreamOpenAndRunning();

		buttonSoundTimesList.clear();
		for (const auto &target : chart->GetTargets())
		{
			TimeSpan buttonTime = GetTimelineTime(target.Tick);
			if (buttonTime >= chartEditor->GetPlaybackTime())
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

	void TargetTimeline::OnAudioCallback()
	{
		UpdateOnCallbackSounds();

		if (checkHitsoundsInCallback && updateInput)
			UpdateOnCallbackPlacementSounds();
	}

	void TargetTimeline::OnSongLoaded()
	{
		updateWaveform = true;
	}

	void TargetTimeline::OnDrawTimelineHeaderWidgets()
	{
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(2, 0));

		ImGuiStyle& style = ImGui::GetStyle();
		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(8, style.FramePadding.y));

		strcpy_s(timeInputBuffer, GetCursorTime().FormatTime().c_str());

		ImGui::PushItemWidth(140);
		ImGui::InputTextWithHint("##TargetTimeline::TimeInput", "00:00.000", timeInputBuffer, sizeof(timeInputBuffer));
		ImGui::PopItemWidth();

		ImGui::SameLine();
		ImGui::Button(ICON_FA_FAST_BACKWARD);
		if (ImGui::IsItemActive()) { scrollDelta -= io->DeltaTime * 1000.0f; }

		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));

		// TODO: jump to last / next target
		ImGui::SameLine();
		ImGui::Button(ICON_FA_BACKWARD);
		if (ImGui::IsItemActive()) { scrollDelta -= io->DeltaTime * 400.0f; }

		ImGui::SameLine();

		if (GetIsPlayback())
		{
			if (ImGui::Button(ICON_FA_PAUSE))
				PausePlayback();
		}
		else
		{
			if (ImGui::Button(ICON_FA_PLAY))
				ResumePlayback();
		}

		ImGui::SameLine();
		if (ImGui::Button(ICON_FA_STOP) && GetIsPlayback())
			StopPlayback();

		ImGui::SameLine();
		ImGui::Button(ICON_FA_FORWARD);
		if (ImGui::IsItemActive()) { scrollDelta += io->DeltaTime * 400.0f; }

		ImGui::SameLine();
		ImGui::Button(ICON_FA_FAST_FORWARD);
		if (ImGui::IsItemActive()) { scrollDelta += io->DeltaTime * 1000.0f; }

		ImGui::PopStyleVar(2);

		ImGui::SameLine();
		ImGui::PushItemWidth(280);
		ImGui::SliderFloat(ICON_FA_SEARCH, &zoomLevel, ZOOM_MIN, ZOOM_MAX);
		ImGui::PopItemWidth();

		ImGui::PopStyleVar(1);

		ImGui::SameLine();
		ImGui::PushItemWidth(80);
		{
			if (gridDivisions[gridDivisionIndex] != gridDivision)
				gridDivisionIndex = GetGridDivisionIndex();

			if (ImGui::Combo("Grid Precision", &gridDivisionIndex, gridDivisionStrings.data(), static_cast<int>(gridDivisionStrings.size())))
				gridDivision = gridDivisions[gridDivisionIndex];
		}
		ImGui::PopItemWidth();
	}

	void TargetTimeline::OnDrawTimelineInfoColumnHeader()
	{
		TimelineBase::OnDrawTimelineInfoColumnHeader();
	}

	void TargetTimeline::OnDrawTimelineInfoColumn()
	{
		TimelineBase::OnDrawTimelineInfoColumn();

		auto drawList = ImGui::GetWindowDrawList();
		for (int i = 0; i < TargetType_Max; i++)
		{
			float y = i * ROW_HEIGHT;
			auto start = ImVec2(0, y) + infoColumnRegion.GetTL();
			auto end = ImVec2(infoColumnWidth, y + ROW_HEIGHT) + infoColumnRegion.GetTL();

			auto center = ImVec2((start.x + end.x) / 2.0f, (start.y + end.y) / 2.0f);
			targetYPositions[i] = center.y;

			TimelineTarget target(0, static_cast<TargetType>(i));
			DrawButtonIcon(drawList, target, center, ICON_SCALE);
		}
	}

	void TargetTimeline::OnDrawTimlineRows()
	{
		ImVec2 timelineTL = timelineContentRegion.GetTL();
		ImVec2 timelineWidth = ImVec2(timelineContentRegion.GetWidth(), 0);

		ImU32 rowColor = GetColor(EditorColor_TimelineRowSeparator);

		// Timeline Target Region rows
		// ---------------------------
		for (int t = 0; t <= TargetType_Max; t++)
		{
			ImVec2 start = timelineTL + ImVec2(0, t * ROW_HEIGHT);
			ImVec2 end = start + timelineWidth;

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

			const int totalTicks = GetTimelineTick(chart->GetDuration()).TotalTicks();
			const int tickStep = GetGridTick().TotalTicks();

			const float scrollX = GetScrollX();

			for (int tick = 0, divisions = 0; tick < totalTicks; tick += tickStep)
			{
				bool isBar = tick % (TimelineTick::TICKS_PER_BEAT * 4) == 0;
				if (isBar)
					barCount++;

				float screenX = glm::round(GetTimelinePosition(TimelineTick(tick)) - scrollX);
				TimelineVisibility visiblity = GetTimelineVisibility(screenX);

				if (visiblity == TimelineVisibility::Left)
					continue;
				if (visiblity == TimelineVisibility::Right)
					break;

				const float startYOffset = timelineHeaderHeight * (isBar ? .85f : .35f);
				ImVec2 start = timelineContentRegion.GetTL() + ImVec2(screenX, -startYOffset);
				ImVec2 end = timelineContentRegion.GetBL() + ImVec2(screenX, 0);

				const ImU32 color = isBar ? barColor : (divisions++ % 2 == 0 ? gridColor : gridAltColor);
				baseDrawList->AddLine(start, end, color);

				if (isBar)
				{
					sprintf_s(barStrBuffer, sizeof(barStrBuffer), "%d", barCount);

					start += ImVec2(3, -1);
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
		if (chartEditor->GetSongStream() == nullptr)
			return;

		if (zoomLevelChanged)
			updateWaveform = true;

		if (updateWaveform)
		{
			TimeSpan timePerPixel = GetTimelineTime(2.0f) - GetTimelineTime(1.0f);
			songWaveform.Calculate(chartEditor->GetSongStream(), timePerPixel);
			updateWaveform = false;
		}

		float scrollXStartOffset = GetScrollX() + GetTimelinePosition(chart->GetStartOffset());

		int64_t leftMostVisiblePixel = static_cast<int64_t>(GetTimelinePosition(TimelineTick(0)));
		int64_t rightMostVisiblePixel = leftMostVisiblePixel + static_cast<int64_t>(timelineContentRegion.GetWidth());
		int64_t waveformPixelCount = static_cast<int64_t>(songWaveform.GetPixelCount());

		float timelineX = timelineContentRegion.GetTL().x;
		float timelineHeight = (TargetType_Max * ROW_HEIGHT);
		float timelineCenterY = timelineContentRegion.GetTL().y + (timelineHeight * 0.5f);

		int64_t waveformPixelsDrawn = 0;
		ImU32 waveformColor = GetColor(EditorColor_GridAlt);

		for (int64_t screenPixel = leftMostVisiblePixel; screenPixel < waveformPixelCount && screenPixel < rightMostVisiblePixel; screenPixel++)
		{
			int64_t timelinePixel = std::min(static_cast<int64_t>(screenPixel + scrollXStartOffset), static_cast<int64_t>(waveformPixelCount - 1));

			if (timelinePixel < 0)
				continue;

			float amplitude = songWaveform.GetPcmForPixel(timelinePixel) * timelineHeight;

			float x = screenPixel + timelineX;
			float halfAmplitude = amplitude * 0.5f;

			ImVec2 start = ImVec2(x, timelineCenterY - halfAmplitude);
			ImVec2 end = ImVec2(x, timelineCenterY + halfAmplitude);

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

			for (size_t i = 0; i < chart->GetTempoMap().TempoChangeCount(); i++)
			{
				TempoChange& tempoChange = chart->GetTempoMap().GetTempoChangeAt(i);

				float screenX = glm::round(GetTimelinePosition(tempoChange.Tick) - GetScrollX());

				TimelineVisibility visiblity = GetTimelineVisibility(screenX);
				if (visiblity == TimelineVisibility::Left)
					continue;
				if (visiblity == TimelineVisibility::Right)
					break;

				auto tempoFgColor = IM_COL32(139, 56, 51, 255);

				sprintf_s(tempoStr, sizeof(tempoStr), "%.2f BPM", tempoChange.Tempo.BeatsPerMinute);

				auto buttonPosition = tempoMapRegion.GetTL() + ImVec2(screenX + 1, 0);
				auto buttonSize = ImVec2(ImGui::CalcTextSize(tempoStr).x, tempoMapHeight);

				ImGui::SetCursorScreenPos(buttonPosition);

				ImGui::PushID(&tempoChange);
				ImGui::InvisibleButton("##InvisibleTempoButton", buttonSize);
				ImGui::PopID();

				// prevent overlapping tempo changes
				//windowDrawList->AddRectFilled(buttonPosition, buttonPosition + buttonSize, TEMPO_MAP_BAR_COLOR);
				if (ImGui::IsItemHovered() && ImGui::IsWindowHovered())
				{
					ImGui::WideSetTooltip("TIME: %s", GetTimelineTime(tempoChange.Tick).FormatTime().c_str());

					baseDrawList->AddRect(buttonPosition, buttonPosition + buttonSize, GetColor(EditorColor_TimelineBg));
					if (ImGui::IsMouseDoubleClicked(0))
					{
						SetScrollX(screenX + GetScrollX());
						//SetScrollX(screenX - timelineContentRegion.GetTL().x - (windowWidth * .5f));
					}

					if (ImGui::IsMouseClicked(1))
					{
						ImGui::OpenPopup("##ChangeTempoPopup");
						tempoPopupIndex = static_cast<int>(i);
					}
				}

				baseDrawList->AddLine(buttonPosition + ImVec2(-1, -1), buttonPosition + ImVec2(-1, buttonSize.y - 1), tempoFgColor);
				baseDrawList->AddText(ImGui::GetFont(), tempoMapHeight, buttonPosition, tempoFgColor, tempoStr);
			}

			// Test Popup
			// ----------
			if (ImGui::WideBeginPopup("##ChangeTempoPopup"))
			{
				ImGui::Text("Change Tempo:");

				if (tempoPopupIndex >= 0)
				{
					TempoChange& tempoChange = chart->GetTempoMap().GetTempoChangeAt(tempoPopupIndex);
					float bpm = tempoChange.Tempo.BeatsPerMinute;

					if (ImGui::DragFloat("##TempoDragFloat", &bpm, 1.0f, MIN_BPM, MAX_BPM, "%.2f BPM"))
					{
						tempoChange.Tempo = bpm;
						UpdateTimelineMap();
					}
				}

				ImGui::EndPopup();
			}
			// ----------
		}
	}

	void TargetTimeline::DrawTimelineTargets()
	{
		ImGuiWindow* window = ImGui::GetCurrentWindow();
		ImDrawList* windowDrawList = window->DrawList;

		for (const auto& target : chart->GetTargets())
		{
			TimeSpan buttonTime = GetTimelineTime(target.Tick);
			float screenX = GetTimelinePosition(buttonTime) - GetScrollX();

			TimelineVisibility visiblity = GetTimelineVisibility(screenX);
			if (visiblity == TimelineVisibility::Left)
				continue;
			if (visiblity == TimelineVisibility::Right)
				break;

			ImVec2 center = ImVec2(screenX + timelineContentRegion.GetTL().x, targetYPositions[target.Type]);

			float scale = ICON_SCALE;

			if (GetIsPlayback())
			{
				TimeSpan cursorTime = GetCursorTime();
				TimeSpan timeUntilButton = buttonTime - cursorTime;

				if (timeUntilButton <= 0.0 && timeUntilButton >= -buttonAnimationDuration)
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
			float prePlaybackX = glm::round(GetTimelinePosition(chartEditor->GetPlaybackTimeOnPlaybackStart()) - GetScrollX());

			ImVec2 start = timelineHeaderRegion.GetTL() + ImVec2(prePlaybackX, 0);
			ImVec2 end = timelineContentRegion.GetBL() + ImVec2(prePlaybackX, 0);

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

			if (ImGui::IsMouseClicked(selectionBoxButton) && ImGui::IsWindowFocused() && !ImGui::IsAnyItemHovered())
				dragRect.Min = ImGui::GetMousePos();
			if (ImGui::IsMouseReleased(selectionBoxButton))
				dragRect.Min = dragRect.Max = ImVec2();

			if (!ImGui::IsAnyItemHovered() && ImGui::IsMouseDragging(selectionBoxButton) && dragRect.Min.x != 0)
			{
				dragRect.Max = ImGui::GetMousePos();
				baseDrawList->AddRectFilled(dragRect.GetTL(), dragRect.GetBR(), GetColor(EditorColor_Selection));
			}
		}

		if (!timeSelectionActive)
			return;

		const float scrollX = GetScrollX();

		float startScreenX = GetTimelinePosition(timeSelectionStart) - scrollX;
		float endScreenX = GetTimelinePosition(timeSelectionEnd) - scrollX;

		ImVec2 start = timelineContentRegion.GetTL() + ImVec2(startScreenX, 0);
		ImVec2 end = timelineContentRegion.GetBL() + ImVec2(endScreenX, 0);

		baseDrawList->AddRectFilled(start, end, GetColor(EditorColor_Selection));
		//ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
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
		if (!ImGui::IsWindowFocused() || !timelineContentRegion.Contains(ImGui::GetMousePos()))
			return;

		// Cursor Mouse Click:
		// -------------------
		if (ImGui::IsMouseClicked(0) && !io->KeyShift) // ImGui::IsMouseReleased(0)
		{
			const bool wasPlaying = GetIsPlayback();
			if (wasPlaying)
				chartEditor->PausePlayback();

			const TimelineTick cursorMouseTick = GetCursorMouseXTick();
			TimeSpan previousTime = GetCursorTime();
			TimeSpan newTime = GetTimelineTime(cursorMouseTick);

			if (previousTime == newTime)
				return;

			chartEditor->SetPlaybackTime(newTime);

			if (wasPlaying)
			{
				chartEditor->ResumePlayback();
			}
			else // play a button sound if a target exists at the cursor tick
			{
				for (const auto& target : chart->GetTargets())
				{
					if (target.Tick == cursorMouseTick)
					{
						EnsureStreamOpenAndRunning();
						audioController.PlayButtonSound();

						buttonAnimations[target.Type].Tick = target.Tick;
						buttonAnimations[target.Type].ElapsedTime = 0;
					}
				}
			}
		}

		// Cursor Mouse Drag:
		// ------------------
		if (!GetIsPlayback() && false)
		{
			if (ImGui::IsMouseClicked(0))
			{
				if (io->KeyShift) // && timeSelectionActive)
				{
					timeSelectionEnd = GetCursorMouseXTick();
				}
				else
				{
					timeSelectionActive = false;
					timeSelectionStart = GetCursorMouseXTick();
				}
			}
			if (ImGui::IsMouseDragging(0))
			{
				if (abs(timeSelectionStart.TotalTicks() - timeSelectionEnd.TotalTicks()) > (GetGridTick().TotalTicks() * 2))
					timeSelectionActive = true;

				timeSelectionEnd = GetCursorMouseXTick();
			}
		}
		// ------------------
	}

	void TargetTimeline::UpdateInputTargetPlacement()
	{
		// Mouse X buttons, increase / decrease grid division
		if (ImGui::IsMouseClicked(3)) SelectNextGridDivision(-1);
		if (ImGui::IsMouseClicked(4)) SelectNextGridDivision(+1);

		for (int type = 0; type < TargetType_Max; type++)
			buttonAnimations[type].ElapsedTime += io->DeltaTime;

		TimelineTick cursorTick = RoundToGrid(GetCursorTick());
		for (size_t i = 0; i < IM_ARRAYSIZE(buttonPlacementMapping); i++)
		{
			if (ImGui::IsKeyPressed(buttonPlacementMapping[i].Key, false))
			{
				if (!checkHitsoundsInCallback)
				{
					EnsureStreamOpenAndRunning();
					audioController.PlayButtonSound();
				}
				PlaceOrRemoveTarget(cursorTick, buttonPlacementMapping[i].Type);
			}
		}
	}

	void TargetTimeline::PlaceOrRemoveTarget(TimelineTick tick, TargetType type)
	{
		int64_t existingTargetIndex = chart->GetTargets().FindIndex(tick, type);

		if (existingTargetIndex > -1)
		{
			if (!GetIsPlayback())
				chart->GetTargets().Remove(existingTargetIndex);
		}
		else
		{
			chart->GetTargets().Add(tick, type);
		}

		buttonAnimations[type].Tick = tick;
		buttonAnimations[type].ElapsedTime = 0;
	}

	void TargetTimeline::SelectNextGridDivision(int direction)
	{
		int nextIndex = ImClamp(gridDivisionIndex + direction, 0, static_cast<int>(gridDivisions.size()) - 1);
		gridDivision = gridDivisions[nextIndex];
	}

	TimeSpan TargetTimeline::GetCursorTime() const
	{
		return chartEditor->GetPlaybackTime();
	}

	bool TargetTimeline::GetIsPlayback() const
	{
		return chartEditor->GetIsPlayback();
	}

	void TargetTimeline::PausePlayback()
	{
		chartEditor->PausePlayback();
	}

	void TargetTimeline::ResumePlayback()
	{
		chartEditor->ResumePlayback();
	}

	void TargetTimeline::StopPlayback()
	{
		chartEditor->StopPlayback();
	}

	float TargetTimeline::GetTimelineSize() const
	{
		return GetTimelinePosition(chart->GetDuration());
	}

	void TargetTimeline::OnTimelineBaseScroll()
	{
		if (GetIsPlayback()) // seek through song
		{
			const TimeSpan increment = TimeSpan((io->KeyShift ? 1.0 : 0.5) * io->MouseWheel);

			chartEditor->PausePlayback();
			chartEditor->SetPlaybackTime(chartEditor->GetPlaybackTime() + increment);

			if (chartEditor->GetPlaybackTime() < 0.0)
				chartEditor->SetPlaybackTime(0.0);

			chartEditor->ResumePlayback();

			CenterCursor();
		}
		else
		{
			TimelineBase::OnTimelineBaseScroll();
		}
	}
}