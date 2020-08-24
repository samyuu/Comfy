#include "TimelineBase.h"
#include "Editor/Core/Theme.h"
#include "Input/Input.h"
#include <FontIcons.h>

namespace Comfy::Studio::Editor
{
	float TimelineBase::GetTimelinePosition(TimeSpan time) const
	{
		return static_cast<float>(time.TotalSeconds() * zoomLevel * zoomBaseFactor);
	}

	TimeSpan TimelineBase::GetTimelineTime(float position) const
	{
		return TimeSpan::FromSeconds(position / zoomLevel / zoomBaseFactor);
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

	TimelineVisibility TimelineBase::GetTimelineVisibilityForScreenSpace(float screenX) const
	{
		const float timelineX = screenX - timelineContentRegion.Min.x;

		const float visibleMin = -timelineVisibleThreshold;
		const float visibleMax = baseWindow->Size.x + timelineVisibleThreshold;

		if (timelineX < visibleMin)
			return TimelineVisibility::Left;

		if (timelineX > visibleMax)
			return TimelineVisibility::Right;

		return TimelineVisibility::Visible;
	}

	void TimelineBase::DrawTimelineCursor()
	{
		const ImU32 outterColor = GetColor(EditorColor_Cursor);
		const ImU32 innerColor = GetColor(EditorColor_CursorInner);

		const float scrollX = GetScrollX();
		const float cursorX = GetCursorTimelinePosition();
		float cursorScreenX = cursorX - scrollX;

		// NOTE: Ensure smooth cursor scrolling
		if (autoScrollCursorEnabled)
			cursorScreenX = timelineContentRegion.GetWidth() - (timelineContentRegion.GetWidth() * (1.0f - autoScrollCursorOffsetPercentage));

		cursorScreenX = glm::round(cursorScreenX);

		const vec2 start = timelineHeaderRegion.GetTL() + vec2(cursorScreenX, 0.0f);
		const vec2 end = timelineContentRegion.GetBL() + vec2(cursorScreenX, 0.0f);

		baseDrawList->AddLine(start + vec2(0.0f, cursorHeadHeight - 1.0f), end, outterColor);

		const float centerX = start.x + 0.5f;
		const vec2 cursorTriangle[3] =
		{
			vec2(centerX - cursorHeadWidth * 0.5f, start.y),
			vec2(centerX + cursorHeadWidth * 0.5f, start.y),
			vec2(centerX, start.y + cursorHeadHeight),
		};
		baseDrawList->AddTriangleFilled(cursorTriangle[0], cursorTriangle[1], cursorTriangle[2], innerColor);
		baseDrawList->AddTriangle(cursorTriangle[0], cursorTriangle[1], cursorTriangle[2], outterColor);
	}

	void TimelineBase::SetZoomCenteredAroundCursor(float newZoom)
	{
		const auto cursorTime = GetCursorTime();
		const auto minVisibleTime = GetTimelineTime(ScreenToTimelinePosition(timelineContentRegion.GetTL().x));
		const auto maxVisibleTime = GetTimelineTime(ScreenToTimelinePosition(timelineContentRegion.GetTR().x));

		// NOTE: Because centered zooming around an off-screen target can be very disorientating as all points on the timeline appear to be moving
		const auto visibleClampedCursorTime = std::clamp(cursorTime, minVisibleTime, maxVisibleTime);
		SetZoomCenteredAroundTime(newZoom, visibleClampedCursorTime);
	}

	void TimelineBase::SetZoomCenteredAroundTime(float newZoom, TimeSpan timeToCenter)
	{
		const float prePosition = GetTimelinePosition(timeToCenter);

		if (newZoom > 0.0f)
			zoomLevel = std::clamp(newZoom, hardZoomLevelMin, hardZoomLevelMax);

		const float postPosition = GetTimelinePosition(timeToCenter);
		SetScrollX(GetScrollX() + postPosition - prePosition);
	}

	void TimelineBase::UpdateInfoColumnInput()
	{
		const auto& io = Gui::GetIO();

		if (Gui::IsWindowHovered() && infoColumnRegion.Contains(io.MousePos) && io.MouseWheel != 0.0f)
			OnInfoColumnScroll();

		// NOTE: Always clamp in case the window has been resized
		SetScrollY(std::clamp(scroll.y, 0.0f, maxScroll.y));
	}

	void TimelineBase::UpdateTimelineRegions()
	{
		const auto& style = Gui::GetStyle();

		const auto timelinePosition = Gui::GetCursorScreenPos();
		const auto timelineSize = Gui::GetWindowSize() - Gui::GetCursorPos() - style.WindowPadding;
		timelineRegion = ImRect(timelinePosition, timelinePosition + timelineSize);

		const auto headerPosition = timelineRegion.GetTL();
		const auto headerSize = vec2(infoColumnWidth, timelineHeaderHeight + tempoMapHeight);
		infoColumnHeaderRegion = ImRect(headerPosition, headerPosition + headerSize);

		const auto infoPosition = infoColumnHeaderRegion.GetBL();
		const auto infoSize = vec2(infoColumnWidth, timelineRegion.GetHeight() - infoColumnHeaderRegion.GetHeight() - timelineScrollbarSize.y);
		infoColumnRegion = ImRect(infoPosition, infoPosition + infoSize);

		const auto timelineBasePosition = infoColumnHeaderRegion.GetTR();
		const auto timelineBaseSize = vec2(timelineRegion.GetWidth() - infoColumnRegion.GetWidth() - timelineScrollbarSize.x, timelineRegion.GetHeight() - timelineScrollbarSize.y);
		timelineBaseRegion = ImRect(timelineBasePosition, timelineBasePosition + timelineBaseSize);

		const auto tempoMapPosition = timelineBaseRegion.GetTL();
		const auto tempoMapSize = vec2(timelineBaseRegion.GetWidth(), tempoMapHeight);
		tempoMapRegion = ImRect(tempoMapPosition, tempoMapPosition + tempoMapSize);

		const auto timelineHeaderPosition = tempoMapRegion.GetBL();
		const auto timelineHeaderSize = vec2(timelineBaseRegion.GetWidth(), timelineHeaderHeight);
		timelineHeaderRegion = ImRect(timelineHeaderPosition, timelineHeaderPosition + timelineHeaderSize);

		const auto timelineTargetPosition = timelineHeaderRegion.GetBL();
		const auto timelineTargetSize = vec2(timelineBaseRegion.GetWidth(), timelineBaseRegion.GetHeight() - timelineHeaderSize.y - tempoMapSize.y);
		timelineContentRegion = ImRect(timelineTargetPosition, timelineTargetPosition + timelineTargetSize);
	}

	void TimelineBase::UpdateInputTimelineScroll()
	{
		const auto& io = Gui::GetIO();

		constexpr int timelineGrabButton = 2;
		if (Gui::IsWindowHovered() && !Gui::IsAnyItemActive() && Gui::IsMouseDown(timelineGrabButton))
		{
			SetScrollX(GetScrollX() - io.MouseDelta.x);
			Gui::SetMouseCursor(ImGuiMouseCursor_Hand);
		}

		if (Gui::IsWindowFocused() && Gui::IsKeyPressed(Input::KeyCode_Escape))
			CenterCursor();

		if (Gui::IsWindowHovered() && io.MouseWheel != 0.0f)
		{
			if (io.KeyAlt) // NOTE: Zoom timeline
			{
				const auto factor = (io.MouseWheel > 0.0f) ? 1.1f : 0.9f;
				const auto newZoom = (zoomLevel * factor);

#if 0
				SetZoomCenteredAroundCursor(newZoom);
#else
				const auto timeToCenterAround = GetTimelineTime(ScreenToTimelinePosition(Gui::GetMousePos().x));
				SetZoomCenteredAroundTime(newZoom, timeToCenterAround);
#endif
			}
			else if (!io.KeyCtrl) // NOTE: Scroll timeline
			{
				OnTimelineBaseScroll();
			}
		}
	}

	void TimelineBase::UpdateInputPlaybackToggle()
	{
		if (!Gui::IsWindowFocused())
			return;

		if (Gui::IsKeyPressed(Input::KeyCode_Space))
		{
			if (GetIsPlayback())
				PausePlayback();
			else
				ResumePlayback();
		}

		if (Gui::IsKeyPressed(Input::KeyCode_Escape) && GetIsPlayback())
		{
			if (GetIsPlayback())
				StopPlayback();
		}
	}

	void TimelineBase::UpdateCursorAutoScroll()
	{
		const auto cursorPos = (GetCursorTimelinePosition());
		const auto endPos = (ScreenToTimelinePosition(timelineContentRegion.GetBR().x));

		const auto autoScrollOffset = (timelineContentRegion.GetWidth() * (1.0f - autoScrollCursorOffsetPercentage));
		if (cursorPos >= endPos - autoScrollOffset)
		{
			const auto increment = (cursorPos - endPos + autoScrollOffset);
			SetScrollX(GetScrollX() + increment);

			// NOTE: Allow the cursor to go offscreen
			if (GetMaxScrollX() - GetScrollX() > autoScrollOffset)
				autoScrollCursorEnabled = true;
		}
	}

	void TimelineBase::CenterCursor()
	{
		const float center = GetCursorTimelinePosition() - (timelineContentRegion.GetWidth() / 2.0f);
		SetScrollX(center);
	}

	bool TimelineBase::IsCursorOnScreen() const
	{
		const auto cursorPosition = GetCursorTimelinePosition() - GetScrollX();
		return cursorPosition >= 0.0f && cursorPosition <= timelineContentRegion.GetWidth();
	}

	void TimelineBase::UpdateAllInput()
	{
		updateInput = Gui::IsWindowFocused();

		UpdateInputTimelineScroll();
		UpdateInputPlaybackToggle();
		OnUpdateInput();
	}

	void TimelineBase::UpdateTimelineBase()
	{
		cursorTime = GetCursorTime();
		autoScrollCursorEnabled = false;

		if (GetIsPlayback())
		{
			UpdateCursorAutoScroll();
		}
	}

	void TimelineBase::UpdateTimelineSize()
	{
		// Gui::ItemSize(vec2(GetTimelineSize(), 0.0f));
		maxScroll.x = GetTimelineSize();

		SetMaxScrollY(GetTimelineHeight());
	}

	void TimelineBase::OnInfoColumnScroll()
	{
		const auto& io = Gui::GetIO();

		const float newScrollY = GetScrollY() - (io.MouseWheel * infoColumnScrollStep);
		SetScrollY(newScrollY);
	}

	void TimelineBase::OnTimelineBaseScroll()
	{
		const auto& io = Gui::GetIO();
		const auto maxStep = (maxScroll.x + baseWindow->WindowPadding.x * 2.0f) * 0.67f;

		const float speed = io.KeyShift ? scrollSpeedFast : scrollSpeed;
		const float scrollStep = ImFloor(ImMin(2.0f * baseWindow->CalcFontSize(), maxStep)) * speed;
		SetScrollX(scroll.x + io.MouseWheel * scrollStep);
	}

	void TimelineBase::DrawTimelineGui()
	{
#if 0 // DEBUG:
		Gui::DEBUG_NOSAVE_WINDOW("timeline base regions", [this]()
		{
			auto drawRegionIfHighlighted = [](const char* name, const ImRect& region)
			{
				Gui::Selectable(name);
				if (Gui::IsItemHovered())
					Gui::GetForegroundDrawList()->AddRectFilled(region.Min, region.Max, 0x60196690);
			};

			drawRegionIfHighlighted("timelineRegion", timelineRegion);
			drawRegionIfHighlighted("infoColumnHeaderRegion", infoColumnHeaderRegion);
			drawRegionIfHighlighted("infoColumnRegion", infoColumnRegion);
			drawRegionIfHighlighted("timelineBaseRegion", timelineBaseRegion);
			drawRegionIfHighlighted("tempoMapRegion", tempoMapRegion);
			drawRegionIfHighlighted("timelineHeaderRegion", timelineHeaderRegion);
			drawRegionIfHighlighted("timelineContentRegion", timelineContentRegion);
		}, ImGuiWindowFlags_None);
#endif

		Gui::BeginGroup();
		{
			OnDrawTimelineHeaderWidgets();
		}
		Gui::EndGroup();

		UpdateTimelineRegions();

		Gui::BeginChild("##InfoColumnChild::TimelineBase", vec2(0.0f, -timelineScrollbarSize.y));
		{
			OnDrawTimelineInfoColumnHeader();
			OnDrawTimelineInfoColumn();
			UpdateInfoColumnInput();
		}
		Gui::EndChild();

		Gui::SetCursorScreenPos(infoColumnHeaderRegion.GetTR());
		Gui::BeginChild("##BaseChild::TimelineBase", vec2(-timelineScrollbarSize.x, 0.0f), false, ImGuiWindowFlags_NoScrollbar);
		{
			baseWindow = Gui::GetCurrentWindow();
			baseDrawList = baseWindow->DrawList;
			DrawTimelineBase();
		}
		Gui::EndChild();

		Gui::PushStyleVar(ImGuiStyleVar_ItemSpacing, vec2(0.0f, 0.0f));

		Gui::SameLine();
		Gui::BeginChild("VerticalScrollChild::TimelineBase", vec2(0.0f, 0.0f), false, ImGuiWindowFlags_NoScrollbar);
		{
			Gui::GetWindowDrawList()->AddRectFilled(
				GImGui->CurrentWindow->Pos,
				GImGui->CurrentWindow->Pos + GImGui->CurrentWindow->Size - vec2(0.0f, timelineScrollbarSize.y),
				Gui::GetColorU32(ImGuiCol_ScrollbarBg));

			const vec2 positionOffset = vec2(0.0f, infoColumnHeaderRegion.GetHeight());
			const vec2 sizeOffset = vec2(0.0f, -timelineScrollbarSize.y);

			const ImRect scrollbarRegion = ImRect(timelineContentRegion.GetTR(), timelineContentRegion.GetBR() + vec2(timelineScrollbarSize.x, 0.0f));

			// BUG: Incorrect available scroll amount (?)
			if (auto scroll = GetScrollY(); verticalScrollbar.Gui(scroll, timelineContentRegion.GetHeight(), GetMaxScrollY(), scrollbarRegion))
				SetScrollY(scroll);
		}
		Gui::EndChild();

		Gui::SetCursorScreenPos(infoColumnRegion.GetBL());
		Gui::BeginChild("HorizontalScrollChild::TimelineBase", vec2(-timelineScrollbarSize.x, timelineScrollbarSize.y), false, ImGuiWindowFlags_NoScrollbar);
		{
			Gui::GetWindowDrawList()->AddRectFilled(
				GImGui->CurrentWindow->Pos,
				GImGui->CurrentWindow->Pos + GImGui->CurrentWindow->Size,
				Gui::GetColorU32(ImGuiCol_ScrollbarBg));

			OnDrawTimelineScrollBarRegion();
			DrawTimelineZoomSlider();

			const vec2 positionOffset = vec2(infoColumnWidth + zoomSliderWidth, 0.0f);
			const ImRect scrollbarRegion = ImRect(
				GImGui->CurrentWindow->Pos + positionOffset,
				GImGui->CurrentWindow->Pos + GImGui->CurrentWindow->Size);

			if (auto scroll = GetScrollX(); horizontalScrollbar.Gui(scroll, timelineContentRegion.GetWidth(), GetMaxScrollX(), scrollbarRegion))
				SetScrollX(scroll);
		}
		Gui::EndChild();

		Gui::PopStyleVar();
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

		UpdateTimelineBase();
		UpdateAllInput();

		zoomLevel = std::clamp(zoomLevel, hardZoomLevelMin, hardZoomLevelMax);
		zoomLevelChanged = (lastZoomLevel != zoomLevel);
		lastZoomLevel = zoomLevel;

		// NOTE: To give the content region a bit more contrast
		const ImU32 dimColor = Gui::GetColorU32(ImGuiCol_PopupBg, 0.15f);
		Gui::GetWindowDrawList()->AddRectFilled(timelineContentRegion.GetTL(), timelineContentRegion.GetBR(), dimColor);

		OnDrawTimlineTempoMap();
		OnDrawTimlineRows();
		OnDrawTimlineDivisors();
		OnDrawTimlineBackground();

		OnDrawTimelineContents();
		DrawTimelineCursor();
	}

	void TimelineBase::DrawTimelineZoomSlider()
	{
		constexpr float percentageFactor = 100.0f;
		constexpr float buttonZoomFactor = 1.1f;
		const auto buttonSize = vec2(zoomButtonWidth, timelineScrollbarSize.y);

		Gui::SetCursorScreenPos(Gui::GetWindowPos() + vec2(infoColumnWidth, 0.0f));

		Gui::PushStyleVar(ImGuiStyleVar_FramePadding, vec2(Gui::GetStyle().FramePadding.x, 0.0f));
		Gui::PushStyleVar(ImGuiStyleVar_ItemSpacing, vec2(0.0f, 0.0f));

		Gui::PushItemWidth(zoomSliderWidth - zoomButtonWidth * 2.0f);
		{
			if (Gui::ComfySmallButton(ICON_FA_SEARCH_MINUS, buttonSize))
				SetZoomCenteredAroundCursor(std::clamp(zoomLevel * (1.0f / buttonZoomFactor), zoomSliderMin, zoomSliderMax));

			Gui::SameLine();

			float zoomPercentage = (zoomLevel * percentageFactor);
			if (Gui::SliderFloat("##ZoomSlider", &zoomPercentage, zoomSliderMin * percentageFactor, zoomSliderMax * percentageFactor, "%.2f %%"))
				SetZoomCenteredAroundCursor(zoomPercentage * (1.0f / percentageFactor));

			Gui::SameLine();

			if (Gui::ComfySmallButton(ICON_FA_SEARCH_PLUS, buttonSize))
				SetZoomCenteredAroundCursor(std::clamp(zoomLevel * buttonZoomFactor, zoomSliderMin, zoomSliderMax));
		}
		Gui::PopItemWidth();

		Gui::PopStyleVar(2);
	}

	void TimelineBase::OnDrawTimelineInfoColumnHeader()
	{
		auto drawList = Gui::GetWindowDrawList();

		// NOTE: Offset to hide the bottom border line
		constexpr vec2 yOffset = vec2(0.0f, 1.0f);
		drawList->AddRect(infoColumnHeaderRegion.GetTL(), infoColumnHeaderRegion.GetBR() + yOffset, Gui::GetColorU32(ImGuiCol_Border));
	}

	void TimelineBase::OnDrawTimelineInfoColumn()
	{
		auto drawList = Gui::GetWindowDrawList();

		// NOTE: Offset to hide the bottom border line
		constexpr vec2 yOffset = vec2(0.0f, 1.0f);
		drawList->AddRect(infoColumnRegion.GetTL(), infoColumnRegion.GetBR() + yOffset, Gui::GetColorU32(ImGuiCol_Border));
	}

	void TimelineBase::OnDrawTimlineTempoMap()
	{
		const ImU32 bottomColor = Gui::GetColorU32(ImGuiCol_MenuBarBg, 0.85f);
		const ImU32 topColor = Gui::GetColorU32(ImGuiCol_MenuBarBg);

		baseDrawList->AddRectFilledMultiColor(tempoMapRegion.GetTL(), tempoMapRegion.GetBR(), bottomColor, bottomColor, topColor, topColor);
	}
}
