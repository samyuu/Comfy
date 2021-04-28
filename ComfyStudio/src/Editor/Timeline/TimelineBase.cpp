#include "TimelineBase.h"
#include "Editor/Core/Theme.h"
#include "Core/ComfyStudioSettings.h"
#include "Input/Input.h"
#include <FontIcons.h>

namespace Comfy::Studio::Editor
{
	f32 TimelineBase::GetTimelinePosition(TimeSpan time) const
	{
		return static_cast<f32>(time.TotalSeconds() * zoomLevel * zoomBaseFactor);
	}

	TimeSpan TimelineBase::GetTimelineTime(f32 position) const
	{
		return TimeSpan::FromSeconds(position / zoomLevel / zoomBaseFactor);
	}

	f32 TimelineBase::ScreenToTimelinePosition(f32 screenPosition) const
	{
		return screenPosition - regions.Content.Min.x + GetScrollX();
	}

	f32 TimelineBase::GetCursorTimelinePosition() const
	{
		return GetTimelinePosition(GetCursorTime());
	}

	TimeSpan TimelineBase::GetCursorTime() const
	{
		return cursorTime;
	}

	TimelineVisibility TimelineBase::GetTimelineVisibility(f32 screenX) const
	{
		const f32 visibleMin = -timelineVisibleThreshold;
		const f32 visibleMax = baseWindow->Size.x + timelineVisibleThreshold;

		if (screenX < visibleMin)
			return TimelineVisibility::Left;

		if (screenX > visibleMax)
			return TimelineVisibility::Right;

		return TimelineVisibility::Visible;
	}

	TimelineVisibility TimelineBase::GetTimelineVisibilityForScreenSpace(f32 screenX) const
	{
		const f32 timelineX = screenX - regions.Content.Min.x;

		const f32 visibleMin = -timelineVisibleThreshold;
		const f32 visibleMax = baseWindow->Size.x + timelineVisibleThreshold;

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

		const f32 scrollX = GetScrollX();
		const f32 cursorX = GetCursorTimelinePosition();
		f32 cursorScreenX = cursorX - scrollX;

		// NOTE: Ensure smooth cursor scrolling
		if (autoScrollCursorEnabled)
			cursorScreenX = regions.Content.GetWidth() - (regions.Content.GetWidth() * (1.0f - autoScrollCursorOffsetPercentage));

		cursorScreenX = glm::round(cursorScreenX);

		const vec2 start = regions.ContentHeader.GetTL() + vec2(cursorScreenX, 0.0f);
		const vec2 end = regions.Content.GetBL() + vec2(cursorScreenX, 0.0f);

		baseDrawList->AddLine(start + vec2(0.0f, cursorHeadHeight - 1.0f), end, outterColor);

		const f32 centerX = start.x + 0.5f;
		const vec2 cursorTriangle[3] =
		{
			vec2(centerX - cursorHeadWidth * 0.5f, start.y),
			vec2(centerX + cursorHeadWidth * 0.5f, start.y),
			vec2(centerX, start.y + cursorHeadHeight),
		};
		baseDrawList->AddTriangleFilled(cursorTriangle[0], cursorTriangle[1], cursorTriangle[2], innerColor);
		baseDrawList->AddTriangle(cursorTriangle[0], cursorTriangle[1], cursorTriangle[2], outterColor);
	}

	void TimelineBase::SetZoomCenteredAroundCursor(f32 newZoom)
	{
		const auto cursorTime = GetCursorTime();
		const auto minVisibleTime = GetTimelineTime(ScreenToTimelinePosition(regions.Content.GetTL().x));
		const auto maxVisibleTime = GetTimelineTime(ScreenToTimelinePosition(regions.Content.GetTR().x));

		// NOTE: Because centered zooming around an off-screen target can be very disorientating as all points on the timeline appear to be moving
		const auto visibleClampedCursorTime = std::clamp(cursorTime, minVisibleTime, maxVisibleTime);
		SetZoomCenteredAroundTime(newZoom, visibleClampedCursorTime);
	}

