#include "TargetTimeline.h"
#include "../Editor.h"
#include "../../TimeSpan.h"
#include "../../Application.h"
#include "TempoMap.h"

namespace Editor
{
	TargetTimeline::TargetTimeline(Application* parent, PvEditor* editor) : IEditorComponent(parent, editor)
	{
	}

	TargetTimeline::~TargetTimeline()
	{
	}

	const char* TargetTimeline::GetGuiName()
	{
		return u8"Target Timeline";
	}

	TimelineTick TargetTimeline::FloorToGrid(TimelineTick tick)
	{
		int gridInterval = TimelineTick::TICKS_PER_BAR / gridDivision;
		return (int)(floor(tick.TotalTicks() / (float)gridInterval) * gridInterval);
	}

	TimelineTick TargetTimeline::RoundToGrid(TimelineTick tick)
	{
		int gridInterval = TimelineTick::TICKS_PER_BAR / gridDivision;
		return (int)(round(tick.TotalTicks() / (float)gridInterval) * gridInterval);
	}

	float TargetTimeline::GetTimelinePosition(TimeSpan time)
	{
		return time.TotalSeconds() * zoomLevel * ZOOM_BASE;
	}

	float TargetTimeline::GetTimelinePosition(TimelineTick tick)
	{
		return GetTimelinePosition(GetTimelineTime(tick));
	}

	TimelineTick TargetTimeline::GetTimelineTick(TimeSpan time)
	{
		return timelineMap.GetTickAt(time);
	}

	TimelineTick TargetTimeline::GetTimelineTick(float position)
	{
		return GetTimelineTick(GetTimelineTime(position));
	}

	TimeSpan TargetTimeline::GetTimelineTime(TimelineTick tick)
	{
		return timelineMap.GetTimeAt(tick);
	}

	TimeSpan TargetTimeline::GetTimelineTime(float position)
	{
		return position / zoomLevel / ZOOM_BASE;
	}

	float TargetTimeline::ScreenToTimelinePosition(float screenPosition)
	{
		return screenPosition - timelineTargetRegion.Min.x + GetScrollX();
	}

	TimelineTick TargetTimeline::GetCursorTick()
	{
		return GetTimelineTick(GetCursorTime());
	}

	TimeSpan TargetTimeline::GetCursorTime()
	{
		return pvEditor->playbackTime;
	}

	float TargetTimeline::GetCursorTimelinePosition()
	{
		return GetTimelinePosition(GetCursorTime());
	}

	void TargetTimeline::Initialize()
	{
		audioEngine = AudioEngine::GetInstance();
		audioEngine->OpenStream();
		audioEngine->StartStream();

		pvEditor->songInstance = std::make_shared<AudioInstance>(&pvEditor->dummySampleProvider, "PvEditor::SongInstance");
		audioEngine->AddAudioInstance(pvEditor->songInstance);
		audioController.Initialize();

		for (size_t i = 0; i < TARGET_MAX; i++)
		{
			iconTextures[i] = Texture();
			iconTextures[i].LoadFromFile(iconPaths[i]);
		}

		// TIME TEST:
		{
			// - tempoMap.Add(TempoChange(TimelineTick::FromBars(0), 180.0f));
			// - tempoMap.Add(TempoChange(TimelineTick::FromBars(2), 200.0f));
			// - //tempoMap.Add(TempoChange(TimelineTick::FromBeats(2 * 4 + 1), 200.0f));
			// - tempoMap.Add(TempoChange(TimelineTick::FromBars(6), 120.0f));
			// - //tempoMap.Add(TempoChange(TimelineTick::FromBars(9), 160.0f));
			// - tempoMap.Add(TempoChange(TimelineTick::FromBars(9), 180.0f));

			//tempoMap.SetTempoChange(TimelineTick::FromBars(0), 180.0f);
			//tempoMap.SetTempoChange(TimelineTick::FromBars(4), 120.0f);

			UpdateTimelineMap();
		}

		// TEMP DEBUG TEST
		// ---------------
		// - // TimeSpan preTime = glfwGetTime();
		// - // int iterations = 20000;
		// - // for (int i = 0; i < iterations; i++)
		// - // {
		// - // 	TimelineTick inputTick = i;
		// - // 	TimeSpan inputTickTime = GetTimelineTime(inputTick);
		// - // 	TimelineTick outputTick = GetTimelineTick(inputTickTime);
		// - // 
		// - // 	//bool match = inputTick == outputTick;
		// - // 	//if (!match)
		// - // 	//	printf("[%d] match: %s\n", i, match ? "true" : "false");
		// - // }
		// - // TimeSpan elapsed = TimeSpan(glfwGetTime()) - preTime;
		// - // printf("elapsed: %f ms for %d iterations\n", elapsed.TotalMilliseconds(), iterations);
		// ---------------
	}

