#pragma once
#include "../TimeSpan.h"
#include "../pch.h"

namespace Editor
{
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

		void DrawTimelineGui();
		void InitializeTimelineGuiState();

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
			ImRect timelineContentRegion;
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

		// ----------------------
		void DrawTimelineBase();
		// ----------------------
		virtual void OnDrawTimelineHeaderWidgets() = 0;
		virtual void OnDrawTimelineInfoColumnHeader();
		virtual void OnDrawTimelineInfoColumn();
		// ----------------------
		virtual void OnDrawTimlineTempoMap();
		virtual void OnDrawTimlineRows() = 0;
		virtual void OnDrawTimlineDivisors() = 0;
		virtual void OnDrawTimlineBackground() = 0;
		virtual void DrawTimelineCursor();
		// ----------------------

		// ----------------------
		void UpdateTimelineBaseState();
		// ----------------------

		// ----------------------
		virtual void UpdateTimelineBase();
		virtual void OnUpdate() = 0;
		virtual void OnUpdateInput() = 0;
		virtual void OnDrawTimelineContents() = 0;
		void UpdateTimelineSize();
		virtual void UpdateTimelineRegions();
		// ----------------------
		virtual void UpdateInputTimelineScroll();
		virtual void UpdateInputPlaybackToggle();
		virtual void OnTimelineBaseScroll();
		// ----------------------
		virtual void UpdateCursorAutoScroll();
		// ----------------------

		// Timeline Control:
		// -----------------
		virtual bool GetIsPlayback() const = 0;
		virtual void PausePlayback() = 0;
		virtual void ResumePlayback() = 0;
		virtual void StopPlayback() = 0;

		virtual float GetTimelineSize() const = 0;
		virtual void CenterCursor();
		virtual bool IsCursorOnScreen() const;
		// -----------------

		virtual inline float GetMaxScrollX() const { return ImGui::GetScrollMaxX(); };
		virtual inline float GetScrollX() const { return ImGui::GetScrollX(); };
		virtual inline void SetScrollX(float value) { ImGui::SetScrollX(value); };

	private:
		void UpdateAllInput();
	};
}