#include "TargetTimeline.h"
#include "TempoMap.h"
#include "Editor/Editor.h"
#include "TimeSpan.h"
#include "Application.h"
#include "Misc/BitFlagsHelper.h"
#include "FileSystem/FileHelper.h"

namespace Editor
{
	TargetTimeline::TargetTimeline(Application* parent, PvEditor* editor) : IEditorComponent(parent, editor)
	{
	}

	TargetTimeline::~TargetTimeline()
	{
	}

	const char* TargetTimeline::GetGuiName() const
	{
		return u8"Target Timeline";
	}

	TimelineTick TargetTimeline::GetGridTick() const
	{
		return (TimelineTick::TICKS_PER_BEAT * 4) / gridDivision;
	}

	TimelineTick TargetTimeline::FloorToGrid(TimelineTick tick) const
	{
		int gridTicks = GetGridTick().TotalTicks();
		return (int)(floor(tick.TotalTicks() / (float)gridTicks) * gridTicks);
	}

	TimelineTick TargetTimeline::RoundToGrid(TimelineTick tick) const
	{
		int gridTicks = GetGridTick().TotalTicks();
		return (int)(round(tick.TotalTicks() / (float)gridTicks) * gridTicks);
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
		return timelineMap.GetTickAt(time);
	}

	TimelineTick TargetTimeline::GetTimelineTick(float position) const
	{
		return GetTimelineTick(GetTimelineTime(position));
	}

	TimeSpan TargetTimeline::GetTimelineTime(TimelineTick tick) const
	{
		return timelineMap.GetTimeAt(tick);
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
		for (size_t i = 0; i < gridDivisions.size(); i++)
		{
			if (gridDivisions[i] == gridDivision)
				return i;
		}

		return -1;
	}

	float TargetTimeline::GetButtonTransparency(float screenX) const
	{
		constexpr float fadeSpan = 35.0f;

		if (screenX < 0.0f || screenX > baseWindow->Size.x)
			return 0.0f;

		const float lowerThreshold = fadeSpan;
		if (screenX < lowerThreshold)
			return ImLerp(0.0f, 1.0f, screenX / lowerThreshold);

		const float upperThreshold = baseWindow->Size.x - fadeSpan;
		if (screenX > upperThreshold)
			return ImLerp(0.0f, 1.0f, 1.0f - ((screenX - upperThreshold) / (baseWindow->Size.x - upperThreshold)));

		return 1.0f;
	}

	int TargetTimeline::GetButtonIconIndex(const TimelineTarget& target) const
	{
		int type = target.Type;

		if (HasFlag(target.Flags, TargetFlags_Chain) && (type == TargetType_SlideL || type == TargetType_SlideR))
			type += 2;

		if (HasFlag(target.Flags, TargetFlags_Sync))
			return type;

		return type + 8;
	}

	void TargetTimeline::DrawButtonIcon(ImDrawList* drawList, const TimelineTarget& target, ImVec2 position, float scale, float transparency)
	{
		const float width = buttonIconWidth * scale;
		const float height = buttonIconWidth * scale;

		position.x -= width * .5f;
		position.y -= height * .5f;

		ImVec2 bottomRight(position.x + width, position.y + height);
		ImRect textureCoordinates = buttonIconsTextureCoordinates[GetButtonIconIndex(target)];

		const ImU32 color = IM_COL32(0xFF, 0xFF, 0xFF, 0xFF * transparency);
		drawList->AddImage(buttonIconsTexture->GetVoidTexture(), position, bottomRight, textureCoordinates.GetBL(), textureCoordinates.GetTR(), color);

		if (HasFlag(target.Flags, TargetFlags_Hold))
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
		timelineMap.CalculateMapTimes(tempoMap);
	}

