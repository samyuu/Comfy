#include "TargetTimeline.h"
#include <boost/algorithm/string/predicate.hpp>
#include "../../TimeSpan.h"
#include "../../Application.h"
#include "TempoMap.h"

namespace Editor
{
	TargetTimeline::TargetTimeline(Application* parent) : BaseWindow(parent)
	{
	}

	TargetTimeline::~TargetTimeline()
	{
	}

	const char* TargetTimeline::GetGuiName()
	{
		return u8"Target Timeline";
	}

	TimelineTick TargetTimeline::RoundToGrid(TimelineTick tick)
	{
		int gridInterval = TimelineTick::TICKS_PER_BAR / gridDivision;
		return (int)(floor(tick.TotalTicks() / (float)gridInterval) * gridInterval);
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
		return screenPosition - timelineTargetRegion.Min.x + ImGui::GetScrollX();
	}

	void TargetTimeline::Initialize()
	{
		audioEngine = AudioEngine::GetInstance();
		audioEngine->OpenStream();
		audioEngine->StartStream();

		songInstance = std::make_shared<AudioInstance>(&dummySampleProvider);
		audioEngine->AddAudioInstance(songInstance);

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

			tempoMap.Add(TempoChange(TimelineTick::FromBars(0), 180.0f));
			tempoMap.Add(TempoChange(TimelineTick::FromBars(4), 120.0f));

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
		ImGui::ItemSize(ImVec2(GetTimelinePosition(songDuration), 0));
	}

