#include "TimelineBase.h"
#include "Editor/Core/Theme.h"
#include "Core/ComfyStudioSettings.h"
#include "Input/Input.h"
#include <FontIcons.h>

namespace Comfy::Studio::Editor
{
	namespace
	{
		// TODO: Move to a different public header
		constexpr f32 SmoothDamp(const f32 current, const f32 target, f32& inOutVelocity, const f32 smoothTime, const f32 deltaTime)
		{
			const f32 omega = 2.0f / Max(0.0001f, smoothTime);
			const f32 x = omega * deltaTime;
			const f32 exp = 1.0f / (1.0f + x + 0.48f * x * x + 0.235f * x * x * x);
			const f32 change = current - target;

			const f32 newTarget = (current - change);
			const f32 oldTarget = target;

			const f32 temp = (inOutVelocity + omega * change) * deltaTime;
			const f32 result = newTarget + (change + temp) * exp;

			if (((oldTarget - current) > 0.0f) == (result > oldTarget))
			{
				inOutVelocity = 0.0f;
				return oldTarget;
			}
			else
			{
				inOutVelocity = (inOutVelocity - omega * temp) * exp;
				return result;
			}
		}
	}

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
		const f32 visibleMin = -visibilityThreshold;
		const f32 visibleMax = baseWindow->Size.x + visibilityThreshold;

		if (screenX < visibleMin)
			return TimelineVisibility::Left;

		if (screenX > visibleMax)
			return TimelineVisibility::Right;