	void TimelineBase::SetZoomCenteredAroundTime(f32 newZoom, TimeSpan timeToCenter)
	{
		const f32 prePosition = GetTimelinePosition(timeToCenter);

		if (newZoom > 0.0f)
			zoomLevel = std::clamp(newZoom, hardZoomLevelMin, hardZoomLevelMax);

		const f32 postPosition = GetTimelinePosition(timeToCenter);
		SetScrollX(GetScrollX() + postPosition - prePosition);
	}

	void TimelineBase::CenterCursor(std::optional<f32> widthFactor)
	{
		const f32 center = GetCursorTimelinePosition() - (regions.Content.GetWidth() * widthFactor.value_or(autoScrollCursorOffsetPercentage));
		SetScrollX(center);
	}

	bool TimelineBase::IsCursorOnScreen() const
	{
		const f32 cursorPosition = GetCursorTimelinePosition() - GetScrollX();
		return cursorPosition >= 0.0f && cursorPosition <= regions.Content.GetWidth();
	}

	void TimelineBase::UpdateInfoColumnInput()
	{
		const auto& io = Gui::GetIO();

		if (Gui::IsWindowHovered() && regions.InfoColumnContent.Contains(io.MousePos) && io.MouseWheel != 0.0f)
			OnInfoColumnScroll();

		// NOTE: Always clamp in case the window has been resized
		SetScrollY(std::clamp(scroll.y, 0.0f, maxScroll.y));
	}

	void TimelineBase::UpdateTimelineRegions()
	{
		const auto& style = Gui::GetStyle();

		const auto timelinePosition = Gui::GetCursorScreenPos();
		const auto timelineSize = Gui::GetWindowSize() - Gui::GetCursorPos() - style.WindowPadding;
		regions.Base = ImRect(timelinePosition, timelinePosition + timelineSize);

		const auto headerPosition = regions.Base.GetTL();
		const auto headerSize = vec2(infoColumnWidth, timelineHeaderHeight + tempoMapHeight);
		regions.InfoColumnHeader = ImRect(headerPosition, headerPosition + headerSize);

		const auto infoPosition = regions.InfoColumnHeader.GetBL();
		const auto infoSize = vec2(infoColumnWidth, regions.Base.GetHeight() - regions.InfoColumnHeader.GetHeight() - timelineScrollbarSize.y);
		regions.InfoColumnContent = ImRect(infoPosition, infoPosition + infoSize);

		const auto timelineBasePosition = regions.InfoColumnHeader.GetTR();
		const auto timelineBaseSize = vec2(regions.Base.GetWidth() - regions.InfoColumnContent.GetWidth() - timelineScrollbarSize.x, regions.Base.GetHeight() - timelineScrollbarSize.y);
		regions.ContentBase = ImRect(timelineBasePosition, timelineBasePosition + timelineBaseSize);

		const auto tempoMapPosition = regions.ContentBase.GetTL();
		const auto tempoMapSize = vec2(regions.ContentBase.GetWidth(), tempoMapHeight);
		regions.TempoMap = ImRect(tempoMapPosition, tempoMapPosition + tempoMapSize);

		const auto timelineHeaderPosition = regions.TempoMap.GetBL();
		const auto timelineHeaderSize = vec2(regions.ContentBase.GetWidth(), timelineHeaderHeight);
		regions.ContentHeader = ImRect(timelineHeaderPosition, timelineHeaderPosition + timelineHeaderSize);

		const auto timelineTargetPosition = regions.ContentHeader.GetBL();
		const auto timelineTargetSize = vec2(regions.ContentBase.GetWidth(), regions.ContentBase.GetHeight() - timelineHeaderSize.y - tempoMapSize.y);
		regions.Content = ImRect(timelineTargetPosition, timelineTargetPosition + timelineTargetSize);
	}

