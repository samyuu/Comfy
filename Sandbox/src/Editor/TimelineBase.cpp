#include "TimelineBase.h"
#include "Theme.h"

namespace Editor
{
	float TimelineBase::GetTimelinePosition(TimeSpan time) const
	{
		return time.TotalSeconds() * zoomLevel * ZOOM_BASE;
	}

	TimeSpan TimelineBase::GetTimelineTime(float position) const
	{
		return position / zoomLevel / ZOOM_BASE;
	}

	float TimelineBase::ScreenToTimelinePosition(float screenPosition) const
	{
		return screenPosition - timelineContentRegion.Min.x + GetScrollX();
	}

	float TimelineBase::GetCursorTimelinePosition() const
	{
		return GetTimelinePosition(GetCursorTime());
	}

	TimeSpan TimelineBase::GetCursorTime() const
	{
		return cursorTime;
	}

	TimelineVisibility TimelineBase::GetTimelineVisibility(float screenX) const
	{
		const float visibleMin = -timelineVisibleThreshold;
		const float visibleMax = baseWindow->Size.x + timelineVisibleThreshold;

		if (screenX < visibleMin)
			return TimelineVisibility::Left;

		if (screenX > visibleMax)
			return TimelineVisibility::Right;

		return TimelineVisibility::Visible;
	}