		return TimelineVisibility::Visible;
	}

	TimelineVisibility TimelineBase::GetTimelineVisibilityForScreenSpace(f32 screenX) const
	{
		const f32 timelineX = screenX - regions.Content.Min.x;

		const f32 visibleMin = -visibilityThreshold;
		const f32 visibleMax = baseWindow->Size.x + visibilityThreshold;

		if (timelineX < visibleMin)
			return TimelineVisibility::Left;

		if (timelineX > visibleMax)
			return TimelineVisibility::Right;

		return TimelineVisibility::Visible;
	}

	void TimelineBase::DrawTimelineCursor()
	{
		ImU32 outterColor = GetColor(EditorColor_Cursor);
		ImU32 innerColor = GetColor(EditorColor_CursorInner);

		const f32 scrollX = GetScrollX();
		const f32 cursorX = GetCursorTimelinePosition();
		const f32 realCursorScreenX = (cursorX - scrollX);
		f32 cursorScreenX = glm::round(realCursorScreenX);

		f32 autoScrollCursorScreenX = regions.Content.GetWidth() - (regions.Content.GetWidth() * (1.0f - playbackAutoScrollCursorPositionFactor));
		if (enablePlaybackAutoScrollLocking && lockCursorToAutoScrollPosition)
		{
			// NOTE: Only allow scroll locking in the forward direction because it's a right side auto scroll
			autoScrollCursorScreenX = Max(autoScrollCursorScreenX, cursorScreenX);

			const bool hasTransition = (realAndAutoScrollLockedCursorPositionTransitionTime > TimeSpan::Zero());
			const f32 percentageOrTransitionElapsedSoFar = static_cast<f32>(ConvertRangeClamped<f64>(
				0.0, realAndAutoScrollLockedCursorPositionTransitionTime.TotalSeconds(),
				0.0, 1.0,
				lastAutoScrollLockStopwatch.GetElapsed().TotalSeconds()));

			cursorScreenX = (percentageOrTransitionElapsedSoFar < 1.0f && hasTransition) ?
				ConvertRange(0.0f, 1.0f, realCursorScreenX, autoScrollCursorScreenX, percentageOrTransitionElapsedSoFar) :
				glm::round(autoScrollCursorScreenX);

			if (debugVisualizeLockedAutoScrollCursorStateByUsingDifferentColors)
			{
				outterColor = GetColor(EditorColor_GreenText);
				innerColor = GetColor(percentageOrTransitionElapsedSoFar < 1.0f ? EditorColor_GreenText : EditorColor_RedText);
			}
		}

		const vec2 start = regions.ContentHeader.GetTL() + vec2(cursorScreenX, 0.0f);
		const vec2 end = regions.Content.GetBL() + vec2(cursorScreenX, 0.0f);

		baseWindowDrawList->AddLine(start + vec2(0.0f, cursorHeadHeight - 1.0f), end, outterColor);

		const f32 centerX = start.x + 0.5f;
		const vec2 cursorTriangle[3] =
		{
			vec2(centerX - cursorHeadWidth * 0.5f, start.y),
			vec2(centerX + cursorHeadWidth * 0.5f, start.y),
			vec2(centerX, start.y + cursorHeadHeight),
		};
		baseWindowDrawList->AddTriangleFilled(cursorTriangle[0], cursorTriangle[1], cursorTriangle[2], innerColor);
		baseWindowDrawList->AddTriangle(cursorTriangle[0], cursorTriangle[1], cursorTriangle[2], outterColor);

		if (debugVisualizeAutoScrollCursorPositionByDrawingAdditionalCursorLine)
		{
			const u32 debugColor = ImColor(0.74f, 0.40f, 0.13f, 0.75f);
			baseWindowDrawList->AddLine(regions.Content.GetTL() + vec2(autoScrollCursorScreenX, 0.0f), regions.Content.GetBL() + vec2(autoScrollCursorScreenX, 0.0f), debugColor);
		}
	}

	void TimelineBase::InvalidateAutoScrollLock()
	{
		lockCursorToAutoScrollPosition = false;
	}

	bool TimelineBase::IsCursorAutoScrollLocked() const
	{
		return lockCursorToAutoScrollPosition;
	}

	void TimelineBase::SetZoomCenteredAroundCursor(f32 newZoom)
	{
		const auto cursorTime = GetCursorTime();
		const auto minVisibleTime = GetTimelineTime(ScreenToTimelinePosition(regions.Content.GetTL().x));
		const auto maxVisibleTime = GetTimelineTime(ScreenToTimelinePosition(regions.Content.GetTR().x));

		// NOTE: Because centered zooming around an off-screen target can be very disorientating as all points on the timeline appear to be moving
		const auto visibleClampedCursorTime = Clamp(cursorTime, minVisibleTime, maxVisibleTime);
		SetZoomCenteredAroundTime(newZoom, visibleClampedCursorTime);
	}

	void TimelineBase::SetZoomCenteredAroundTime(f32 newZoom, TimeSpan timeToCenter)
	{
		const f32 prePosition = GetTimelinePosition(timeToCenter);

		if (newZoom > 0.0f)
			zoomLevel = Clamp(newZoom, hardZoomLevelMin, hardZoomLevelMax);

		const f32 postPosition = GetTimelinePosition(timeToCenter);
		scroll.x = scrollTarget.x = (GetScrollTargetX() + postPosition - prePosition);
		InvalidateAutoScrollLock();
	}

	void TimelineBase::CenterCursor(std::optional<f32> widthFactor)
	{
		const f32 centerScrollX = GetCursorTimelinePosition() - (regions.Content.GetWidth() * widthFactor.value_or(playbackAutoScrollCursorPositionFactor));
		scrollTarget.x = centerScrollX;
	}

	bool TimelineBase::IsCursorOnScreen() const
	{
		const f32 cursorPosition = GetCursorTimelinePosition() - GetScrollX();
		return cursorPosition >= 0.0f && cursorPosition <= regions.Content.GetWidth();
	}

	void TimelineBase::UpdateInfoColumnInput()
	{
		const auto& io = Gui::GetIO();

		if (Gui::IsWindowHovered() && regions.InfoColumnContent.Contains(io.MousePos) && io.MouseWheel != 0.0f && !isMouseScrollGrabbing)
			OnInfoColumnScroll();

		// NOTE: Always clamp in case the window has been resized
		scrollTarget.y = Clamp(scrollTarget.y, 0.0f, scrollMax.y);
	}

	void TimelineBase::UpdateTimelineRegions()
	{
		const auto& style = Gui::GetStyle();

		const vec2 basePosition = Gui::GetCursorScreenPos();
		const vec2 baseSize = Gui::GetWindowSize() - Gui::GetCursorPos() - style.WindowPadding;
		regions.Base = ImRect(basePosition, basePosition + baseSize);

		const vec2 headerPosition = regions.Base.GetTL();
		const vec2 headerSize = vec2(infoColumnWidth, timelineHeaderHeight + tempoMapHeight);
		regions.InfoColumnHeader = ImRect(headerPosition, headerPosition + headerSize);

		const vec2 infoColumnPosition = regions.InfoColumnHeader.GetBL();
		const vec2 infoColumnSize = vec2(infoColumnWidth, regions.Base.GetHeight() - regions.InfoColumnHeader.GetHeight() - scrollbarSize.y);
		regions.InfoColumnContent = ImRect(infoColumnPosition, infoColumnPosition + infoColumnSize);

		const vec2 contentBasePosition = regions.InfoColumnHeader.GetTR();
		const vec2 contentBaseSize = vec2(regions.Base.GetWidth() - regions.InfoColumnContent.GetWidth() - scrollbarSize.x, regions.Base.GetHeight() - scrollbarSize.y);
		regions.ContentBase = ImRect(contentBasePosition, contentBasePosition + contentBaseSize);

		const vec2 tempoMapPosition = regions.ContentBase.GetTL();
		const vec2 tempoMapSize = vec2(regions.ContentBase.GetWidth(), tempoMapHeight);
		regions.TempoMap = ImRect(tempoMapPosition, tempoMapPosition + tempoMapSize);

		const vec2 contentHeaderPosition = regions.TempoMap.GetBL();
		const vec2 contentHeaderSize = vec2(regions.ContentBase.GetWidth(), timelineHeaderHeight);
		regions.ContentHeader = ImRect(contentHeaderPosition, contentHeaderPosition + contentHeaderSize);

		const vec2 contentPosition = regions.ContentHeader.GetBL();
		const vec2 contentSize = vec2(regions.ContentBase.GetWidth(), regions.ContentBase.GetHeight() - contentHeaderSize.y - tempoMapSize.y);
		regions.Content = ImRect(contentPosition, contentPosition + contentSize);
	}

	void TimelineBase::UpdateInputTimelineScroll()
	{
		const auto& io = Gui::GetIO();
		constexpr ImGuiMouseButton scrollGrabMouseButton = ImGuiMouseButton_Middle;

		if (Gui::IsMouseReleased(scrollGrabMouseButton) || !IsGuiWindowOrChildrenFocused())
			isMouseScrollGrabbing = false;

		if (isMouseScrollGrabbing)
		{
			Gui::SetMouseCursor(ImGuiMouseCursor_Hand);
			Gui::SetActiveID(Gui::GetID(&isMouseScrollGrabbing), Gui::GetCurrentWindow());

			// NOTE: To prevent ugly snapping when it's not needed
			if (io.MouseDelta.x != 0.0f)
			{
				scroll.x = scrollTarget.x = (scrollTarget.x - io.MouseDelta.x);
				InvalidateAutoScrollLock();
			}
		}

		if (Gui::IsWindowHovered(ImGuiHoveredFlags_AllowWhenBlockedByActiveItem) && Gui::IsMouseClicked(scrollGrabMouseButton))
		{
			isMouseScrollGrabbing = true;
			Gui::SetWindowFocus();
		}

		if (IsGuiWindowOrChildrenFocused() && Input::IsAnyPressed(GlobalUserData.Input.Timeline_CenterCursor, false))
			CenterCursor();

		if (Gui::IsWindowHovered(ImGuiHoveredFlags_AllowWhenBlockedByActiveItem) && io.MouseWheel != 0.0f && !isMouseScrollGrabbing)
		{
			if (io.KeyAlt) // NOTE: Zoom timeline
			{
				OnTimelineBaseMouseWheelZoom();
			}
			else if (!io.KeyCtrl) // NOTE: Scroll timeline
			{
				OnTimelineBaseScroll();
			}
		}
	}

	void TimelineBase::UpdateInputPlaybackToggle()
	{
		if (!IsGuiWindowOrChildrenFocused())
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

	f32 TimelineBase::UpdateCursorAutoScrollX()
	{
#if 0
		auto screenToTimelinePosition = [this](f32 screenPosition) { return ScreenToTimelinePosition(screenPosition); };
#else			
		auto screenToTimelinePosition = [this](f32 screenPosition) { return (screenPosition - regions.Content.Min.x + scrollTarget.x); };
#endif

		const f32 cursorX = GetCursorTimelinePosition();
		const f32 endX = (screenToTimelinePosition(regions.Content.GetBR().x));

		const f32 smoothScrollSpeedOffset = ((smoothScrollSpeedSec.x > 0.0f) ? GetTimelinePosition(TimeSpan::FromSeconds(smoothScrollSpeedSec.x)) : 0.0f);
		const f32 autoScrollTargetX = (regions.Content.GetWidth() * (1.0f - playbackAutoScrollCursorPositionFactor)) + smoothScrollSpeedOffset;

		if (cursorX >= (endX - autoScrollTargetX))
			scrollTarget.x += (cursorX - endX + autoScrollTargetX);

		const f32 cursorScreenX = cursorX - scroll.x;
		const f32 autoScrollCursorScreenX = regions.Content.GetWidth() - (regions.Content.GetWidth() * (1.0f - playbackAutoScrollCursorPositionFactor));
		const f32 outCursorAutoScrollDifference = (cursorScreenX - autoScrollCursorScreenX) / zoomLevel;
		return outCursorAutoScrollDifference;
	}

	void TimelineBase::UpdateAllInput()
	{
		UpdateInputTimelineScroll();
		UpdateInputPlaybackToggle();
		OnUpdateInput();
	}

	void TimelineBase::OnInfoColumnScroll()
	{
		const auto& io = Gui::GetIO();
		scrollTarget.y = (scrollTarget.y - (io.MouseWheel * infoColumnScrollStep));
	}

	void TimelineBase::OnTimelineBaseScroll()
	{
		const auto& io = Gui::GetIO();

		const f32 maxStep = (scrollMax.x + baseWindow->WindowPadding.x * 2.0f) * 0.67f;
		const f32 speed = io.KeyShift ? mouseScrollSpeedShift : mouseScrollSpeed;
		const f32 scrollStep = glm::floor(Min(2.0f * baseWindow->CalcFontSize(), maxStep)) * speed;

		scrollTarget.x = (scrollTarget.x + io.MouseWheel * scrollStep);
		InvalidateAutoScrollLock();
	}

	void TimelineBase::OnTimelineBaseMouseWheelZoom()
	{
		const f32 newZoomFactor = (Gui::GetIO().MouseWheel > 0.0f) ? 1.1f : 0.9f;
		const f32 newZoom = (zoomLevel * newZoomFactor);

		const TimeSpan timeToCenterAround = GetTimelineTime(ScreenToTimelinePosition(Gui::GetMousePos().x));
		SetZoomCenteredAroundTime(newZoom, timeToCenterAround);
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

		isAnyGuiChildWindowFocused = false;

		Gui::BeginChild("##InfoColumnChild::TimelineBase", vec2(0.0f, -scrollbarSize.y));
		{
			infoColumnWindow = Gui::GetCurrentWindow();
			isAnyGuiChildWindowFocused |= Gui::IsWindowFocused();

			OnDrawTimelineInfoColumnHeader();
			OnDrawTimelineInfoColumn();
			UpdateInfoColumnInput();
		}
		Gui::EndChild();

		Gui::SetCursorScreenPos(regions.InfoColumnHeader.GetTR());
		Gui::BeginChild("##BaseChild::TimelineBase", vec2(-scrollbarSize.x, 0.0f), false, ImGuiWindowFlags_NoScrollbar);
		{
			baseWindow = Gui::GetCurrentWindow();
			baseWindowDrawList = baseWindow->DrawList;
			isAnyGuiChildWindowFocused |= Gui::IsWindowFocused();

			DrawTimelineBase();
		}
		Gui::EndChild();

		Gui::PushStyleVar(ImGuiStyleVar_ItemSpacing, vec2(0.0f, 0.0f));

		Gui::SameLine();
		Gui::BeginChild("VerticalScrollChild::TimelineBase", vec2(0.0f, 0.0f), false, ImGuiWindowFlags_NoScrollbar);
		{
			verticalScrollBarWindow = Gui::GetCurrentWindow();
			isAnyGuiChildWindowFocused |= Gui::IsWindowFocused();

			Gui::GetWindowDrawList()->AddRectFilled(
				GImGui->CurrentWindow->Pos,
				GImGui->CurrentWindow->Pos + GImGui->CurrentWindow->Size - vec2(0.0f, scrollbarSize.y),
				Gui::GetColorU32(ImGuiCol_ScrollbarBg));

			const vec2 positionOffset = vec2(0.0f, regions.InfoColumnHeader.GetHeight());
			const vec2 sizeOffset = vec2(0.0f, -scrollbarSize.y);

			const ImRect scrollbarRegion = ImRect(regions.Content.GetTR(), regions.Content.GetBR() + vec2(scrollbarSize.x, 0.0f));

			// BUG: Incorrect available scroll amount (?)
			if (f32 tempScrollY = scrollTarget.y; verticalScrollbar.Gui(tempScrollY, regions.Content.GetHeight(), GetMaxScrollY(), scrollbarRegion))
				scrollTarget.y = tempScrollY;
		}
		Gui::EndChild();

		Gui::SetCursorScreenPos(regions.InfoColumnContent.GetBL());
		Gui::BeginChild("HorizontalScrollChild::TimelineBase", vec2(-scrollbarSize.x, scrollbarSize.y), false, ImGuiWindowFlags_NoScrollbar);
		{
			horizontalScrollBarWindow = Gui::GetCurrentWindow();
			isAnyGuiChildWindowFocused |= Gui::IsWindowFocused();

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

			if (f32 tempScrollX = scrollTarget.x; horizontalScrollbar.Gui(tempScrollX, regions.Content.GetWidth(), GetMaxScrollX() + 1.0f, scrollbarRegion))
			{
				scrollTarget.x = tempScrollX;
				InvalidateAutoScrollLock();
			}
		}
		Gui::EndChild();

		Gui::PopStyleVar();
	}

	bool TimelineBase::IsGuiWindowOrChildrenFocused() const
	{
		// BUG: scrollbar children
		return Gui::IsWindowFocused() || isAnyGuiChildWindowFocused;
	}

	void TimelineBase::DrawTimelineBase()
	{
		scrollMax.x = GetTimelineSize();
		scrollMax.y = GetTimelineHeight();

		OnUpdate();

		cursorTime = GetCursorTime();

		UpdateAllInput();

#if COMFY_DEBUG && 0
		debugVisualizeLockedAutoScrollCursorStateByUsingDifferentColors = true;
		debugVisualizeAutoScrollCursorPositionByDrawingAdditionalCursorLine = true;
#endif

		const f32 thisFramePlaybackSpeed = GetDerivedClassPlaybackSpeedOverride();
		if (regions.Content != lastFrameContentRectForAutoScrollInvalidation || !ApproxmiatelySame(thisFramePlaybackSpeed, lastFramePlaybackSpeedForAutoScrollInvalidation))
			InvalidateAutoScrollLock();
		lastFrameContentRectForAutoScrollInvalidation = regions.Content;
		lastFramePlaybackSpeedForAutoScrollInvalidation = thisFramePlaybackSpeed;

		if (GetIsPlayback())
		{
			const f32 autoScrollDifference = UpdateCursorAutoScrollX();

			if (enablePlaybackAutoScrollLocking && !lockCursorToAutoScrollPosition && (thisFramePlaybackSpeed > 0.0f))
			{
				// BUG: This currently doesn't correctly account for smooth scroll speed which means at slow playback + smooth-scroll speeds it won't auto scroll lock
				//		 because slower smooth scrolling results in the screen space cursor X being further behind the auto scroll target.
				//		 ~~Though this might actually be the desired behavior..?~~
				// TODO: Try to fix the root of the problem inside UpdateCursorAutoScrollX() directly... (?)
				//		 Maybe shift auto scroll x position to the left depending on smooth scroll speed and expose this adjusted position via funcion (?)
				const f32 snapThresholdAtReferenceFPS = 3.0f;

				const f32 playbackSpeedSnapThresholdFactor = ApproxmiatelySame(thisFramePlaybackSpeed, 0.0f) ? 1.0f : (1.0f / Clamp(thisFramePlaybackSpeed, 0.5f, 2.0f));
				const f32 frameRateAdjustedSnapThreshold = snapThresholdAtReferenceFPS * (Gui::GetIO().DeltaTime / (1.0f / 60.0f)) * playbackSpeedSnapThresholdFactor;

				const f32 snapThreshold = Clamp(frameRateAdjustedSnapThreshold, 2.0f, 10.0f);
				if (autoScrollDifference < +snapThreshold && autoScrollDifference >= -snapThreshold)
				{
					lockCursorToAutoScrollPosition = true;
					lastAutoScrollLockStopwatch.Restart();
				}
			}
		}

		// NOTE: Make sure to always update this *after* user input (?)
		const vec2 scrollSpeed = GetSmoothScrollSpeedSecOverride().value_or(smoothScrollSpeedSec);
		scroll.x = (scrollSpeed.x <= 0.0f) ? scrollTarget.x : SmoothDamp(scroll.x, scrollTarget.x, smoothScrollVelocity.x, scrollSpeed.x, Gui::GetIO().DeltaTime);
		scroll.y = (scrollSpeed.y <= 0.0f) ? scrollTarget.y : SmoothDamp(scroll.y, scrollTarget.y, smoothScrollVelocity.y, scrollSpeed.y, Gui::GetIO().DeltaTime);

		constexpr f32 snapThreshold = 0.01f;
		if (scroll.x != scrollTarget.x && ApproxmiatelySame(scroll.x, scrollTarget.x, snapThreshold)) scroll.x = scrollTarget.x;
		if (scroll.y != scrollTarget.y && ApproxmiatelySame(scroll.y, scrollTarget.y, snapThreshold)) scroll.y = scrollTarget.y;

		zoomLevel = Clamp(zoomLevel, hardZoomLevelMin, hardZoomLevelMax);
		zoomLevelChangedThisFrame = (lastFrameZoomLevel != zoomLevel);
		lastFrameZoomLevel = zoomLevel;

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
		constexpr f32 buttonZoomFactor = 1.1f;
		const vec2 buttonSize = vec2(zoomButtonWidth, scrollbarSize.y);

		Gui::SetCursorScreenPos(Gui::GetWindowPos() + vec2(infoColumnWidth, 0.0f));

		Gui::PushStyleVar(ImGuiStyleVar_FramePadding, vec2(Gui::GetStyle().FramePadding.x, 0.0f));
		Gui::PushStyleVar(ImGuiStyleVar_ItemSpacing, vec2(0.0f, 0.0f));

		Gui::PushItemWidth(zoomSliderWidth - zoomButtonWidth * 2.0f);
		{
			if (Gui::ComfySmallButton(ICON_FA_SEARCH_MINUS, buttonSize))
				SetZoomCenteredAroundCursor(Clamp(zoomLevel * (1.0f / buttonZoomFactor), zoomSliderMin, zoomSliderMax));

			Gui::SameLine();

			f32 zoomPercentage = ToPercent(zoomLevel);
			if (Gui::SliderFloat("##ZoomSlider", &zoomPercentage, ToPercent(zoomSliderMin), ToPercent(zoomSliderMax), "%.2f %%"))
				SetZoomCenteredAroundCursor(FromPercent(zoomPercentage));

			Gui::SameLine();

			if (Gui::ComfySmallButton(ICON_FA_SEARCH_PLUS, buttonSize))
				SetZoomCenteredAroundCursor(Clamp(zoomLevel * buttonZoomFactor, zoomSliderMin, zoomSliderMax));
		}
		Gui::PopItemWidth();

		Gui::PopStyleVar(2);
	}

	void TimelineBase::OnDrawTimelineInfoColumnHeader()
	{
		auto* drawList = Gui::GetWindowDrawList();

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

		baseWindowDrawList->AddRectFilledMultiColor(regions.TempoMap.GetTL(), regions.TempoMap.GetBR(), bottomColor, bottomColor, topColor, topColor);
	}
}