	void TimelineBase::UpdateInputTimelineScroll()
	{
		const auto& io = Gui::GetIO();
		constexpr i32 scrollGrabMouseButton = 2;

		if (Gui::IsMouseReleased(scrollGrabMouseButton) || !Gui::IsWindowFocused())
			isMouseScrollGrabbing = false;

		if (isMouseScrollGrabbing)
		{
			Gui::SetMouseCursor(ImGuiMouseCursor_Hand);
			Gui::SetActiveID(Gui::GetID(&isMouseScrollGrabbing), Gui::GetCurrentWindow());
			SetScrollX(GetScrollX() - io.MouseDelta.x);
		}

		if (Gui::IsWindowHovered(ImGuiHoveredFlags_AllowWhenBlockedByActiveItem) && Gui::IsMouseClicked(scrollGrabMouseButton))
		{
			isMouseScrollGrabbing = true;
			Gui::SetWindowFocus();
		}

		if (Gui::IsWindowFocused() && Input::IsAnyPressed(GlobalUserData.Input.Timeline_CenterCursor, false))
			CenterCursor();

		if (Gui::IsWindowHovered(ImGuiHoveredFlags_AllowWhenBlockedByActiveItem) && io.MouseWheel != 0.0f)
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

		if (Input::IsAnyPressed(GlobalUserData.Input.Timeline_TogglePlayback, false, Input::ModifierBehavior_Relaxed))
		{
			if (GetIsPlayback())
				PausePlayback();
			else
				ResumePlayback();
		}

		if (Input::IsAnyPressed(GlobalUserData.Input.Timeline_StopPlayback, false) && GetIsPlayback())
		{
			if (GetIsPlayback())
				StopPlayback();
		}
	}

	void TimelineBase::UpdateCursorAutoScroll()
	{
		const auto cursorPos = (GetCursorTimelinePosition());
		const auto endPos = (ScreenToTimelinePosition(regions.Content.GetBR().x));

		const auto autoScrollOffset = (regions.Content.GetWidth() * (1.0f - autoScrollCursorOffsetPercentage));
		if (cursorPos >= endPos - autoScrollOffset)
		{
			const auto increment = (cursorPos - endPos + autoScrollOffset);
			SetScrollX(GetScrollX() + increment);

			// NOTE: Allow the cursor to go offscreen
			if (GetMaxScrollX() - GetScrollX() > autoScrollOffset)
				autoScrollCursorEnabled = true;
		}
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

		const f32 newScrollY = GetScrollY() - (io.MouseWheel * infoColumnScrollStep);
		SetScrollY(newScrollY);
	}

	void TimelineBase::OnTimelineBaseScroll()
	{
		const auto& io = Gui::GetIO();
		const auto maxStep = (maxScroll.x + baseWindow->WindowPadding.x * 2.0f) * 0.67f;

		const f32 speed = io.KeyShift ? scrollSpeedFast : scrollSpeed;
		const f32 scrollStep = ImFloor(ImMin(2.0f * baseWindow->CalcFontSize(), maxStep)) * speed;
		SetScrollX(scroll.x + io.MouseWheel * scrollStep);
	}