	void TargetTimeline::UpdateOnCallbackSounds()
	{
		if (!GetIsPlayback())
			return;

		for (int i = 0; i < buttonSoundTimesList.size(); ++i)
		{
			if (pvEditor->GetPlaybackTime() >= buttonSoundTimesList[i])
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
			buttonPlacementKeyStates[i].Down = glfwGetKey(GetParent()->GetWindow(), buttonPlacementMapping[i].Key);

			if (buttonPlacementKeyStates[i].Down && !buttonPlacementKeyStates[i].WasDown)
				audioController.PlayButtonSound();
		}
	}

	void TargetTimeline::DrawGui()
	{
		DrawTimelineGui();

		if (ImGui::Begin("Sync Window"))
			DrawSyncWindow();
		ImGui::End();
	}

	void TargetTimeline::DrawSyncWindow()
	{
		ImGuiWindow* syncWindow = ImGui::GetCurrentWindow();

		ImGui::Text("Adjust Sync:");
		ImGui::Separator();

		float startOffset = pvEditor->songStartOffset.TotalMilliseconds();
		if (ImGui::InputFloat("Offset##test", &startOffset, 1.0f, 10.0f, "%.2f ms"))
			pvEditor->songStartOffset = TimeSpan::FromMilliseconds(startOffset);
		ImGui::Separator();

		static Tempo newTempo = DEFAULT_TEMPO;
		if (ImGui::InputFloat("Tempo##test", &newTempo.BeatsPerMinute, 1.0f, 10.0f, "%.2f BPM"))
			newTempo = std::clamp(newTempo.BeatsPerMinute, MIN_BPM, MAX_BPM);

		const float width = ImGui::CalcItemWidth();

		if (ImGui::Button("Set Tempo Change", ImVec2(width, 0)))
		{
			TimelineTick cursorTick = RoundToGrid(GetCursorTick());

			tempoMap.SetTempoChange(cursorTick, newTempo);
			UpdateTimelineMap();
		}
		if (ImGui::Button("Remove Tempo Change", ImVec2(width, 0)))
		{
			TimelineTick cursorTick = RoundToGrid(GetCursorTick());

			tempoMap.RemoveTempoChange(cursorTick);
			UpdateTimelineMap();
		}
		ImGui::Separator();
	}

