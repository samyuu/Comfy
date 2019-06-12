#pragma once
#include "../TimeSpan.h"
#include "../pch.h"

enum class TimelineVisibility
{
	Visible,
	Left,
	Right
};

class TimelineBase
{
public:
	virtual float GetTimelinePosition(TimeSpan time) const;
	virtual TimeSpan GetTimelineTime(float position) const;

	float ScreenToTimelinePosition(float screenPosition) const;
	float GetCursorTimelinePosition() const;

	virtual TimeSpan GetCursorTime() const;

	TimelineVisibility GetTimelineVisibility(float screenX) const;

protected:
	// Timeline Zoom:
	// --------------
	struct
	{
		const float ZOOM_BASE = 150.0f;
		const float ZOOM_MIN = 1.0f;
		const float ZOOM_MAX = 10.0f;

		bool zoomLevelChanged = false;
		float zoomLevel = 2.0f, lastZoomLevel;
	};
	// --------------


	// ----------------------
	ImGuiWindow* baseWindow;
	ImDrawList* baseDrawList;

	const float timelineVisibleThreshold = 46.0f;

	TimeSpan cursorTime;
	// --------------

	// ImGui Data:
	// -----------
	struct
	{
		bool updateInput;
		ImGuiIO* io;
	};
	// -----------

	// Timeline Regions:
	// -----------------
	struct
	{
		ImRect timelineRegion;
		ImRect infoColumnHeaderRegion;
		ImRect infoColumnRegion;
		ImRect timelineBaseRegion;
		ImRect tempoMapRegion;
		ImRect timelineHeaderRegion;
		ImRect timelineTargetRegion;
	};
	// -----------------

	// ----------------------
	struct
	{
		float infoColumnWidth = 46.0f;
		float timelineHeaderHeight = 40.0f - 13.0f;
		float tempoMapHeight = 13.0f;
	};
	// ----------------------

	// ----------------------
	struct
	{
		// fraction of the timeline width at which the timeline starts scrolling relative to the cursor
		const float autoScrollOffsetFraction = 4.0f;

		bool autoScrollCursor = false;
		const float CURSOR_HEAD_WIDTH = 17.0f;
		const float CURSOR_HEAD_HEIGHT = 8.0f;

		float scrollDelta = 0.0f;
		const float scrollSpeed = 2.0f, scrollSpeedFast = 4.5f;
	};
	// ----------------------

	void InitializeTimelineBase();
	void UpdateTimelineBase();

	virtual void UpdateTimelineRegions();
	virtual void UpdateInputTimelineScroll();
	virtual void UpdateCursorAutoScroll();

	virtual bool GetIsPlayback() const = 0;

	virtual void CenterCursor();
	virtual bool IsCursorOnScreen() const;

	void UpdateTimelineSize();
	virtual float GetTimelineSize() const = 0;
	virtual inline float GetMaxScrollX() const { return ImGui::GetScrollMaxX(); };
	virtual inline float GetScrollX() const { return ImGui::GetScrollX(); };
	virtual inline void SetScrollX(float value) { ImGui::SetScrollX(value); };

	virtual void OnTimelineBaseScroll();

	virtual void DrawTimlineDivisors() = 0;
	virtual void DrawTimelineCursor() = 0;
};