	void TimelineBase::DrawTimelineGui()
	{
#if 0 // DEBUG:
		if (Gui::Begin(__FUNCTION__" DEBUG REGIONS", nullptr, ImGuiWindowFlags_NoSavedSettings))
		{
			auto drawRegionIfHighlighted = [](const char* name, const ImRect& region, ImDrawList* drawList = Gui::GetForegroundDrawList())
			{
				Gui::Selectable(name);
				if (Gui::IsItemHovered())
					drawList->AddRectFilled(region.Min, region.Max, 0x60196690);
			};

			drawRegionIfHighlighted("regions.Base", regions.Base);
			drawRegionIfHighlighted("regions.InfoColumnHeader", regions.InfoColumnHeader);
			drawRegionIfHighlighted("regions.InfoColumnContent", regions.InfoColumnContent);
			drawRegionIfHighlighted("regions.ContentBase", regions.ContentBase);
			drawRegionIfHighlighted("regions.TempoMap", regions.TempoMap);
			drawRegionIfHighlighted("regions.ContentHeader", regions.ContentHeader);
			drawRegionIfHighlighted("regions.Content", regions.Content);
		}
		Gui::End();
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

		Gui::SetCursorScreenPos(regions.InfoColumnHeader.GetTR());
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

			const vec2 positionOffset = vec2(0.0f, regions.InfoColumnHeader.GetHeight());
			const vec2 sizeOffset = vec2(0.0f, -timelineScrollbarSize.y);

			const ImRect scrollbarRegion = ImRect(regions.Content.GetTR(), regions.Content.GetBR() + vec2(timelineScrollbarSize.x, 0.0f));

			// BUG: Incorrect available scroll amount (?)
			if (auto scroll = GetScrollY(); verticalScrollbar.Gui(scroll, regions.Content.GetHeight(), GetMaxScrollY(), scrollbarRegion))
				SetScrollY(scroll);
		}
		Gui::EndChild();

		Gui::SetCursorScreenPos(regions.InfoColumnContent.GetBL());
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

			if (auto scroll = GetScrollX(); horizontalScrollbar.Gui(scroll, regions.Content.GetWidth(), GetMaxScrollX() + 1.0f, scrollbarRegion))
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
		Gui::GetWindowDrawList()->AddRectFilled(regions.Content.GetTL(), regions.Content.GetBR(), dimColor);

		OnDrawTimlineTempoMap();
		OnDrawTimlineRows();
		OnDrawTimlineDivisors();
		OnDrawTimlineBackground();

		OnDrawTimelineContents();
		DrawTimelineCursor();
	}

	void TimelineBase::DrawTimelineZoomSlider()
	{
		constexpr f32 percentageFactor = 100.0f;
		constexpr f32 buttonZoomFactor = 1.1f;
		const auto buttonSize = vec2(zoomButtonWidth, timelineScrollbarSize.y);

		Gui::SetCursorScreenPos(Gui::GetWindowPos() + vec2(infoColumnWidth, 0.0f));

		Gui::PushStyleVar(ImGuiStyleVar_FramePadding, vec2(Gui::GetStyle().FramePadding.x, 0.0f));
		Gui::PushStyleVar(ImGuiStyleVar_ItemSpacing, vec2(0.0f, 0.0f));

		Gui::PushItemWidth(zoomSliderWidth - zoomButtonWidth * 2.0f);
		{
			if (Gui::ComfySmallButton(ICON_FA_SEARCH_MINUS, buttonSize))
				SetZoomCenteredAroundCursor(std::clamp(zoomLevel * (1.0f / buttonZoomFactor), zoomSliderMin, zoomSliderMax));

			Gui::SameLine();

			f32 zoomPercentage = (zoomLevel * percentageFactor);
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
		drawList->AddRect(regions.InfoColumnHeader.GetTL(), regions.InfoColumnHeader.GetBR() + yOffset, Gui::GetColorU32(ImGuiCol_Border));
	}

	void TimelineBase::OnDrawTimelineInfoColumn()
	{
		auto drawList = Gui::GetWindowDrawList();

		// NOTE: Offset to hide the bottom border line
		constexpr vec2 yOffset = vec2(0.0f, 1.0f);
		drawList->AddRect(regions.InfoColumnContent.GetTL(), regions.InfoColumnContent.GetBR() + yOffset, Gui::GetColorU32(ImGuiCol_Border));
	}

	void TimelineBase::OnDrawTimlineTempoMap()
	{
		const ImU32 bottomColor = Gui::GetColorU32(ImGuiCol_MenuBarBg, 0.85f);
		const ImU32 topColor = Gui::GetColorU32(ImGuiCol_MenuBarBg);

		baseDrawList->AddRectFilledMultiColor(regions.TempoMap.GetTL(), regions.TempoMap.GetBR(), bottomColor, bottomColor, topColor, topColor);
	}
}