	void TargetTimeline::UpdateRegions()
	{
		ImVec2 timelinePosition = ImGui::GetCursorScreenPos();
		ImVec2 timelineSize = ImGui::GetWindowSize() - ImGui::GetCursorPos() - ImGui::GetStyle().WindowPadding;
		timelineRegion = ImRect(timelinePosition, timelinePosition + timelineSize);

		ImVec2 headerPosition = timelineRegion.GetTL();
		ImVec2 headerSize = { infoColumnWidth, timelineHeaderHeight + tempoMapHeight };
		infoColumnHeaderRegion = ImRect(headerPosition, headerPosition + headerSize);

		ImVec2 infoPosition = infoColumnHeaderRegion.GetBL();
		ImVec2 infoSize = { infoColumnWidth, timelineRegion.GetHeight() - infoColumnHeaderRegion.GetHeight() };
		infoColumnRegion = ImRect(infoPosition, infoPosition + infoSize);

		ImVec2 timelineBasePosition = infoColumnHeaderRegion.GetTR();
		ImVec2 timelineBaseSize = ImVec2(timelineRegion.GetWidth() - infoColumnRegion.GetWidth(), timelineRegion.GetHeight());
		timelineBaseRegion = ImRect(timelineBasePosition, timelineBasePosition + timelineBaseSize);

		ImVec2 tempoMapPosition = timelineBaseRegion.GetTL();
		ImVec2 tempoMapSize = ImVec2(timelineBaseRegion.GetWidth(), tempoMapHeight);
		tempoMapRegion = ImRect(tempoMapPosition, tempoMapPosition + tempoMapSize);

		ImVec2 timelineHeaderPosition = tempoMapRegion.GetBL();
		ImVec2 timelineHeaderSize = ImVec2(timelineBaseRegion.GetWidth(), timelineHeaderHeight);
		timelineHeaderRegion = ImRect(timelineHeaderPosition, timelineHeaderPosition + timelineHeaderSize);

		ImVec2 timelineTargetPosition = timelineHeaderRegion.GetBL();
		ImVec2 timelineTargetSize = ImVec2(timelineBaseRegion.GetWidth(), timelineBaseRegion.GetHeight() - timelineHeaderSize.y - tempoMapSize.y - ImGui::GetStyle().ScrollbarSize);
		timelineTargetRegion = ImRect(timelineTargetPosition, timelineTargetPosition + timelineTargetSize);
	}

	void TargetTimeline::UpdateTimelineMap()
	{
		timelineMap.CalculateMapTimes(tempoMap);
	}

	void TargetTimeline::UpdateTimelineSize()
	{
		ImGui::ItemSize(ImVec2(GetTimelinePosition(pvEditor->songDuration), 0));
	}