	void TargetTimeline::InitializeButtonIcons()
	{
		// sankaku		| shikaku		| batsu		 | maru		 | slide_l		| slide_r	   | slide_chain_l		| slide_chain_r
		// sankaku_sync | shikaku_sync  | batsu_sync | maru_sync | slide_l_sync | slide_r_sync | slide_chain_l_sync | slide_chain_r_sync
		{
			std::vector<uint8_t> sprFileBuffer;
			FileSystem::ReadAllBytes("rom/spr/spr_comfy_editor.bin", &sprFileBuffer);
			
			sprSet.Parse(sprFileBuffer.data());
			for (auto texture : sprSet.TxpSet->Textures)
				texture->UploadTexture2D();

			buttonIconsTexture = sprSet.TxpSet->Textures.front()->Texture2D.get();
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
		buttonSoundTimesList.clear();
		for (const auto &target : targets)
		{
			TimeSpan buttonTime = GetTimelineTime(target.Tick);
			if (buttonTime >= pvEditor->GetPlaybackTime())
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

	void TargetTimeline::OnLoad()
	{
		updateWaveform = true;
	}

	void TargetTimeline::OnDrawTimelineHeaderWidgets()
	{
		static char timeInputBuffer[32] = "00:00.000";
		strcpy_s<sizeof(timeInputBuffer)>(timeInputBuffer, GetCursorTime().FormatTime().c_str());

		ImGui::PushItemWidth(140);
		ImGui::InputTextWithHint("##time_input", "00:00.000", timeInputBuffer, sizeof(timeInputBuffer));
		ImGui::PopItemWidth();

		ImGui::SameLine();
		if (ImGui::Button("Stop") && GetIsPlayback())
			StopPlayback();

		ImGui::SameLine();
		if (ImGui::Button("Pause") && GetIsPlayback())
			PausePlayback();

		ImGui::SameLine();
		if (ImGui::Button("Play") && !GetIsPlayback())
			ResumePlayback();

		ImGui::SameLine();
		ImGui::Button("|<");
		if (ImGui::IsItemActive()) { scrollDelta -= io->DeltaTime * 1000.0f; }

		ImGui::SameLine();
		ImGui::Button(">|");
		if (ImGui::IsItemActive()) { scrollDelta += io->DeltaTime * 1000.0f; }

		ImGui::SameLine();
		ImGui::PushItemWidth(80);
		{
			if (gridDivisions[gridDivisionIndex] != gridDivision)
				gridDivisionIndex = GetGridDivisionIndex();

			if (ImGui::Combo("Grid Precision", &gridDivisionIndex, gridDivisionStrings.data(), gridDivisionStrings.size()))
				gridDivision = gridDivisions[gridDivisionIndex];
		}
		ImGui::PopItemWidth();

		ImGui::SameLine();
		ImGui::PushItemWidth(280);
		ImGui::SliderFloat("Zoom Level", &zoomLevel, ZOOM_MIN, ZOOM_MAX);
		ImGui::PopItemWidth();

		ImGui::SameLine();
		if (ImGui::Button("Load Test Song"))
		{
			pvEditor->Load(testSongPath);
		}
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
		// Timeline Target Region rows
		// ---------------------------
		for (int t = 0; t <= TargetType_Max; t++)
		{
			float y = t * ROW_HEIGHT;
			ImVec2 start = timelineContentRegion.GetTL() + ImVec2(0, y);
			ImVec2 end = start + ImVec2(timelineContentRegion.GetWidth(), 0);

			baseDrawList->AddLine(start, end, GetColor(EditorColor_TimelineRowSeparator));
		}
	}

	void TargetTimeline::OnDrawTimlineDivisors()
	{
		// Timeline Bar / Beat lines
		// -------------------------
		{
			char barStrBuffer[16];
			int barCount = -1;

			const int totalTicks = GetTimelineTick(pvEditor->songDuration).TotalTicks();
			const int tickStep = GetGridTick().TotalTicks();

			const float scrollX = GetScrollX();

			for (int tick = 0, divisions = 0; tick < totalTicks; tick += tickStep)
			{
				bool isBar = tick % (TimelineTick::TICKS_PER_BEAT * 4) == 0;
				if (isBar)
					barCount++;

				float screenX = GetTimelinePosition(TimelineTick(tick)) - scrollX;
				TimelineVisibility visiblity = GetTimelineVisibility(screenX);

				if (visiblity == TimelineVisibility::Left)
					continue;
				if (visiblity == TimelineVisibility::Right)
					break;

				const float startYOffset = timelineHeaderHeight * (isBar ? .85f : .35f);
				ImVec2 start = timelineContentRegion.GetTL() + ImVec2(screenX, -startYOffset);
				ImVec2 end = timelineContentRegion.GetBL() + ImVec2(screenX, 0);

				const ImU32 color = GetColor(isBar ? EditorColor_Bar : (divisions++ % 2 == 0 ? EditorColor_Grid : EditorColor_GridAlt));
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
		// TEST WAVEFORM
		// -------------
		{
			//ImGui::SetTooltip("timleine mouse: %f", ImGui::GetMousePos().x + GetScrollX() - timelineContentRegion.GetTL().x);

			if (pvEditor->songStream != nullptr)
			{
				if (zoomLevelChanged)
					updateWaveform = true;

				if (updateWaveform)
				{
					TimeSpan timePerPixel = GetTimelineTime(1.0f);
					songWaveform.Calculate(pvEditor->songStream.get(), timePerPixel);
					updateWaveform = false;
				}

				ImDrawList* drawList = baseDrawList;

				float scrollX = GetScrollX() + GetTimelinePosition(pvEditor->songStartOffset);
				int64_t leftMostVisiblePixel = 0;
				int64_t rightMostVisiblePixel = static_cast<int64_t>(timelineBaseRegion.GetWidth());
				int64_t pixelCount = songWaveform.GetPixelCount();
				float timelineTargetX = timelineContentRegion.GetTL().x;
				float timelineTargetHeight = (TargetType_Max * ROW_HEIGHT);
				float y = timelineContentRegion.GetTL().y + ((TargetType_Max * ROW_HEIGHT) / 2);

				int linesDrawn = 0;
				for (int64_t screenPixel = leftMostVisiblePixel; screenPixel < songWaveform.GetPixelCount() && screenPixel < rightMostVisiblePixel; screenPixel++)
				{
					size_t timelinePixel = std::min((size_t)(screenPixel + scrollX), (size_t)(pixelCount - 1));

					if (timelinePixel < 0)
						continue;

					float amplitude = songWaveform.GetPcmForPixel(timelinePixel) * timelineTargetHeight;

					float x = screenPixel + timelineTargetX;
					float halfAmplitude = amplitude * .5f;
					ImVec2 start = ImVec2(x, y - halfAmplitude);
					ImVec2 end = ImVec2(x, y + halfAmplitude);

					drawList->AddLine(start, end, GetColor(EditorColor_GridAlt));

					linesDrawn++;
				}

				//Logger::Log("%d line(s) drawn \n", i);
			}
		}
		// -------------
	}

	void TargetTimeline::DrawTimelineTempoMap()
	{
		// Tempo Map Region / Tempo Changes
		// --------------------------------
		{
			char tempoStr[16];
			static int tempoPopupIndex = -1;

			for (size_t i = 0; i < tempoMap.TempoChangeCount(); i++)
			{
				TempoChange& tempoChange = tempoMap.GetTempoChangeAt(i);

				float screenX = GetTimelinePosition(tempoChange.Tick) - GetScrollX();

				TimelineVisibility visiblity = GetTimelineVisibility(screenX);
				if (visiblity == TimelineVisibility::Left)
					continue;
				if (visiblity == TimelineVisibility::Right)
					break;

				//auto tempoBgColor = IM_COL32(147, 125, 125, 255);
				auto tempoFgColor = IM_COL32(139, 56, 51, 255);

				sprintf_s(tempoStr, sizeof(tempoStr), "%.2f BPM", tempoChange.Tempo.BeatsPerMinute);

				auto buttonPosition = tempoMapRegion.GetTL() + ImVec2(screenX + 1, 0);
				auto buttonSize = ImVec2(ImGui::CalcTextSize(tempoStr).x, tempoMapHeight);

				ImGui::SetCursorScreenPos(buttonPosition);

				ImGui::PushID(&tempoChange);
				ImGui::InvisibleButton("##invisible_tempo_button", buttonSize);
				ImGui::PopID();

				// prevent overlapping tempo changes
				//windowDrawList->AddRectFilled(buttonPosition, buttonPosition + buttonSize, TEMPO_MAP_BAR_COLOR);
				if (ImGui::IsItemHovered() && ImGui::IsWindowHovered())
				{
					ImGui::SetTooltip("TIME: %s", GetTimelineTime(tempoChange.Tick).FormatTime().c_str());

					baseDrawList->AddRect(buttonPosition, buttonPosition + buttonSize, GetColor(EditorColor_TimelineBg));
					if (ImGui::IsMouseDoubleClicked(0))
					{
						SetScrollX(screenX + GetScrollX());
						//SetScrollX(screenX - timelineContentRegion.GetTL().x - (windowWidth * .5f));
					}

					if (ImGui::IsMouseClicked(1))
					{
						ImGui::OpenPopup("##change_tempo_popup");
						tempoPopupIndex = static_cast<int>(i);
					}
				}

				baseDrawList->AddLine(buttonPosition + ImVec2(-1, -1), buttonPosition + ImVec2(-1, buttonSize.y - 1), tempoFgColor);
				baseDrawList->AddText(ImGui::GetFont(), tempoMapHeight, buttonPosition, tempoFgColor, tempoStr);
			}

			// Test Popup
			// ----------
			if (ImGui::BeginPopup("##change_tempo_popup"))
			{
				ImGui::Text("Change Tempo:");

				if (tempoPopupIndex >= 0)
				{
					TempoChange& tempoChange = tempoMap.GetTempoChangeAt(tempoPopupIndex);
					float bpm = tempoChange.Tempo.BeatsPerMinute;

					if (ImGui::DragFloat("##bpm_drag_float", &bpm, 1.0f, MIN_BPM, MAX_BPM, "%.2f BPM"))
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

		for (const auto& target : targets)
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
			float prePlaybackX = GetTimelinePosition(pvEditor->playbackTimeOnPlaybackStart) - GetScrollX();

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

			if (ImGui::IsMouseClicked(selectionBoxButton) && ImGui::IsMouseHoveringWindow() && !ImGui::IsAnyItemHovered())
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
		if (!ImGui::IsMouseHoveringWindow() || !timelineContentRegion.Contains(ImGui::GetMousePos()))
			return;

		// Cursor Mouse Click:
		// -------------------
		if (ImGui::IsMouseClicked(0) && !io->KeyShift) // ImGui::IsMouseReleased(0)
		{
			const bool wasPlaying = GetIsPlayback();
			if (wasPlaying)
				pvEditor->PausePlayback();

			const TimelineTick cursorMouseTick = GetCursorMouseXTick();
			TimeSpan previousTime = GetCursorTime();
			TimeSpan newTime = GetTimelineTime(cursorMouseTick);

			if (previousTime == newTime)
				return;

			pvEditor->SetPlaybackTime(newTime);

			if (wasPlaying)
			{
				pvEditor->ResumePlayback();
			}
			else // play a button sound if a target exists at the cursor tick
			{
				for (const auto& target : targets)
				{
					if (target.Tick == cursorMouseTick)
					{
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
					audioController.PlayButtonSound();

				PlaceOrRemoveTarget(cursorTick, buttonPlacementMapping[i].Type);
			}
		}
	}

	void TargetTimeline::PlaceOrRemoveTarget(TimelineTick tick, TargetType type)
	{
		int64_t existingTarget = targets.FindIndex(tick, type);

		if (existingTarget > -1)
		{
			if (!GetIsPlayback())
				targets.Remove(existingTarget);
		}
		else
		{
			targets.Add(tick, type);

			buttonAnimations[type].Tick = tick;
			buttonAnimations[type].ElapsedTime = 0;
		}
	}

	void TargetTimeline::SelectNextGridDivision(int direction)
	{
		int nextIndex = ImClamp(gridDivisionIndex + direction, 0, (int)gridDivisions.size() - 1);
		gridDivision = gridDivisions[nextIndex];
	}

	TimeSpan TargetTimeline::GetCursorTime() const
	{
		return pvEditor->GetPlaybackTime();
	}

	bool TargetTimeline::GetIsPlayback() const
	{
		return pvEditor->GetIsPlayback();
	}

	void TargetTimeline::PausePlayback()
	{
		pvEditor->PausePlayback();
	}

	void TargetTimeline::ResumePlayback()
	{
		pvEditor->ResumePlayback();
	}

	void TargetTimeline::StopPlayback()
	{
		pvEditor->StopPlayback();
	}

	float TargetTimeline::GetTimelineSize() const
	{
		return GetTimelinePosition(pvEditor->songDuration);
	}

	void TargetTimeline::OnTimelineBaseScroll()
	{
		if (GetIsPlayback()) // seek through song
		{
			const TimeSpan increment = TimeSpan((io->KeyShift ? 1.0 : 0.5) * io->MouseWheel);

			pvEditor->PausePlayback();
			pvEditor->SetPlaybackTime(pvEditor->GetPlaybackTime() + increment);

			if (pvEditor->GetPlaybackTime() < 0.0)
				pvEditor->SetPlaybackTime(0.0);

			pvEditor->ResumePlayback();

			CenterCursor();
		}
		else
		{
			TimelineBase::OnTimelineBaseScroll();
		}
	}
}