	void TimelineBase::DrawTimelineCursor()
	{
		ImU32 outterColor = GetColor(EditorColor_Cursor);
		ImU32 innerColor = GetColor(EditorColor_CursorInner);

		const float scrollX = GetScrollX();
		const float cursorX = GetCursorTimelinePosition();
		float cursorScreenX = cursorX - scrollX;

		// ensure smooth cursor scrolling
		if (autoScrollCursor)
			cursorScreenX = timelineContentRegion.GetWidth() - (timelineContentRegion.GetWidth() / autoScrollOffsetFraction);

		ImVec2 start = timelineHeaderRegion.GetTL() + ImVec2(cursorScreenX, 0);
		ImVec2 end = timelineContentRegion.GetBL() + ImVec2(cursorScreenX, 0);

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

	void TimelineBase::InitializeTimelineGuiState()
	{
		io = &ImGui::GetIO();
	}

	void TimelineBase::UpdateTimelineBaseState()
	{
		baseWindow = ImGui::GetCurrentWindow();
		baseDrawList = baseWindow->DrawList;

		zoomLevelChanged = lastZoomLevel != zoomLevel;
		lastZoomLevel = zoomLevel;
	}

	void TimelineBase::UpdateTimelineRegions()
	{
		ImVec2 timelinePosition = ImGui::GetCursorScreenPos();
		ImVec2 timelineSize = ImGui::GetWindowSize() - ImGui::GetCursorPos() - ImGui::GetStyle().WindowPadding;
		timelineRegion = ImRect(timelinePosition, timelinePosition + timelineSize);

		ImVec2 headerPosition = timelineRegion.GetTL();
		ImVec2 headerSize = ImVec2(infoColumnWidth, timelineHeaderHeight + tempoMapHeight);
		infoColumnHeaderRegion = ImRect(headerPosition, headerPosition + headerSize);

		ImVec2 infoPosition = infoColumnHeaderRegion.GetBL();
		ImVec2 infoSize = ImVec2(infoColumnWidth, timelineRegion.GetHeight() - infoColumnHeaderRegion.GetHeight());
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
		timelineContentRegion = ImRect(timelineTargetPosition, timelineTargetPosition + timelineTargetSize);
	}

	void TimelineBase::UpdateInputTimelineScroll()
	{
		// Grab Control
		// ------------
		{
			constexpr int timelineGrabButton = 2;

			if (ImGui::IsWindowHovered() && !ImGui::IsAnyItemActive() && ImGui::IsMouseDragging(timelineGrabButton, 0.0f))
				SetScrollX(GetScrollX() - io->MouseDelta.x);
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
			if (ImGui::IsWindowHovered() && io->MouseWheel != 0.0f)
			{
				if (io->KeyAlt) // Zoom Timeline
				{
					float prePosition = GetCursorTimelinePosition();

					float amount = .5f;
					zoomLevel = zoomLevel + amount * io->MouseWheel;

					if (zoomLevel <= 0)
						zoomLevel = amount;

					float postPosition = GetCursorTimelinePosition();
					SetScrollX(GetScrollX() + postPosition - prePosition);
				}
				else if (!io->KeyCtrl) // Scroll Timeline
				{
					OnTimelineBaseScroll();
				}
			}
		}
	}

	void TimelineBase::UpdateInputPlaybackToggle()
	{
		if (ImGui::IsKeyPressed(GLFW_KEY_SPACE))
		{
			if (GetIsPlayback())
				PausePlayback();
			else
				ResumePlayback();
		}

		if (ImGui::IsKeyPressed(GLFW_KEY_ESCAPE) && GetIsPlayback())
		{
			if (GetIsPlayback())
				StopPlayback();
		}
	}

	void TimelineBase::UpdateCursorAutoScroll()
	{
		// Scroll Cursor
		float cursorPos = (GetCursorTimelinePosition());
		float endPos = (ScreenToTimelinePosition(timelineContentRegion.GetBR().x));

		float autoScrollOffset = (timelineContentRegion.GetWidth() / autoScrollOffsetFraction);
		if (cursorPos >= endPos - autoScrollOffset)
		{
			float increment = cursorPos - endPos + autoScrollOffset;
			SetScrollX(GetScrollX() + increment);

			// allow the cursor to go offscreen
			if (GetMaxScrollX() - GetScrollX() > autoScrollOffset)
				autoScrollCursor = true;
		}
	}

	void TimelineBase::CenterCursor()
	{
		float center = GetCursorTimelinePosition() - (timelineContentRegion.GetWidth() / 2.0f);
		SetScrollX(center);
	}

	bool TimelineBase::IsCursorOnScreen() const
	{
		float cursorPosition = GetCursorTimelinePosition() - GetScrollX();
		return cursorPosition >= 0.0f && cursorPosition <= timelineContentRegion.GetWidth();
	}

	void TimelineBase::UpdateAllInput()
	{
		if (!(updateInput = ImGui::IsWindowFocused()))
			return;

		UpdateInputTimelineScroll();
		UpdateInputPlaybackToggle();
		OnUpdateInput();
	}

	void TimelineBase::UpdateTimelineBase()
	{
		// make sure the cursor time is the same through the entire draw tick
		cursorTime = GetCursorTime();
		autoScrollCursor = false;

		if (GetIsPlayback())
		{
			UpdateCursorAutoScroll();
		}
	}

	void TimelineBase::UpdateTimelineSize()
	{
		ImGui::ItemSize(ImVec2(GetTimelineSize(), 0));
	}

	void TimelineBase::OnTimelineBaseScroll()
	{
		ImVec2 maxStep = (baseWindow->ContentsRegionRect.GetSize() + baseWindow->WindowPadding * 2.0f) * 0.67f;

		float speed = io->KeyShift ? scrollSpeedFast : scrollSpeed;
		float scrollStep = ImFloor(ImMin(2 * baseWindow->CalcFontSize(), maxStep.x)) * speed;
		SetScrollX(baseWindow->Scroll.x + io->MouseWheel * scrollStep);
	}

	void TimelineBase::DrawTimelineGui()
	{
		ImGui::BeginGroup();
		OnDrawTimelineHeaderWidgets();
		ImGui::EndGroup();

		UpdateTimelineRegions();

		ImGui::BeginChild("##timeline_info_column", ImVec2(0, -ImGui::GetStyle().ScrollbarSize));
		OnDrawTimelineInfoColumnHeader();
		OnDrawTimelineInfoColumn();
		ImGui::EndChild();

		ImGui::SetCursorScreenPos(infoColumnHeaderRegion.GetTR());
		ImGui::BeginChild("##timeline_base", ImVec2(), false, ImGuiWindowFlags_HorizontalScrollbar | ImGuiWindowFlags_AlwaysHorizontalScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
		UpdateTimelineBaseState();
		DrawTimelineBase();
		ImGui::EndChild();
	}

	void TimelineBase::DrawTimelineBase()
	{
		if (scrollDelta != 0.0f)
		{
			SetScrollX(GetScrollX() + scrollDelta);
			scrollDelta = 0.0f;
		}

		UpdateTimelineSize();
		OnUpdate();

		// Timeline Header Region BG
		// -------------------------
		baseDrawList->AddRectFilled(timelineHeaderRegion.GetTL(), timelineHeaderRegion.GetBR(), GetColor(EditorColor_InfoColumn));
		
		// Timeline Target Region BG
		// -------------------------
		baseDrawList->AddRectFilled(timelineContentRegion.GetTL(), timelineContentRegion.GetBR(), GetColor(EditorColor_TimelineBg));

		OnDrawTimlineTempoMap();
		OnDrawTimlineRows();
		OnDrawTimlineDivisors();
		OnDrawTimlineBackground();

		if (timelineHeaderRegion.Contains(ImGui::GetMousePos()))
		{
			ImGui::SetTooltip("TIME: %s", GetTimelineTime(ScreenToTimelinePosition(ImGui::GetMousePos().x)).FormatTime().c_str());
		}

		UpdateTimelineBase();
		UpdateAllInput();

		OnDrawTimelineContents();
		DrawTimelineCursor();
	}

	void TimelineBase::OnDrawTimelineInfoColumnHeader()
	{
		auto drawList = ImGui::GetWindowDrawList();

		drawList->AddRectFilled(infoColumnHeaderRegion.GetTL(), infoColumnHeaderRegion.GetBR(), GetColor(EditorColor_InfoColumn), 8.0f, ImDrawCornerFlags_TopLeft);
	}

	void TimelineBase::OnDrawTimelineInfoColumn()
	{
		auto drawList = ImGui::GetWindowDrawList();

		// top part
		drawList->AddRectFilled(infoColumnRegion.GetTL(), infoColumnRegion.GetBR(), GetColor(EditorColor_InfoColumn));

		// bottom part
		ImDrawList* parentDrawList = ImGui::GetCurrentWindow()->ParentWindow->DrawList;
		parentDrawList->AddRectFilled(infoColumnRegion.GetBL(), infoColumnRegion.GetBL() + ImVec2(infoColumnRegion.GetWidth(), -ImGui::GetStyle().ScrollbarSize), GetColor(EditorColor_InfoColumn));
	}

	void TimelineBase::OnDrawTimlineTempoMap()
	{
		baseDrawList->AddRectFilled(tempoMapRegion.GetTL(), tempoMapRegion.GetBR(), GetColor(EditorColor_TempoMapBg));
	}
}