	void TargetTimeline::DrawGui()
	{
		if (ImGui::Begin("target list"))
		{
			// for (auto target : targets)
			// 	ImGui::Text("%d : %d", target.Tick.TotalTicks(), target.Type);

			TimelineTick cursorTick = RoundToGrid(GetCursorTick());
			static Tempo newTempo = 180.0f;
			ImGui::DragFloat("Tempo##test", &newTempo.BeatsPerMinute, 1.0f, MIN_BPM, MAX_BPM, "%.2f BPM");

			if (ImGui::Button("Add Tempo Change"))
			{
				tempoMap.SetTempoChange(cursorTick, newTempo);
				UpdateTimelineMap();
			}

			if (ImGui::Button("Remove Tempo Change"))
			{
				tempoMap.RemoveTempoChange(cursorTick);
				UpdateTimelineMap();
			}

		}
		ImGui::End();

		zoomLevelChanged = lastZoomLevel != zoomLevel;
		lastZoomLevel = zoomLevel;

		if (false)
			ImGui::ShowDemoWindow(nullptr);

		GRID_COLOR = ImGui::GetColorU32(ImGuiCol_Separator, .75f);
		GRID_COLOR_ALT = ImGui::GetColorU32(ImGuiCol_Separator, .5f);
		INFO_COLUMN_COLOR = ImGui::GetColorU32(ImGuiCol_ScrollbarBg);
		TEMPO_MAP_BG_COLOR = ImGui::GetColorU32(ImGuiCol_MenuBarBg);
		SELECTION_COLOR = ImGui::GetColorU32(ImGuiCol_TextSelectedBg);
		TIMELINE_BG_COLOR = ImGui::GetColorU32(ImGuiCol_DockingEmptyBg);
		TIMELINE_ROW_SEPARATOR_COLOR = ImGui::GetColorU32(ImGuiCol_Separator);
		BAR_COLOR = ImGui::GetColorU32(ImGuiCol_PlotLines);

		ImGui::BeginGroup();
		TimelineHeaderWidgets();
		ImGui::EndGroup();

		UpdateRegions();

		ImGui::BeginChild("##timeline_info_column", ImVec2(0, -ImGui::GetStyle().ScrollbarSize));
		TimelineInfoColumnHeader();
		TimelineInfoColumn();
		ImGui::EndChild();

		ImGui::SetCursorScreenPos(infoColumnHeaderRegion.GetTR());
		ImGui::BeginChild("##timeline_base", ImVec2(), false, ImGuiWindowFlags_HorizontalScrollbar | ImGuiWindowFlags_AlwaysHorizontalScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
		TimelineBase();
		ImGui::EndChild();
	}

	void TargetTimeline::OnPlaybackResumed()
	{
		buttonSoundTimesList.clear();
		for (const auto &target : targets)
		{
			TimeSpan buttonTime = GetTimelineTime(target.Tick);
			if (buttonTime >= pvEditor->playbackTime)
				buttonSoundTimesList.push_back(buttonTime);
		}
	}

	void TargetTimeline::OnLoad()
	{
		updateWaveform = true;
	}

	void TargetTimeline::TimelineHeaderWidgets()
	{
		static char timeInputBuffer[32] = "00:00.000";
		strcpy_s<sizeof(timeInputBuffer)>(timeInputBuffer, GetCursorTime().FormatTime().c_str());

		ImGui::PushItemWidth(140);
		ImGui::InputTextWithHint("##time_input", "00:00.000", timeInputBuffer, sizeof(timeInputBuffer));
		ImGui::PopItemWidth();

		ImGui::SameLine();
		if (ImGui::Button("Stop") && pvEditor->isPlaying)
			pvEditor->StopPlayback();

		ImGui::SameLine();
		if (ImGui::Button("Pause") && pvEditor->isPlaying)
			pvEditor->PausePlayback();

		ImGui::SameLine();
		if (ImGui::Button("Play") && !pvEditor->isPlaying)
			pvEditor->ResumePlayback();

		ImGui::SameLine();
		ImGui::Button("|<");
		if (ImGui::IsItemActive()) { scrollDelta -= ImGui::GetIO().DeltaTime * 1000.0f; }

		ImGui::SameLine();
		ImGui::Button(">|");
		if (ImGui::IsItemActive()) { scrollDelta += ImGui::GetIO().DeltaTime * 1000.0f; }

		ImGui::SameLine();
		ImGui::PushItemWidth(80);
		{
			static const char* gridDivisionStrings[] = { "1/1", "1/2", "1/4", "1/8", "1/12", "1/16", "1/24", "1/32", "1/48", "1/64" };
			static int gridDivisions[] = { 1, 2, 4, 8, 12, 16, 24, 32, 48, 64 };
			static int gridDivisionIndex = -1;

			if (gridDivisionIndex < 0)
			{
				for (size_t i = 0; i < IM_ARRAYSIZE(gridDivisions); i++)
				{
					if (gridDivisions[i] == gridDivision)
						gridDivisionIndex = i;
				}
			}

			if (ImGui::Combo("Grid Precision", &gridDivisionIndex, gridDivisionStrings, IM_ARRAYSIZE(gridDivisionStrings)))
				gridDivision = gridDivisions[gridDivisionIndex];
		}
		ImGui::PopItemWidth();

		ImGui::SameLine();
		ImGui::PushItemWidth(280);
		ImGui::SliderFloat("Zoom Level", &zoomLevel, ZOOM_MIN, ZOOM_MAX);
		ImGui::PopItemWidth();

		ImGui::SameLine();
		if (ImGui::Button("Load Song"))
		{
			pvEditor->Load(testSongPath);
		}
	}

	void TargetTimeline::TimelineInfoColumnHeader()
	{
		auto drawList = ImGui::GetWindowDrawList();

		drawList->AddRectFilled(infoColumnHeaderRegion.GetTL(), infoColumnHeaderRegion.GetBR(), INFO_COLUMN_COLOR, 8.0f, ImDrawCornerFlags_TopLeft);
	}

	void TargetTimeline::TimelineInfoColumn()
	{
		auto drawList = ImGui::GetWindowDrawList();

		// top part
		drawList->AddRectFilled(infoColumnRegion.GetTL(), infoColumnRegion.GetBR(), INFO_COLUMN_COLOR);

		// bottom part
		ImDrawList* parentDrawList = ImGui::GetCurrentWindow()->ParentWindow->DrawList;
		parentDrawList->AddRectFilled(infoColumnRegion.GetBL(), infoColumnRegion.GetBL() + ImVec2(infoColumnRegion.GetWidth(), -ImGui::GetStyle().ScrollbarSize), INFO_COLUMN_COLOR);

		for (int i = 0; i < TARGET_MAX; i++)
		{
			float y = i * ROW_HEIGHT;
			auto start = ImVec2(0, y) + infoColumnRegion.GetTL();
			auto end = ImVec2(infoColumnWidth, y + ROW_HEIGHT) + infoColumnRegion.GetTL();

			auto center = ImVec2((start.x + end.x) / 2.0f, (start.y + end.y) / 2.0f);
			ImGui::AddTexture(drawList, &iconTextures[i], center, ICON_SIZE);

			targetYPositions[i] = center.y;
		}
	}

	void TargetTimeline::TimelineBase()
	{
		baseWindow = ImGui::GetCurrentWindow();
		baseDrawList = baseWindow->DrawList;

		// Scroll Size Test
		// ----------------
		{
			if (scrollDelta != 0.0f)
			{
				ImGui::SetScrollX(GetScrollX() + scrollDelta);
				scrollDelta = 0.0f;
			}

			UpdateTimelineSize();
			//ImGui::ItemSize(ImVec2(GetTimelinePosition(TimelineTick::FromBars(30)), 0));
		}

		// Timeline Header Region BG
		// -------------------------
		{
			baseDrawList->AddRectFilled(timelineHeaderRegion.GetTL(), timelineHeaderRegion.GetBR(), INFO_COLUMN_COLOR);
		}
		// Timeline Target Region BG
		// -------------------------
		{
			baseDrawList->AddRectFilled(timelineTargetRegion.GetTL(), timelineTargetRegion.GetBR(), TIMELINE_BG_COLOR);
		}

		TimlineDivisors();
		TimelineTempoMap();

		if (timelineHeaderRegion.Contains(ImGui::GetMousePos()))
		{
			ImGui::SetTooltip("TIME: %s", GetTimelineTime(ScreenToTimelinePosition(ImGui::GetMousePos().x)).FormatTime().c_str());
		}

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
				baseDrawList->AddRectFilled(dragRect.GetTL(), dragRect.GetBR(), SELECTION_COLOR);
			}
		}

