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
			tempoMap.Add(TempoChange(TimelineTick::FromBars(2), 300.0f));
			tempoMap.Add(TempoChange(TimelineTick::FromBars(6), 060.0f));

			timelineMap = TimelineMap::CalculateMapTimes(tempoMap, 30);
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

		GRID_COLOR = ImColor(ImGui::GetStyle().Colors[ImGuiCol_Separator]);
		auto _gridColor = ImGui::GetStyle().Colors[ImGuiCol_Separator]; _gridColor.w *= .5f;
		GRID_COLOR_ALT = ImColor(_gridColor);
		INFO_COLUMN_COLOR = ImColor(ImGui::GetStyle().Colors[ImGuiCol_ScrollbarBg]); // IM_COL32(0x30, 0x30, 0x30, 0xFF); // ImColor(ImGui::GetStyle().Colors[ImGuiCol_Separator]);
		SELECTION_COLOR = ImColor(ImGui::GetStyle().Colors[ImGuiCol_TextSelectedBg]);
		TIMELINE_ROW_BG_COLOR = ImColor(ImGui::GetStyle().Colors[ImGuiCol_DockingEmptyBg]);

		parentWindow = ImGui::GetCurrentWindow();

		ImGui::BeginGroup();
		TimelineHeaderWidgets();
		ImGui::EndGroup();

		ImGui::BeginGroup();
		TempoMapHeader();
		ImGui::EndGroup();

		auto preColumnCursorPos = ImGui::GetCursorPos();
		{
			ImGui::BeginChild("##timeline_info_column", ImVec2(0, -scrollBarHeight));
			TimelineInfoColumn();
			ImGui::EndChild();
		}
		ImGui::SetCursorPos(preColumnCursorPos);

		ImGui::SetCursorPosX(infoColumnWidth + parentWindow->WindowPadding.x);
		ImGui::BeginChild("##timeline_child", ImVec2(), false, ImGuiWindowFlags_HorizontalScrollbar | ImGuiWindowFlags_AlwaysHorizontalScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
		TimelineChild();
		ImGui::EndChild();
	}

	void TargetTimeline::TimelineHeaderWidgets()
	{
		static char timeInputBuffer[32] = "00:00.000";
		ImGui::PushItemWidth(140);
		ImGui::InputTextWithHint("##time", "00:00.000", timeInputBuffer, sizeof(timeInputBuffer));
		ImGui::PopItemWidth();

		ImGui::SameLine();
		ImGui::Button("Stop");

		ImGui::SameLine();
		ImGui::Button("Pause");

		ImGui::SameLine();
		ImGui::Button("Play");

		ImGui::SameLine();
		ImGui::Button("|<");

		ImGui::SameLine();
		ImGui::Button(">|");

		static const char* gridPrecisions[]{ "1/1", "1/2", "1/4", "1/8", "1/12", "1/16", "1/24", "1/32", "1/48", "1/64" };
		static int gridPrecisionIndex;

		ImGui::SameLine();
		ImGui::PushItemWidth(80);
		ImGui::Combo("Grid Precision", &gridPrecisionIndex, gridPrecisions, IM_ARRAYSIZE(gridPrecisions));
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
		ImVec2 size = ImVec2(parentWindow->Size.x - (parentWindow->WindowPadding.x * 2.0f), tempoMapHeaderHeight);
		ImVec2 end = start + size;

		drawList->AddRectFilled(start, end, INFO_COLUMN_COLOR, 8.0f, ImDrawCornerFlags_TopLeft);
		ImGui::ItemSize(ImRect(start, end - ImVec2(0, parentWindow->WindowPadding.y)));

		// TODO:
	}

	void TargetTimeline::TimelineInfoColumn()
	{
		ImVec2 viewTopLeft = ImGui::GetCursorScreenPos();
		ImVec2 viewBotLeft = viewTopLeft + ImVec2(0.0f, ImGui::GetWindowHeight());
		ImVec2 viewBotRight = viewBotLeft + ImVec2(infoColumnWidth, 0.0f);

		ImGui::GetWindowDrawList()->AddRectFilled(viewTopLeft, viewBotRight, INFO_COLUMN_COLOR);
		parentWindow->DrawList->AddRectFilled(viewBotLeft, viewBotLeft + ImVec2(infoColumnWidth, scrollBarHeight), INFO_COLUMN_COLOR);

		for (int i = 0; i < IM_ARRAYSIZE(timelineRows); i++)
		{
			float y = i * ROW_HEIGHT;
			auto start = ImVec2(0, y) + viewTopLeft;
			auto end = ImVec2(infoColumnWidth, y + ROW_HEIGHT) + viewTopLeft;

			//ImGui::SetCursorPos({ 0, y });
			//ImGui::SetCursorScreenPos({ 0, ImGui::GetCursorScreenPos().y });
			//ImGui::SetCursorScreenPos(start);

			//ImVec2 iconSize = ImVec2(iconTextures[i].GetWidth() * ICON_SIZE, iconTextures[i].GetHeight() * ICON_SIZE);
			//iconSize.x = INFO_COLUMN_WIDTH;
			//ImGui::Image(iconTextures[i].GetVoidTexture(), iconSize, ImVec2(0, 1), ImVec2(1, 0));

			//ImGui::GetForegroundDrawList()->AddRectFilled(start, start + ImVec2(1, 1), IM_COL32_BLACK);
			//ImGui::GetForegroundDrawList()->AddRectFilled(end, end + ImVec2(1, 1), IM_COL32_WHITE);

			auto center = ImVec2((start.x + end.x) / 2.0f, (start.y + end.y) / 2.0f);
			ImGui::AddTexture(ImGui::GetWindowDrawList(), &iconTextures[i], center, ICON_SIZE);
		}
	}

	void TargetTimeline::TimelineChild()
	{
		ImGuiWindow* window = ImGui::GetCurrentWindow();
		ImDrawList* windowDrawList = ImGui::GetWindowDrawList();

		ImVec2 viewTopLeft = ImGui::GetWindowPos() + ImGui::GetCursorPos(); // -ImVec2(BUTTON_STRIP_WIDTH, 0.0f);
		const float windowWidth = ImGui::GetWindowWidth() + infoColumnWidth;
		const float windowHeight = ImGui::GetWindowHeight() - window->ScrollbarSizes.y - 2;

		// Scroll Test
		// -----------
		{
			ImGui::ItemSize(ImVec2(GetTimelinePosition(timelineMap.TimelineLength()), 0));
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
			windowDrawList->AddLine(start, end, GRID_COLOR);

			// Draw timeline row background
			// ----------------------------
			if (i < IM_ARRAYSIZE(timelineRows))
			{
				start.y += 1;
				end.y -= 1;
				end.y += ROW_HEIGHT;
				windowDrawList->AddRectFilled(start, end, TIMELINE_ROW_BG_COLOR);
			}
		}

		// Then draw the buttonstrip
		// -------------------------
		//{
		//	buttonStripDrawList->AddRectFilled(viewTopLeft, viewTopLeft + ImVec2(BUTTON_STRIP_WIDTH, ImGui::GetWindowHeight()), BUTTON_STRIP_COLOR);

		//	// And the icons
		//	// -------------
		//	for (int i = 0; i < TIMELINE_ROWS; i++)
		//	{
		//		float y = i * ROW_HEIGHT;
		//		auto start = ImVec2(0, y) + viewTopLeft;
		//		auto end = ImVec2(BUTTON_STRIP_WIDTH, y) + viewTopLeft;

		//		start.y += 1;
		//		start.x += 1;
		//		end.y += ROW_HEIGHT;
		//		end.x -= 1;
		//		//parentDrawList->AddRectFilled(start, end, IM_COL32(0, 0, 0, 16));
		//		ImGui::AddTexture(buttonStripDrawList, icons[i], ImVec2((start.x + end.x) / 2.0f, (start.y + end.y) / 2.0f), ICON_SIZE);
		//	}

		//	viewTopLeft += ImVec2(BUTTON_STRIP_WIDTH, 0);
		//}

		//static ImVec2 scrolling = ImVec2(0.0f, 0.0f);
		//if (ImGui::IsWindowHovered() && !ImGui::IsAnyItemActive() && ImGui::IsMouseDragging(2, 0.0f))
		//	scrolling = scrolling + ImGui::GetIO().MouseDelta;
		//if (ImGui::IsWindowHovered() && !ImGui::IsAnyItemActive() && ImGui::GetIO().MouseWheel != 0.0f)
		//	scrolling.x -= ImGui::GetIO().MouseWheel * SCROLL_AMOUNT;

		//ImVec2 win_pos = ImGui::GetCursorScreenPos();
		//ImVec2 canvas_sz = ImGui::GetWindowSize();

		//windowDrawList->AddLine(ImVec2(0.0f, 0) + win_pos, ImVec2(canvas_sz.x, 0) + win_pos, GRID_COLOR);
		//for (float x = fmodf(scrolling.x, GRID_SZ); x < canvas_sz.x; x += GRID_SZ)
		//	windowDrawList->AddLine(ImVec2(x, 0.0f) + win_pos, ImVec2(x, canvas_sz.y) + win_pos, GRID_COLOR);

		// Test out bar dividors
		// ---------------------
		{
			for (size_t i = 0; i < timelineMap.TimelineLength().TotalBars(); i++)
			{
				float x = GetTimelinePosition(TimelineTick::FromBars(i)) - ImGui::GetScrollX();

				auto start = ImVec2(x, 0) + viewTopLeft;
				auto end = ImVec2(x, windowHeight) + viewTopLeft;

				windowDrawList->AddLine(start, end, i++ % 2 == 0 ? GRID_COLOR : GRID_COLOR_ALT);
			}

			//float barWidth = 48;
			//int i = 0;
			//for (float x = 0.0f; x <= windowWidth; x += barWidth)
			//{
			//	auto start = ImVec2(x, 0) + viewTopLeft;
			//	auto end = ImVec2(x, windowHeight) + viewTopLeft;

			//	windowDrawList->AddLine(start, end, i++ % 2 == 0 ? GRID_COLOR : GRID_COLOR_ALT);
			//}
		}

		// Test out timline buttons
		// ------------------------
		{
			static int target;
			if (ImGui::IsKeyPressed(GLFW_KEY_UP)) target++;
			if (ImGui::IsKeyPressed(GLFW_KEY_DOWN)) target--;
			if (target > TARGET_SLIDE_R) target = TARGET_SANKAKU;
			if (target < TARGET_SANKAKU) target = TARGET_SLIDE_R;

			ImGui::AddTexture(windowDrawList, &iconTextures[target], viewTopLeft + ImVec2(0, ROW_HEIGHT / 2), ICON_SIZE);
			//ImGui::AddTexture(windowDrawList, &iconTextures[target], ImGui::GetMousePos(), ICON_SIZE);
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


		// Scroll Control
		// --------------
		{
			auto io = ImGui::GetIO();

			if (ImGui::IsWindowHovered() && io.MouseWheel != 0.0f)
			{
				if (io.KeyAlt) // Zoom Timeline
				{
					if (io.MouseWheel > 0.0f)
					{
						zoomLevel += .5f;
					}
					else
					{
						zoomLevel -= .5f;
					}
				}
				else if (!io.KeyCtrl) // Scroll Timeline
				{
					ImVec2 max_step = (window->ContentsRegionRect.GetSize() + window->WindowPadding * 2.0f) * 0.67f;

					if (ImGui::GetIO().KeyShift)
					{
						float scroll_step = ImFloor(ImMin(5 * window->CalcFontSize(), max_step.y));
						ImGui::SetWindowScrollY(window, window->Scroll.y - io.MouseWheel * scroll_step);
					}
					else if (!ImGui::GetIO().KeyShift)
					{
						float scroll_step = ImFloor(ImMin(2 * window->CalcFontSize(), max_step.x));
						ImGui::SetWindowScrollX(window, window->Scroll.x + io.MouseWheel * scroll_step);
					}
				}
			}
		}
	}
}