	void TargetTimeline::DrawGui()
	{
		if (!initialized)
		{
			Initialize();
			initialized = true;
		}

		zoomLevelChanged = lastZoomLevel != zoomLevel;
		lastZoomLevel = zoomLevel;

		if (false)
			ImGui::ShowDemoWindow(nullptr);

		UpdateFileDrop();

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

	void TargetTimeline::TimelineHeaderWidgets()
	{
		static char timeInputBuffer[32] = "00:00.000";
		strcpy_s<sizeof(timeInputBuffer)>(timeInputBuffer, playbackTime.FormatTime().c_str());

		ImGui::PushItemWidth(140);
		ImGui::InputTextWithHint("##time_input", "00:00.000", timeInputBuffer, sizeof(timeInputBuffer));
		ImGui::PopItemWidth();

		ImGui::SameLine();
		if (ImGui::Button("Stop") && isPlaying)
			StopPlayback();

		ImGui::SameLine();
		if (ImGui::Button("Pause") && isPlaying)
			PausePlayback();

		ImGui::SameLine();
		if (ImGui::Button("Play") && !isPlaying)
			ResumePlayback();

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
		ImGui::SliderFloat("Zoom", &zoomLevel, ZOOM_MIN, ZOOM_MAX);
		ImGui::PopItemWidth();

		ImGui::SameLine();
		if (ImGui::Button("Load Song"))
		{
			LoadSong(testSongPath);
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

			targetHeights[i] = center.y;
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
				ImGui::SetScrollX(ImGui::GetScrollX() + scrollDelta);
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
		CursorControl();
		ScrollControl();
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

			int totalTicks = GetTimelineTick(songDuration).TotalTicks();
			int tickStep = TimelineTick::TICKS_PER_BAR / gridDivision;

			const float visibleMin = 0, visibleMax = ImGui::GetWindowWidth();
			const float scrollX = ImGui::GetScrollX();

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
			//ImGui::SetTooltip("timleine mouse: %f", ImGui::GetMousePos().x + ImGui::GetScrollX() - timelineTargetRegion.GetTL().x);

			if (songStream != nullptr)
			{
				if (zoomLevelChanged)
					updateWaveform = true;

				if (updateWaveform)
				{
					TimeSpan timePerPixel = GetTimelineTime(1.0f);
					songWaveform.Calculate(songStream.get(), timePerPixel);
					updateWaveform = false;
				}

				ImDrawList* drawList = baseDrawList;

				float scrollX = ImGui::GetScrollX();
				int64_t leftMostVisiblePixel = 0;
				int64_t rightMostVisiblePixel = timelineBaseRegion.GetWidth();
				int64_t pixelCount = songWaveform.GetPixelCount();
				float timelineTargetX = timelineTargetRegion.GetTL().x;
				float timelineTargetHeight = (TARGET_MAX * ROW_HEIGHT) * 10.0f;
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

				float x = GetTimelinePosition(tempoChange.Tick) - ImGui::GetScrollX();

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
						ImGui::SetScrollX(x + ImGui::GetScrollX());
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
		{
			static int target;
			if (ImGui::IsKeyPressed(GLFW_KEY_UP)) target--;
			if (ImGui::IsKeyPressed(GLFW_KEY_DOWN)) target++;
			if (target > TARGET_SLIDE_R) target = TARGET_SANKAKU;
			if (target < TARGET_SANKAKU) target = TARGET_SLIDE_R;

			float position = GetTimelinePosition(TimelineTick(0)) - ImGui::GetScrollX();
			ImVec2 center = ImVec2(position + timelineTargetRegion.GetTL().x, targetHeights[target]);
			ImGui::AddTexture(windowDrawList, &iconTextures[target], center, ICON_SIZE);
		}
	}

	void TargetTimeline::TimelineCursor()
	{
		ImColor outterColor = CURSOR_COLOR;
		ImColor innerColor = ImColor(outterColor.Value.x, outterColor.Value.y, outterColor.Value.z, .5f);

		if (isPlaying)
		{
			float prePlaybackX = GetTimelinePosition(playbackTimeOnPlaybackStart) - ImGui::GetScrollX();

			ImVec2 start = timelineHeaderRegion.GetTL() + ImVec2(prePlaybackX, 0);
			ImVec2 end = timelineTargetRegion.GetBL() + ImVec2(prePlaybackX, 0);

			baseDrawList->AddLine(start, end, innerColor);
		}

		float x = GetTimelinePosition(playbackTime) - ImGui::GetScrollX();

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

	void TargetTimeline::CursorControl()
	{
		if (ImGui::IsWindowFocused())
		{
			if (ImGui::IsMouseHoveringWindow())
			{
				if (ImGui::IsMouseReleased(0) && timelineTargetRegion.Contains(ImGui::GetMousePos()))
				{
					playbackTime = GetTimelineTime(RoundToGrid(GetTimelineTick(ScreenToTimelinePosition(ImGui::GetMousePos().x))));
					//playbackTimeOnPlaybackStart = playbackTime;

					songInstance->SetPosition(playbackTime);
				}
			}

			if (ImGui::IsKeyPressed(GLFW_KEY_SPACE))
			{
				if (isPlaying)
					PausePlayback();
				else
					ResumePlayback();
			}

			if (ImGui::IsKeyPressed(GLFW_KEY_ESCAPE) && isPlaying)
			{
				if (isPlaying)
					StopPlayback();
			}
		}

		if (isPlaying)
		{
			playbackTime += ImGui::GetIO().DeltaTime;
			if (songInstance->GetIsPlaying() && songStream != nullptr)
				playbackTime = songInstance->GetPosition();

			// Scroll Cursor
			{
				float cursorPos = GetTimelinePosition(playbackTime);
				float endPos = ScreenToTimelinePosition(timelineTargetRegion.GetBR().x);

				float autoScrollOffset = timelineTargetRegion.GetWidth() / autoScrollOffsetFraction;
				if (cursorPos > endPos - autoScrollOffset)
					ImGui::SetScrollX(ImGui::GetScrollX() + cursorPos - endPos + autoScrollOffset);
			}
			//printf("cursor pos: %f\n", GetTimelinePosition(cursor.Tick));
		}

	}

	void TargetTimeline::ScrollControl()
	{
		ImGuiWindow* window = ImGui::GetCurrentWindow();

		auto io = ImGui::GetIO();

		// Grab Control
		// ------------
		{
			constexpr int timelineGrabButton = 2;

			if (ImGui::IsWindowHovered() && !ImGui::IsAnyItemActive() && ImGui::IsMouseDragging(timelineGrabButton, 0.0f))
				ImGui::SetScrollX(ImGui::GetScrollX() - io.MouseDelta.x);
		}

		// Scroll Control
		// --------------
		{
			if (ImGui::IsWindowHovered() && io.MouseWheel != 0.0f)
			{
				if (io.KeyAlt) // Zoom Timeline
				{
					float prePosition = GetTimelinePosition(playbackTime);

					float amount = .5f;
					zoomLevel = zoomLevel + amount * io.MouseWheel;

					if (zoomLevel <= 0)
						zoomLevel = amount;

					float postPosition = GetTimelinePosition(playbackTime);
					ImGui::SetScrollX(ImGui::GetScrollX() + postPosition - prePosition);
				}
				else if (!io.KeyCtrl) // Scroll Timeline
				{
					ImVec2 maxStep = (window->ContentsRegionRect.GetSize() + window->WindowPadding * 2.0f) * 0.67f;

					float speed = io.KeyShift ? scrollSpeedFast : scrollSpeed;
					float scrollStep = ImFloor(ImMin(2 * window->CalcFontSize(), maxStep.x)) * speed;
					ImGui::SetWindowScrollX(window, window->Scroll.x + io.MouseWheel * scrollStep);
				}
			}
		}
	}

	void TargetTimeline::ResumePlayback()
	{
		playbackTimeOnPlaybackStart = playbackTime;

		isPlaying = true;
		songInstance->SetIsPlaying(true);
		songInstance->SetPosition(playbackTime);
	}

	void TargetTimeline::PausePlayback()
	{
		songInstance->SetIsPlaying(false);
		isPlaying = false;
	}

	void TargetTimeline::StopPlayback()
	{
		playbackTime = playbackTimeOnPlaybackStart;

		PausePlayback();
	}

	void TargetTimeline::LoadSong(const std::string& path)
	{
		auto newSongStream = std::make_shared<MemoryAudioStream>();
		newSongStream->LoadFromFile(path);

		songInstance->SetSampleProvider(newSongStream.get());
		songDuration = songInstance->GetDuration();

		if (songStream != nullptr)
			songStream->Dispose();

		// adds some copy overhead but we don't want to delete the old pointer while the new one is still loading
		songStream = newSongStream;
		updateWaveform = true;
	}

	void TargetTimeline::UpdateFileDrop()
	{
		Application* parent = GetParent();

		if (!parent->GetDispatchFileDrop())
			return;

		const char* extensions[] = { ".wav", ".flac", ".ogg", ".mp3" };

		auto droppedFiles = parent->GetDroppedFiles();
		for (size_t i = 0; i < droppedFiles->size(); i++)
		{
			const std::string& file = droppedFiles->at(i);

			for (size_t e = 0; e < IM_ARRAYSIZE(extensions); e++)
			{
				if (boost::iends_with(file, extensions[e]))
				{
					LoadSong(file);

					parent->SetFileDropDispatched();
					return;
				}
			}
		}
	}
}