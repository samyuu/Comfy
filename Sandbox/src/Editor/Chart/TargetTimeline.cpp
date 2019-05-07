#include "TargetTimeline.h"
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

	float TargetTimeline::GetTimelinePosition(TimelineTick tick)
	{
		return timelineMap.GetTimeAt(tick).TotalSeconds() * zoomLevel * ZOOM_BASE;

		//return tick.TotalTicks() * zoomLevel;
	}

	void TargetTimeline::Initialize()
	{
		for (size_t i = 0; i < TARGET_MAX; i++)
		{
			iconTextures[i] = Texture();
			iconTextures[i].LoadFromFile(iconPaths[i]);
		}

		// TIME TEST:
		{
			tempoMap.Add(TempoChange(TimelineTick::FromBars(0), 180.0f));
			tempoMap.Add(TempoChange(TimelineTick::FromBars(2), 200.0f));
			tempoMap.Add(TempoChange(TimelineTick::FromBars(6), 120.0f));
			tempoMap.Add(TempoChange(TimelineTick::FromBars(9), 160.0f));

			timelineMap = TimelineMap::CalculateMapTimes(tempoMap);
		}
	}

	void TargetTimeline::DrawGui()
	{
		if (!initialized)
		{
			Initialize();
			initialized = true;
		}

		if (false)
			ImGui::ShowDemoWindow(nullptr);

		GRID_COLOR = ImGui::GetColorU32(ImGuiCol_Separator, .75f);
		GRID_COLOR_ALT = ImGui::GetColorU32(ImGuiCol_Separator, .5f);
		INFO_COLUMN_COLOR = ImGui::GetColorU32(ImGuiCol_ScrollbarBg);
		SELECTION_COLOR = ImGui::GetColorU32(ImGuiCol_TextSelectedBg);
		TIMELINE_BG_COLOR = ImGui::GetColorU32(ImGuiCol_DockingEmptyBg);
		TIMELINE_ROW_SEPARATOR_COLOR = ImGui::GetColorU32(ImGuiCol_Separator);
		BAR_COLOR = ImGui::GetColorU32(ImGuiCol_PlotLines);

		parentWindow = ImGui::GetCurrentWindow();

		ImGui::BeginGroup();
		TimelineHeaderWidgets();
		ImGui::EndGroup();

		ImGui::BeginGroup();
		TempoMapHeader();
		ImGui::EndGroup();

		auto preColumnCursorPos = ImGui::GetCursorPos();
		{
			ImGui::BeginChild("##timeline_info_column", ImVec2(0, -ImGui::GetStyle().ScrollbarSize));
			TimelineInfoColumn();
			ImGui::EndChild();
		}
		ImGui::SetCursorPos(preColumnCursorPos);

		ImGui::SetCursorPosX(infoColumnWidth + parentWindow->WindowPadding.x);
		ImGui::SetCursorPosY(ImGui::GetCursorPosY() - tempoMapHeaderHeight);
		ImGui::BeginChild("##timeline_child", ImVec2(), false, ImGuiWindowFlags_HorizontalScrollbar | ImGuiWindowFlags_AlwaysHorizontalScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
		TimelineChild();
		ImGui::EndChild();
	}

	void TargetTimeline::TimelineHeaderWidgets()
	{
		static char timeInputBuffer[32] = "00:00.000";
		ImGui::PushItemWidth(140);
		ImGui::InputTextWithHint("##time_input", "00:00.000", timeInputBuffer, sizeof(timeInputBuffer));
		ImGui::PopItemWidth();

		ImGui::SameLine();
		ImGui::Button("Stop");

		ImGui::SameLine();
		ImGui::Button("Pause");

		ImGui::SameLine();
		ImGui::Button("Play");

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
	}

	void TargetTimeline::TempoMapHeader()
	{
		auto drawList = ImGui::GetCurrentWindow()->DrawList;

		ImVec2 start = ImGui::GetCursorScreenPos();
		start.y += ImGui::GetStyle().ItemSpacing.y - parentWindow->WindowPadding.y / 2;

		ImVec2 size = ImVec2(infoColumnWidth, tempoMapHeaderHeight);
		ImVec2 end = start + size;

		drawList->AddRectFilled(start, end, INFO_COLUMN_COLOR, 8.0f, ImDrawCornerFlags_TopLeft);
		ImGui::ItemSize(ImRect(start, end - ImVec2(0, parentWindow->WindowPadding.y)));
	}

	void TargetTimeline::TimelineInfoColumn()
	{
		ImVec2 viewTopLeft = ImGui::GetCursorScreenPos();
		ImVec2 viewBotLeft = viewTopLeft + ImVec2(0.0f, ImGui::GetWindowHeight());
		ImVec2 viewBotRight = viewBotLeft + ImVec2(infoColumnWidth, 0.0f);

		ImGui::GetWindowDrawList()->AddRectFilled(viewTopLeft, viewBotRight, INFO_COLUMN_COLOR);
		parentWindow->DrawList->AddRectFilled(viewBotLeft, viewBotLeft + ImVec2(infoColumnWidth, ImGui::GetStyle().ScrollbarSize), INFO_COLUMN_COLOR);

		for (int i = 0; i < IM_ARRAYSIZE(timelineRows); i++)
		{
			float y = i * ROW_HEIGHT;
			auto start = ImVec2(0, y) + viewTopLeft;
			auto end = ImVec2(infoColumnWidth, y + ROW_HEIGHT) + viewTopLeft;

			auto center = ImVec2((start.x + end.x) / 2.0f, (start.y + end.y) / 2.0f);
			ImGui::AddTexture(ImGui::GetWindowDrawList(), &iconTextures[i], center, ICON_SIZE);

			targetHeights[i] = center.y;
		}
	}

	void TargetTimeline::TimelineChild()
	{
		ImGuiWindow* window = ImGui::GetCurrentWindow();
		ImDrawList* windowDrawList = ImGui::GetWindowDrawList();

		ImVec2 viewTopLeft = ImGui::GetWindowPos() + ImGui::GetCursorPos();
		const float windowWidth = ImGui::GetWindowWidth() + infoColumnWidth;
		const float windowHeight = ImGui::GetWindowHeight() - window->ScrollbarSizes.y - 2;

		// TempoMap Header
		// ---------------
		{
			ImVec2 start = viewTopLeft + ImVec2(0, parentWindow->WindowPadding.y * .5f);
			ImVec2 size = ImVec2(parentWindow->Size.x - (parentWindow->WindowPadding.x * 2.0f) - infoColumnWidth, tempoMapHeaderHeight);
			ImVec2 end = start + size;

			windowDrawList->AddRectFilled(start, end, INFO_COLUMN_COLOR);
			viewTopLeft.y += tempoMapHeaderHeight;
		}

		// Scroll Test
		// -----------
		{
			ImGui::SetScrollX(ImGui::GetScrollX() + scrollDelta);
			scrollDelta = 0;

			ImGui::ItemSize(ImVec2(GetTimelinePosition(TimelineTick::FromBars(30)), 0));
			//ImGui::BeginGroup();
			//window->ScrollbarY = 1000;
			//ImGui::ItemSize(ImVec2(10000, 0));
			//window->SizeContents.y = 10000;
			//ImGui::EndGroup();
		}


		// First draw the timeline row lines
		// ---------------------------------
		for (int i = 0; i <= IM_ARRAYSIZE(timelineRows); i++)
		{
			TimelineRow* timelineRow;

			float y = i * ROW_HEIGHT;
			auto start = ImVec2(0, y) + viewTopLeft;
			auto end = ImVec2(windowWidth, y) + viewTopLeft;

			// Draw timeline row separator
			// ---------------------------
			windowDrawList->AddLine(start, end, TIMELINE_ROW_SEPARATOR_COLOR);

			// Draw timeline row background
			// ----------------------------
			if (i < IM_ARRAYSIZE(timelineRows))
			{
				start.y += 1;
				end.y -= 1;
				end.y += ROW_HEIGHT;
				windowDrawList->AddRectFilled(start, end, TIMELINE_BG_COLOR);
			}
		}

		// Test out bar divisors
		// ---------------------
		{
			char buffer[16];

			int divisions = 0;
			for (int tick = 0; tick < TimelineTick::FromBars(30).TotalTicks(); tick += TimelineTick::TICKS_PER_BAR / gridDivision)
			{
				bool isBar = tick % TimelineTick::TICKS_PER_BAR == 0;
				auto color = isBar ? BAR_COLOR : (divisions++ % 2 == 0 ? GRID_COLOR : GRID_COLOR_ALT);

				float x = GetTimelinePosition(tick) - ImGui::GetScrollX();

				auto start = ImVec2(x, 0) + viewTopLeft;
				if (isBar)
					start.y -= tempoMapHeaderHeight;

				auto end = start + ImVec2(0, windowHeight);

				windowDrawList->AddLine(start, end, color);

				if (isBar)
				{
					sprintf_s(buffer, sizeof(buffer), "%d", TimelineTick::FromTicks(tick).TotalBars());
					windowDrawList->AddText(start, color, buffer);
				}
			}
		}

		// Tempo Changes
		{
			char buffer[16];

			for (size_t i = 0; i < tempoMap.TempoChangeCount(); i++)
			{
				TempoChange& tempoChange = tempoMap.GetTempoChangeAt(i);

				float x = GetTimelinePosition(tempoChange.Tick) - ImGui::GetScrollX();

				auto start = ImVec2(x, -tempoMapHeaderHeight) + viewTopLeft;
				auto end = ImVec2(x, windowHeight) + viewTopLeft;

				auto color = IM_COL32(212, 212, 12, 200);
				windowDrawList->AddLine(start, end, color);

				sprintf_s(buffer, sizeof(buffer), "\n%2.f BPM", tempoChange.Tempo.BeatsPerMinute);
				windowDrawList->AddText(start, color, buffer);
			}
		}

		// Test out timline buttons
		// ------------------------
		{
			static int target;
			if (ImGui::IsKeyPressed(GLFW_KEY_UP)) target++;
			if (ImGui::IsKeyPressed(GLFW_KEY_DOWN)) target--;
			if (target > TARGET_SLIDE_R) target = TARGET_SANKAKU;
			if (target < TARGET_SANKAKU) target = TARGET_SLIDE_R;

			float position = GetTimelinePosition(0) - ImGui::GetScrollX();
			auto center = ImVec2(position + viewTopLeft.x, targetHeights[target]);
			ImGui::AddTexture(windowDrawList, &iconTextures[target], center, ICON_SIZE);
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
				windowDrawList->AddRectFilled(dragRect.GetTL(), dragRect.GetBR(), SELECTION_COLOR);
			}
		}

		// Grab Control
		// ------------
		{
			if (ImGui::IsWindowHovered() && !ImGui::IsAnyItemActive() && ImGui::IsMouseDragging(2, 0.0f))
				ImGui::SetScrollX(ImGui::GetScrollX() - ImGui::GetIO().MouseDelta.x);
		}

		// Scroll Control
		// --------------
		{
			auto io = ImGui::GetIO();

			if (ImGui::IsWindowHovered() && io.MouseWheel != 0.0f)
			{
				if (io.KeyAlt) // Zoom Timeline
				{
					zoomLevel = zoomLevel + (io.MouseWheel > 0.0f ? .5f : -.5f);
				}
				else if (!io.KeyCtrl) // Scroll Timeline
				{
					ImVec2 max_step = (window->ContentsRegionRect.GetSize() + window->WindowPadding * 2.0f) * 0.67f;

					if (io.KeyShift)
					{
						float scroll_step = ImFloor(ImMin(5 * window->CalcFontSize(), max_step.y));
						ImGui::SetWindowScrollY(window, window->Scroll.y - io.MouseWheel * scroll_step);
					}
					else
					{
						float scroll_step = ImFloor(ImMin(2 * window->CalcFontSize(), max_step.x)) * scrollSpeed;
						ImGui::SetWindowScrollX(window, window->Scroll.x + io.MouseWheel * scroll_step);
					}
				}
			}
		}
	}
}