		TimelineTargets();
		TimelineCursor();
		Update();
		UpdateInput();
	}

	void TargetTimeline::TimlineDivisors()
	{
		// Timeline Target Region rows
		// ---------------------------
		{
			for (int t = 0; t <= TARGET_MAX; t++)
			{
				float y = t * ROW_HEIGHT;
				ImVec2 start = timelineTargetRegion.GetTL() + ImVec2(0, y);
				ImVec2 end = start + ImVec2(timelineTargetRegion.GetWidth(), 0);

				baseDrawList->AddLine(start, end, TIMELINE_ROW_SEPARATOR_COLOR);
			}
		}

		// Timeline Bar / Beat lines
		// -------------------------
		{
			baseDrawList->AddRectFilled(tempoMapRegion.GetTL(), tempoMapRegion.GetBR(), TEMPO_MAP_BG_COLOR);

			char barStrBuffer[16];

			int totalTicks = GetTimelineTick(pvEditor->songDuration).TotalTicks();
			int tickStep = TimelineTick::TICKS_PER_BAR / gridDivision;

			const float visibleMin = 0, visibleMax = ImGui::GetWindowWidth();
			const float scrollX = GetScrollX();

			for (int tick = 0, divisions = 0; tick < totalTicks; tick += tickStep)
			{
				float screenX = GetTimelinePosition(TimelineTick(tick)) - scrollX;

				// skip everything to the left of the screen
				if (screenX < visibleMin)
					continue;
				// break once we reach the right side
				if (screenX > visibleMax)
					break;

				ImVec2 start = timelineTargetRegion.GetTL() + ImVec2(screenX, 0);
				ImVec2 end = timelineTargetRegion.GetBL() + ImVec2(screenX, 0);

				bool isBar = tick % TimelineTick::TICKS_PER_BAR == 0;
				auto color = isBar ? BAR_COLOR : (divisions++ % 2 == 0 ? GRID_COLOR : GRID_COLOR_ALT);

				start.y -= timelineHeaderHeight * (isBar ? .85f : .35f);

				baseDrawList->AddLine(start, end, color);

				if (isBar)
				{
					sprintf_s(barStrBuffer, sizeof(barStrBuffer), "%d", TimelineTick::FromTicks(tick).TotalBars());

					start += ImVec2(3, -1);
					baseDrawList->AddText(start, color, barStrBuffer);
				}
			}
		}

		// TEST WAVEFORM
		// -------------
		{
			//ImGui::SetTooltip("timleine mouse: %f", ImGui::GetMousePos().x + GetScrollX() - timelineTargetRegion.GetTL().x);

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

				float scrollX = GetScrollX();
				int64_t leftMostVisiblePixel = 0;
				int64_t rightMostVisiblePixel = timelineBaseRegion.GetWidth();
				int64_t pixelCount = songWaveform.GetPixelCount();
				float timelineTargetX = timelineTargetRegion.GetTL().x;
				float timelineTargetHeight = (TARGET_MAX * ROW_HEIGHT);
				float y = timelineTargetRegion.GetTL().y + ((TARGET_MAX * ROW_HEIGHT) / 2);

				int linesDrawn = 0;
				for (int64_t screenPixel = leftMostVisiblePixel; screenPixel < songWaveform.GetPixelCount() && screenPixel < rightMostVisiblePixel; screenPixel++)
				{
					float timelinePixel = std::clamp((int64_t)(screenPixel + scrollX), (int64_t)0, (int64_t)pixelCount - 1);
					float amplitude = songWaveform.GetPcmForPixel(timelinePixel) * timelineTargetHeight;

					float x = screenPixel + timelineTargetX;
					float halfAmplitude = amplitude * .5f;
					ImVec2 start = ImVec2(x, y - halfAmplitude);
					ImVec2 end = ImVec2(x, y + halfAmplitude);

					drawList->AddLine(start, end, GRID_COLOR_ALT);

					linesDrawn++;
				}

				//printf("%d line(s) drawn \n", i);
			}
		}
		// -------------
	}

	void TargetTimeline::TimelineTempoMap()
	{
		// Tempo Map Region / Tempo Changes
		// --------------------------------
		{
			char tempoStr[16];
			char buttonIdStr[28];
			static int tempoPopupIndex = -1;

			for (size_t i = 0; i < tempoMap.TempoChangeCount(); i++)
			{
				TempoChange& tempoChange = tempoMap.GetTempoChangeAt(i);

				float x = GetTimelinePosition(tempoChange.Tick) - GetScrollX();

				//auto tempoBgColor = IM_COL32(147, 125, 125, 255);
				auto tempoFgColor = IM_COL32(139, 56, 51, 255);

				sprintf_s(tempoStr, sizeof(tempoStr), "%2.f BPM", tempoChange.Tempo.BeatsPerMinute);
				sprintf_s(buttonIdStr, sizeof(buttonIdStr), "##%s_%04d", tempoStr, i);

				auto buttonPosition = tempoMapRegion.GetTL() + ImVec2(x + 1, 0);
				auto buttonSize = ImVec2(ImGui::CalcTextSize(tempoStr).x, tempoMapHeight);

				ImGui::SetCursorScreenPos(buttonPosition);
				ImGui::InvisibleButton(buttonIdStr, buttonSize);

				// prevent overlapping tempo changes
				//windowDrawList->AddRectFilled(buttonPosition, buttonPosition + buttonSize, TEMPO_MAP_BAR_COLOR);
				if (ImGui::IsItemHovered())
				{
					ImGui::SetTooltip("TIME: %s", GetTimelineTime(tempoChange.Tick).FormatTime().c_str());

					baseDrawList->AddRect(buttonPosition, buttonPosition + buttonSize, TIMELINE_BG_COLOR);
					if (ImGui::IsMouseDoubleClicked(0))
					{
						ImGui::SetScrollX(x + GetScrollX());
						//ImGui::SetScrollX(screenX - timelineTargetRegion.GetTL().x - (windowWidth * .5f));
					}

					if (ImGui::IsMouseClicked(1))
					{
						ImGui::OpenPopup("##change_tempo_popup");
						tempoPopupIndex = i;
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

	void TargetTimeline::TimelineTargets()
	{
		ImGuiWindow* window = ImGui::GetCurrentWindow();
		ImDrawList* windowDrawList = window->DrawList;

		// Test out timline targets
		// ------------------------
		for (const auto& target : targets)
		{
			float position = GetTimelinePosition(TimelineTick(target.Tick)) - GetScrollX();
			ImVec2 center = ImVec2(position + timelineTargetRegion.GetTL().x, targetYPositions[target.Type]);

			ImGui::AddTexture(windowDrawList, &iconTextures[target.Type], center, ICON_SIZE);
		}
	}

	void TargetTimeline::TimelineCursor()
	{
		ImColor outterColor = CURSOR_COLOR;
		ImColor innerColor = ImColor(outterColor.Value.x, outterColor.Value.y, outterColor.Value.z, .5f);

		if (pvEditor->isPlaying)
		{
			float prePlaybackX = GetTimelinePosition(pvEditor->playbackTimeOnPlaybackStart) - GetScrollX();

			ImVec2 start = timelineHeaderRegion.GetTL() + ImVec2(prePlaybackX, 0);
			ImVec2 end = timelineTargetRegion.GetBL() + ImVec2(prePlaybackX, 0);

			baseDrawList->AddLine(start, end, innerColor);
		}

		float x = GetCursorTimelinePosition() - GetScrollX();

		ImVec2 start = timelineHeaderRegion.GetTL() + ImVec2(x, 0);
		ImVec2 end = timelineTargetRegion.GetBL() + ImVec2(x, 0);

		baseDrawList->AddLine(start + ImVec2(0, CURSOR_HEAD_HEIGHT - 1), end, outterColor);

		float centerX = start.x + .5f;
		ImVec2 cursorTriangle[3] =
		{
			ImVec2(centerX - CURSOR_HEAD_WIDTH * .5f, start.y),
			ImVec2(centerX + CURSOR_HEAD_WIDTH * .5f, start.y),
			ImVec2(centerX, start.y + CURSOR_HEAD_HEIGHT),
		};
		baseDrawList->AddTriangleFilled(cursorTriangle[0], cursorTriangle[1], cursorTriangle[2], innerColor);
		baseDrawList->AddTriangle(cursorTriangle[0], cursorTriangle[1], cursorTriangle[2], outterColor);
	}

	void TargetTimeline::Update()
	{
		if (pvEditor->isPlaying)
		{
			// Scroll Cursor
			{
				float cursorPos = GetCursorTimelinePosition();
				float endPos = ScreenToTimelinePosition(timelineTargetRegion.GetBR().x);

				float autoScrollOffset = timelineTargetRegion.GetWidth() / autoScrollOffsetFraction;
				if (cursorPos > endPos - autoScrollOffset)
					ImGui::SetScrollX(GetScrollX() + cursorPos - endPos + autoScrollOffset);
			}

			for (int i = 0; i < buttonSoundTimesList.size(); ++i)
			{
				if (pvEditor->playbackTime >= buttonSoundTimesList[i])
				{
					audioController.PlayButtonSound();
					buttonSoundTimesList.erase(buttonSoundTimesList.begin() + i);
					--i;
				}
			}
		}
	}

	void TargetTimeline::UpdateInput()
	{
		if (!ImGui::IsWindowFocused())
			return;

		if (ImGui::IsKeyPressed(GLFW_KEY_SPACE))
		{
			if (pvEditor->isPlaying)
				pvEditor->PausePlayback();
			else
				pvEditor->ResumePlayback();
		}

		if (ImGui::IsKeyPressed(GLFW_KEY_ESCAPE) && pvEditor->isPlaying)
		{
			if (pvEditor->isPlaying)
				pvEditor->StopPlayback();
		}

		UpdateCursorControl();
		UpdateScrollControl();

		// Target Placement Test:
		// ----------------------
		{
			TimelineTick cursorTick = RoundToGrid(GetCursorTick());

			auto placeTarget = [&](TargetType type)
			{
				audioController.PlayButtonSound();

				TargetIterator existingTarget = targets.Find(cursorTick, type);
				if (existingTarget != targets.end())
				{
					if (!pvEditor->isPlaying)
						targets.Remove(existingTarget);
				}
				else
				{
					targets.Add(cursorTick, type);
				}
			};

			static struct { TargetType Type; int Key; } mapping[4]
			{
				{ TARGET_SANKAKU, 'W'},
				{ TARGET_SHIKAKU, 'A'},
				{ TARGET_BATSU, 'S'},
				{ TARGET_MARU, 'D'},
			};

			for (size_t i = 0; i < IM_ARRAYSIZE(mapping); i++)
			{
				if (ImGui::IsKeyPressed(mapping[i].Key, false))
					placeTarget(mapping[i].Type);
			}
		}
	}

	void TargetTimeline::UpdateCursorControl()
	{
		if (ImGui::IsMouseHoveringWindow())
		{
			if (ImGui::IsMouseClicked(0) /*ImGui::IsMouseReleased(0)*/ && timelineTargetRegion.Contains(ImGui::GetMousePos()))
			{
				bool wasPlaying = pvEditor->isPlaying;
				if (wasPlaying)
					pvEditor->PausePlayback();

				TimelineTick cursorTick = FloorToGrid(GetTimelineTick(ScreenToTimelinePosition(ImGui::GetMousePos().x)));
				TimeSpan previousTime = pvEditor->playbackTime;
				TimeSpan newTime = GetTimelineTime(cursorTick);

				if (previousTime == newTime)
					return;

				pvEditor->playbackTime = newTime;
				pvEditor->songInstance->SetPosition(pvEditor->playbackTime);

				if (wasPlaying)
				{
					pvEditor->ResumePlayback();
				}
				else // play a button sound if a target exists at the cursor tick
				{
					for (const auto& target : targets)
					{
						if (target.Tick == cursorTick)
							audioController.PlayButtonSound();
					}
				}
			}
		}
	}

	void TargetTimeline::UpdateScrollControl()
	{
		auto io = ImGui::GetIO();

		// Grab Control
		// ------------
		{
			constexpr int timelineGrabButton = 2;

			if (ImGui::IsWindowHovered() && !ImGui::IsAnyItemActive() && ImGui::IsMouseDragging(timelineGrabButton, 0.0f))
				ImGui::SetScrollX(GetScrollX() - io.MouseDelta.x);
		}

		// Focus Control
		// -------------
		{
			if (ImGui::IsKeyPressed(GLFW_KEY_ESCAPE))
				CenterCursor();
		}
		// -------------

		// Scroll Control
		// --------------
		{
			if (ImGui::IsWindowHovered() && io.MouseWheel != 0.0f)
			{
				if (io.KeyAlt) // Zoom Timeline
				{
					float prePosition = GetCursorTimelinePosition();

					float amount = .5f;
					zoomLevel = zoomLevel + amount * io.MouseWheel;

					if (zoomLevel <= 0)
						zoomLevel = amount;

					float postPosition = GetCursorTimelinePosition();
					ImGui::SetScrollX(GetScrollX() + postPosition - prePosition);
				}
				else if (!io.KeyCtrl) // Scroll Timeline
				{
					if (pvEditor->isPlaying) // seek through song
					{
						const TimeSpan increment = TimeSpan((io.KeyShift ? 1.0 : 0.5) * io.MouseWheel);

						pvEditor->PausePlayback();
						pvEditor->playbackTime += increment;
						if (pvEditor->playbackTime < 0.0)
							pvEditor->playbackTime = 0.0;
						pvEditor->ResumePlayback();

						CenterCursor();
					}
					else
					{
						ImVec2 maxStep = (baseWindow->ContentsRegionRect.GetSize() + baseWindow->WindowPadding * 2.0f) * 0.67f;

						float speed = io.KeyShift ? scrollSpeedFast : scrollSpeed;
						float scrollStep = ImFloor(ImMin(2 * baseWindow->CalcFontSize(), maxStep.x)) * speed;
						ImGui::SetWindowScrollX(baseWindow, baseWindow->Scroll.x + io.MouseWheel * scrollStep);
					}
				}
			}
		}
	}

	void TargetTimeline::CenterCursor()
	{
		float center = GetCursorTimelinePosition() - (timelineTargetRegion.GetWidth() / 2.0f);
		ImGui::SetScrollX(center);
	}
}