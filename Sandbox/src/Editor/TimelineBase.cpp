#include "TimelineBase.h"

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
	return screenPosition - timelineTargetRegion.Min.x + GetScrollX();
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

void TimelineBase::InitializeTimelineBase()
{
	io = &ImGui::GetIO();
}

void TimelineBase::UpdateTimelineBase()
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
	timelineTargetRegion = ImRect(timelineTargetPosition, timelineTargetPosition + timelineTargetSize);
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

void TimelineBase::UpdateCursorAutoScroll()
{
	// Scroll Cursor
	float cursorPos = (GetCursorTimelinePosition());
	float endPos = (ScreenToTimelinePosition(timelineTargetRegion.GetBR().x));

	float autoScrollOffset = (timelineTargetRegion.GetWidth() / autoScrollOffsetFraction);
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
	float center = GetCursorTimelinePosition() - (timelineTargetRegion.GetWidth() / 2.0f);
	SetScrollX(center);
}

bool TimelineBase::IsCursorOnScreen() const
{
	float cursorPosition = GetCursorTimelinePosition() - GetScrollX();
	return cursorPosition >= 0.0f && cursorPosition <= timelineTargetRegion.GetWidth();
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
