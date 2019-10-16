#pragma once
#include "Core/TimeSpan.h"
#include "TimelineBaseRegions.h"
#include "ITimelinePlaybackControllable.h"
#include "ITimelineUnitConverter.h"
#include "ImGui/Gui.h"

namespace Editor
{
	enum class TimelineVisibility
	{
		Visible,
		Left,
		Right
	};

	class TimelineBase : public TimelineBaseRegions, public ITimelinePlaybackControllable
	{
	public:
		TimelineBase() {};
		virtual ~TimelineBase() {};

		virtual float GetTimelinePosition(TimeSpan time) const;
		virtual TimeSpan GetTimelineTime(float position) const;

		float ScreenToTimelinePosition(float screenPosition) const;
		float GetCursorTimelinePosition() const;

		virtual TimeSpan GetCursorTime() const;
		TimelineVisibility GetTimelineVisibility(float screenX) const;

		void DrawTimelineGui();
		void Initialize();

	public:
		inline float GetMaxScrollX() const { return Gui::GetWindowScrollMaxX(baseWindow); };
		inline float GetScrollX() const { return baseWindow->Scroll.x; };

	protected:
		inline void SetScrollX(float value) { baseWindow->ScrollTarget.x = value; baseWindow->ScrollTargetCenterRatio.x = 0.0f; };

		// TODO: Implement, probably don't wanna use the scroll y field of the window itself (?)
		// inline void SetScrollY(float value) { Gui::SetScrollY(value); };

	protected:
		// TODO: Initialize in derived class, each derived class then exposes its own casted getter (?)
		// UniquePtr<ITimelineUnitConverter> unitConverter;

		TimeSpan cursorTime;

		struct /* TimelineImGuiData */
		{
			ImGuiWindow* baseWindow = nullptr;
			ImDrawList* baseDrawList = nullptr;

			bool updateInput = false;
		};

		struct
		{
			const float timelineVisibleThreshold = 46.0f;
			float infoColumnWidth = 46.0f;
			float timelineHeaderHeight = 32.0f - 13.0f;
			float tempoMapHeight = 13.0f;
			const vec2 timelineScrollbarSize = vec2(14.0f, 16.0f);
		};

		static constexpr float ZOOM_BASE = 150.0f;
		static constexpr float ZOOM_MIN = 1.0f;
		static constexpr float ZOOM_MAX = 10.0f;

		struct /* TimelineZoomData */
		{

			bool zoomLevelChanged = false;
			float zoomLevel = 2.0f, lastZoomLevel = zoomLevel;
		};

		struct /* TimelineScrollData */
		{
			// fraction of the timeline width at which the timeline starts scrolling relative to the cursor
			const float autoScrollOffsetFraction = 4.0f;

			bool autoScrollCursor = false;

			float scrollDelta = 0.0f;
			float scrollSpeed = 2.0f, scrollSpeedFast = 4.5f;
		};

		static constexpr float CURSOR_HEAD_WIDTH = 17.0f;
		static constexpr float CURSOR_HEAD_HEIGHT = 8.0f;

		// ----------------------
		virtual void OnInitialize() {};
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
		virtual void OnDrawTimelineScrollBarRegion() {};
		// ----------------------

		void UpdateTimelineBaseState();

		// ----------------------
		virtual void UpdateTimelineBase();
		virtual void OnUpdate() = 0;
		virtual void OnUpdateInput() = 0;
		virtual void OnDrawTimelineContents() = 0;

		void UpdateTimelineSize();
		virtual void UpdateTimelineRegions() override;

		virtual void UpdateInputTimelineScroll();
		virtual void UpdateInputPlaybackToggle();
		virtual void OnTimelineBaseScroll();

		virtual void UpdateCursorAutoScroll();

		virtual float GetTimelineSize() const = 0;
		virtual void CenterCursor();
		virtual bool IsCursorOnScreen() const;

	private:
		void UpdateAllInput();
	};
}